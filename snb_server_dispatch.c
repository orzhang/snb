#include <snb_protocal.h>
#include <snb_session.h>
#include <snb_lun_service.h>
#include <string.h>
#include <snb_common.h>
void snb_server_process_login(snb_command_t* cmd)
{
	snb_command_login_t* login = (snb_command_login_t*)cmd;
	snb_command_login_ack_t* ack;
	SNB_TRACE("login %s:%s\n", login->usr, login->passwd);
	if(strcmp(login->usr, "root") == 0)
	{
		ack = snb_create_command_login_ack(snb_sucess);
	}
	else
	{
		ack = snb_create_command_login_ack(snb_login_error);
	}
	snb_session_push_command(cmd->parent, ack, pipe_out);
}

void snb_server_process_list(snb_command_t* cmd)
{
	snb_command_list_ack_t * ack = NULL;
	uint16_t *id_array = NULL;
	snb_command_rc_t rc;

	int luns = snb_get_luns();
	id_array = malloc(sizeof(uint16_t) * luns);
	if(id_array == NULL)
		SNB_TRACE("no memory!!!\n");

	rc = snb_sucess;
	snb_get_lun_id(id_array, luns);
	ack = snb_create_command_list_ack(id_array, luns, rc);
	snb_session_push_command(cmd->parent, ack, pipe_out);
}

void snb_server_process_info(snb_command_t* cmd)
{
	
	uint16_t* LUNs;
	uint32_t* block_num;
	uint32_t* block_size;
	int i = 0;
	snb_block_config_t* config = NULL;
	snb_command_rc_t rc;

	snb_command_info_ack_t * ack;
	snb_command_info_t * ask = (snb_command_info_t *)cmd;
	LUNs = malloc(sizeof(uint16_t) * ask->size);
	block_num = malloc(sizeof(uint32_t) * ask->size);
	block_size = malloc(sizeof(uint32_t) * ask->size);
	for (i = 0; i < ask->size; i++)
	{
		config = snb_get_lun_info(ask->LUNs[i]);
		SNB_TRACE("%d,%d,%d\n",ask->LUNs[i],
			config->nblock, config->size);

		LUNs[i] = ask->LUNs[i];
		block_num[i] = config->nblock;
		block_size[i] = config->size;
	}
	rc = snb_sucess;
	ack = snb_create_command_info_ack(ask->size,
		LUNs, block_num, block_size, rc);
	snb_session_push_command(cmd->parent, ack, pipe_out);
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
	if(cmd == NULL)
		return;
	SNB_TRACE("dispatch cmd=[%d]\n", cmd->id);
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
