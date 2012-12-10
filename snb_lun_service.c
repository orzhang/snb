#include <snb_lun_service.h>

static snb_LUN_t * lun_list;

void snb_LUN_service_init()
{
	lun_list = malloc(sizeof(snb_LUN_t));
	lun_list[0].id = 0;
	lun_list[0].file = fopen("/tmp/lun0","rw");
	lun_list[0].nblock = 64;
	lun_list[0].size = 64 * 128;
}

void snb_LUN_service_push_command(uint16_t LUN_id, snb_command_t* cmd)
{
	snb_LUN_t* LUN;
	LUN = &lun_list[LUN_id];
	snb_session_pipe_push_command(&LUN->rw_pipe, cmd);
}

void snb_LUN_service_thread(void* args)
{
	snb_LUN_t * lun = (snb_LUN_t*)args;
	snb_command_rw_t* cmd = NULL;
	snb_command_rw_t* ack_cmd = NULL;
	int size;
	int offset;
	uint8_t* pdata = NULL;
	while(lun->thread_run)
	{
		cmd = (snb_command_rw_t*)snb_session_pipe_pop_command(&lun->rw_pipe);
		if(cmd == NULL)
		{
			sleep(10);
			continue;
		}

		size = cmd->size;
		offset = cmd->offset;
		pdata = cmd->data;
		
		fseek(lun->file, offset, SEEK_SET);

		if (SNB_IS_R_CMD(cmd->rw_mask))
		{
			if((pdata = malloc(size)) == NULL)
			{
				//error;
				printf("LUN[%d] no memory\n", lun->id);
			}
			fread(pdata, size, 1, lun->file);
			ack_cmd = snb_create_command_rw_ack(lun->id, pdata, 
			offset, size, cmd->rw_mask,snb_sucess);
			free(pdata);
		}
		else if (SNB_IS_W_CMD(cmd->rw_mask))
		{
			fwrite(pdata, size, 1, lun->file);
			ack_cmd = snb_create_command_rw_ack(lun->id, pdata, 
			offset, size, cmd->rw_mask,snb_sucess);
		}
	}
}