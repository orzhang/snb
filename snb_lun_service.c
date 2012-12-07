#include <stdio.h>
#include <stdlib.h>

static snb_lun_t * lun_list;

void snb_LUN_service_init()
{

}

void snb_LUN_service_push_command(uint16_t LUN, snb_command_t* cmd)
{
	snb_lun_t* LUN;
	LUN = lun_list[LUN];
	snb_session_pipe_push_command(&LUN->rw_pipe, cmd);
}

void snb_LUN_service_thread()
{

}