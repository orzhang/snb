#include <snb_protocal.h>
#include <snb_session.h>
#include <snb_lun_service.h>


void snb_server_process_login(snb_command_t* cmd)
{
	snb_command_login_t* login = (snb_command_login_t*)cmd;
	snb_command_login_ack_t* ack;
	ack = snb_create_command_login_ack(snb_sucess);
	snb_session_push_command(cmd->parent, ack, pipe_out);
}

void snb_server_process_list(snb_command_t* cmd)
{

}

void snb_server_process_info(snb_command_t* cmd)
{

}

void snb_server_process_rw(snb_command_t* cmd)
{
	snb_command_rw_t* rw_cmd = (snb_command_rw_t*)cmd;
	snb_LUN_service_push_command(rw_cmd->LUN, cmd);
}

void snb_server_process_ping(snb_command_t* cmd)
{
	snb_command_ping_ack_t *ack = NULL;
	ack = snb_create_command_ping_ack(snb_sucess);
	snb_session_push_command(cmd->parent, ack, pipe_out);
}

void snb_server_dispatch(snb_session_t* session)
{
	snb_command_t* cmd = NULL;
	cmd = snb_session_pop_command(session, pipe_in);
	switch(cmd->id)
	{
		case SNB_CMD_ID_LIST:
			snb_server_process_list(cmd);
			break;
		case SNB_CMD_ID_INFO:
			snb_server_process_info(cmd);
			break;
		case SNB_CMD_ID_LOGIN:
			snb_server_process_login(cmd);
			break;
		case SNB_CMD_ID_PING:
			snb_server_process_ping(cmd);
			break;
		case SNB_CMD_ID_RW:
			snb_server_process_rw(cmd);
		default:
			break;
	}
}
