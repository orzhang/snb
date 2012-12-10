#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <snb_session.h>
#define MAX_CONNECTION (256)
#define BACK_LOG (5)

typedef struct snb_server
{
	int sockfd;
	int max_sockfd;
	uint8_t * sock_buf;
} snb_server_t;

snb_server g_server;
int server_init(uint16_t port)
{
	struct sockaddr_in serv_addr;
	g_server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (g_server.sockfd < 0)
	{
		perror("ERROR opening socket");
		return -1;
	}

	if (setsockopt(g_server.sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt error!");
        return -1;
    }

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(g_server.sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		return -1;
	}
	listen(g_server.sockfd,BACK_LOG);
	g_server.max_sockfd = g_server.sockfd;
	if ((g_server.sock_buf = malloc(MAX_BUFFER)) == NULL)
		return -1;
	return 0;
}

int server_process()
{
	struct timeval tv;
	fd_set fdsr_r,fdsr_w;
	snb_session_t* it;
	int ret = 0;
	FD_ZERO(&fdsr_r);
    FD_ZERO(&fdsr_w);
	FD_SET(g_server.sockfd, &fdsr_r);
	/* Wait up to five seconds. */
	tv.tv_sec = 5;
	tv.tv_usec = 0;

    for(it = snb_iteritor_session_begin();
    	it != snb_iteritor_session_end();
    	it = snb_iteritor_session_next())
    {
        FD_SET(it->sock, &fdsr_r);
        FD_SET(it->sock, &fdsr_w);
    }

    ret = select(g_server.max_sockfd + 1,
    	&fdsr_r, fdsr_w, NULL, &tv);
	if (ret < 0)
    {
    	perror("select error!");
    	break;
    }
    else if (ret == 0)
    {
    	printf("timeout\n");
    	return 0;
    }

    if (FD_ISSET(sock_fd, &fdsr_rd))
    {
        server_accept(&g_server);
    }
    

    for(it = snb_iteritor_session_begin();
    	it != snb_iteritor_session_end();
    	it = snb_iteritor_session_next())
    {
    	
    	if (FD_ISSET(it->sock, &fdsr_r))
    	{
    		server_recv(it);
    	}
        snb_server_dispatch(it);
        if (FD_ISSET(it->sock, &fdsr_w))
        {
            server_send(it);
        }
    }
}

int server_accept(snb_server_t * server)
{
	struct sockaddr_in client_addr;
	snb_session_t * session;
	socklen_t sin_size;
	sin_size = sizeof(client_addr);
	int new_fd = 0;
	new_fd = accept(server->sockfd,
        (struct sockaddr *)&client_addr, &sin_size);
	if (new_fd <= 0)
	{
		perror("accept socket error!");
		return -1;
	}
	if (conn_amount < MAX_CONNECTION)
	{
		if (new_fd > server->sockfd)
			server->max_sockfd = new_fd;
		session = snb_add_session(new_fd);
		if(session == NULL)
		{
			printf("can not malloc a new session, exit\n");
			close(new_fd);
			return -1;
		}
		session->addr = client_addr;
        session->seq_num = 1;
		return 0;
	}
	else
	{
		printf("max connections arrive, exit\n");
		close(new_fd);
		return 0;
	}
	return 0;
}

int server_recv(snb_session_t* session)
{
    int ret = 0;
    int rc;
    int expect = SNB_COMMAND_HEAD_SIZE;
    int state = SNB_RECV_STATE_HEAD;
    snb_command_t* command = NULL;
    snb_msg_buf_t * msg_buf = session->msg_buf_in;
    switch (msg_buf->msg_state)
   	{
   		case SNB_RECV_STATE_HEAD:
   			msg_buf->pbuf = msg_buf->buf;
   			ret = recv(session->sock, msg_buf->pbuf, msg_buf->expect, 0);
   			if(ret < 0)
    				goto error:
   			msg_buf->pbuf += ret;
   			msg_buf->expect -= ret;
   			if(msg_buf->expect == 0)
   			{
   				msg_buf->expect = SNB_COMMAND_HEAD_ATTR_LENGTH(msg_buf->buf);
   				msg_buf->msg_state = SNB_RECV_STATE_BODY;
   			}
   		break;
    	
    	case SNB_RECV_STATE_BODY:
			ret = recv(session->sock, msg_buf->pbuf, expect, 0);
			if(ret < 0)
				goto error:
			msg_buf->pubf += ret;
			msg_buf->expect -= ret;
			if(expect == 0)
			{
				msg_buf->msg_state = SNB_RECV_STATE_HEAD;
				if (snb_unpack_command(msg_buf->pbuf, &command) != 0)
					goto error:
				snb_session_push_command(session, command,pipe_in);
			}
			return 0;
		break;

		default:
		break;
	}
    return 0;

error:
    printf("session[%d] close\n", session->id);
    close(session->sock);
    FD_CLR(session->sock, &fdsr);
    snb_remove_session(session);
}

int server_send(snb_session_t* session)
{
    int ret = 0;
    int rc;
    int expect = SNB_COMMAND_HEAD_SIZE;
    int state = SNB_RECV_STATE_HEAD;
    snb_command_t* command = NULL;
    snb_msg_buf_t* msg_buf = session->msg_buf_out;
    switch (msg_buf->state)
    {
        case SNB_MSG_SEND_STATE_START:
            command = snb_session_pop_command(session, command, pipe_out);
            if(command == NULL)
                return 0;
            if((msg_buf = snb_realloc_msg_buf(msg_buf, command->length)) == NULL)
                goto error:
            msg_buf->state = SNB_MSG_SEND_STATE_TX;
            msg_buf->pbuf = msg_buf->buf;
            msg_buf->expect = msg_buf->size;
            snb_pack_command(msg_buf->buf, msg_buf->size, command);
        break;
        
        case SNB_MSG_SEND_STATE_TX:
            ret = send(session->sock, msg_buf->pbuf, expect, 0);
            if(ret < 0)
                goto error:
            msg_buf->pubf += ret;
            msg_buf->expect -= ret;
            if(expect == 0)
            {
                msg_buf->msg_state = SNB_MSG_SEND_STATE_START;
            }
            return 0;
        break;

        default:
        break;
    }
    return 0;
error:
    printf("session[%d] close\n", session->id);
    close(session->sock);
    FD_CLR(session->sock, &fdsr);
    snb_remove_session(session);
	return 0;
}

int main()
{
	init_server(8086);
    while(1)
    {
        server_process();
    }
    return 0;
}