#include <snb_session.h>
#include <snb_protocal.h>

static struct {
	snb_session_t* head;
	snb_session_t* rear;
}session_list;

static int snb_session_new_id()
{
	return 0;
}

int snb_session_pipe_push_command(snb_command_pipe_t* pipe, snb_command_t* cmd)
{
	if (cmd == NULL)
		return -1;
	SNB_LOCK_MUTEX(pipe);
	if(pipe->cmd_list_head == NULL && 
		pipe->cmd_list_tail == NULL)
	{
		pipe->cmd_list_head = cmd;
		pipe->cmd_list_tail = cmd;
	}
	else
	{
		pipe->cmd_list_tail->next = cmd;
		pipe->cmd_list_tail = pipe->cmd_list_tail->next;
	}
	SNB_UNLOCK_MUTEX(pipe);
}

snb_command_t* snb_session_pipe_pop_command(snb_command_pipe_t* pipe)
{
	snb_command_t* cmd = NULL;

	SNB_LOCK_MUTEX(pipe);
	if(pipe->cmd_list_head == NULL && 
		pipe->cmd_list_tail == NULL)
	{
		cmd = NULL;
	}
	else
	{
		cmd = pipe->cmd_list_head;
		pipe->cmd_list_head = pipe->cmd_list_head->next;
	}
	SNB_UNLOCK_MUTEX(pipe);
	return cmd;
}

int snb_session_push_command(snb_session_t* session,
	snb_command_t* cmd, pipe_type_t io)
{
	if ((cmd == NULL) || (session == NULL))
		return -1;
	cmd->parent = session;
	if(io == pipe_in)
		snb_session_pipe_push_command(&session->pipe_in, cmd);
	else
		snb_session_pipe_push_command(&session->pipe_out cmd);
	return 0;
}

snb_command_t* snb_session_pop_command(snb_session_t* session)
{
	snb_command_t* cmd = NULL;

	if(io == pipe_in)
		cmd = snb_session_pipe_pop_command(&session->pipe_in, cmd);
	else
		cmd = snb_session_pipe_pop_command(&session->pipe_out cmd);
	return cmd;
}

snb_session_t* snb_add_session(int sock)
{
	snb_session_t* session;
	if(sock == -1)
		return NULL;
	session = SNB_MALLOC(sizeof(*session));
		if(session == NULL)
			return NULL;
	session->next = NULL;
	session->cmd_list_head = NULL;
	session->cmd_lead_tail = NULL;
	session->passwd = {NULL, NULL};
	session->seq_num = 0;
	session->sock = 0;
	session->id = snb_session_new_id();
	if((session->msg_buf_in = snb_alloc_msg_buf()) == NULL)
	{
		free(session);
		return NULL;
	}
	if((session->msg_buf_out = snb_alloc_msg_buf()) == NULL)
	{
		free(session->msg_buf_in);
		free(session);
		return NULL;
	}
	if(session_list.head == NULL && session_list.rear == NULL)
	{
		session_list.head = session;
		session_list.rear = session;
		session_list.head->next = session;
	}
	else
	{
		session_list.rear->next = session;
		session_list.rear = session;
	}
	return session;
}

int snb_remove_session(snb_session_t* session)
{
	snb_session_t* tmp_session;
	snb_session_t* next_session;

	if(session == NULL)
		return -1;

	tmp_session = session_list.head;
	
	if(session_list.rear == session)
	{
		if(session_list.head == session)
		{
			session_list.head = NULL;
			session_list.rear = NULL;
			return 0;
		}

		tmp_session = session_list.head;
		while(tmp_session->next != session_list.rear)
		{
			tmp_session = tmp_session->next;
		}

		tmp_session->next = NULL;
	}
	else
	{
		*session = *(session->next);
	}
	return 0;
}

snb_session_t* snb_find_session(int sock)
{
	snb_session_t* session;
	session = session_list.head;
	while(session != NULL)
	{
		if(session->sock == sock)
			return session;
		session = session->next;
	}
	return NULL;
}

const snb_session_t*  snb_iteritor_session_begin()
{
	return session_list.head;
}

const snb_session_t*  snb_iteritor_session_next(const snb_session_t* it)
{
	if(it == NULL)
		return NULL;
	return it->next;
}

const snb_session_t*  snb_iteritor_session_end()
{
	return session_list.rear;
}