#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <snb_session.h>
#include <snb_protocal.h>
#include <sys/select.h>
#include <unistd.h>
#include <snb_common.h>
#define DEFAULT_SERVER "127.0.0.1"
int sock_fd;
struct sockaddr_in addr;

int snb_connect(uint16_t port)
{
	int rc = 0;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(DEFAULT_SERVER, &addr.sin_addr);
	rc = connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
}
void test_ping()
{
	int expect;
	int ret = 0;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	snb_command_t* cmd = snb_create_command_ping(server);
	snb_command_t* ack_cmd;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);
	SNB_TRACE("ping sent\n");

	ret = recv(sock_fd, buf, SNB_COMMAND_HEAD_SIZE, 0);
	snb_unpack_command(buf, sizeof(buf), &ack_cmd);
	if(ack_cmd != NULL)
	{
		SNB_TRACE("id = %d\n", ack_cmd->id);
		SNB_TRACE("type = %d\n", ack_cmd->type);
		SNB_TRACE("proxy = %d\n", ack_cmd->proxy);
		SNB_TRACE("rc = %d\n", ack_cmd->rc);
	}	
}
void test_login()
{
	int expect;
	int ret = 0;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	snb_command_t* cmd = snb_create_command_login("root", "root");
	snb_command_t* ack_cmd;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);
	SNB_TRACE("login sent\n");

	ret = recv(sock_fd, buf, SNB_COMMAND_HEAD_SIZE, 0);
	snb_unpack_command(buf, sizeof(buf), &ack_cmd);
	if(ack_cmd != NULL)
	{
		SNB_TRACE("id = %d\n", ack_cmd->id);
		SNB_TRACE("type = %d\n", ack_cmd->type);
		SNB_TRACE("proxy = %d\n", ack_cmd->proxy);
		SNB_TRACE("rc = %d\n", ack_cmd->rc);
	}

}

int main()
{
	snb_connect(8086);
	test_login();
	test_ping();
	getchar();
	close(sock_fd);
	return 0;
}