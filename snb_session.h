#ifndef SNB_SESSION_H
#define SNB_SESSION_H
#include <snb_common.h>

typedef enum pipe_type = {
	pipe_in,
	pipe_out
} pipe_type_t;

typedef struct snb_passwd
{
	const char * usr;
	const char * passwd;
}snb_passwd_t;

typedef struct snb_command_pipe {
	snb_command_t* cmd_list_head;
	snb_command_t* cmd_list_tail;
} snb_command_pipe_t;

struct snb_session
{
	struct snb_session* next;
	snb_command_pipe_t pipe_in;
	snb_command_pipe_t pipe_out;
	snb_msg_buf_t * msg_buf_in;
	snb_msg_buf_t * msg_buf_out;
	struct sockaddr_in addr;
	uint64_t seq_num;
	int sock;
	int id;
};

int snb_session_push_command(snb_session_t* session,
	snb_command_t* cmd, int io);

snb_command_t* snb_session_pop_command(snb_session_t* session, int io);

snb_session_t* snb_find_session(int sock);

int snb_remove_session(snb_session_t* session);

snb_session_t* snb_add_session(int sock);

nt snb_session_pipe_push_command(snb_command_pipe_t * pipe,
	snb_command_t* cmd);

snb_command_t* snb_session_pipe_pop_command(snb_command_pipe_t * pipe);

const snb_session_t*  snb_iteritor_session_begin();

const snb_session_t*  snb_iteritor_session_next(const snb_session_t* it);

const snb_session_t*  snb_iteritor_session_end();

#endif