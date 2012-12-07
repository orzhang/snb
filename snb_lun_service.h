#ifndef SNB_LUN_SERVICE_H
#define SNB_LUN_SERVICE_H

typedef struct snb_LUN
{
	uint16_t id;
	FILE *file;
	uint32_t nblock;
	uint32_t size;
	snb_command_pipe_t rw_pipe;
}snb_LUN_t;


void snb_LUN_service_init();

void snb_LUN_service_push_command(uint16_t LUN, snb_command_t* cmd);

#endif