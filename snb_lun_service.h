#ifndef SNB_LUN_SERVICE_H
#define SNB_LUN_SERVICE_H
#include <stdio.h>
#include <stdlib.h>
#include <snb_common.h>
#include <snb_protocal.h>
#include <snb_session.h>
typedef struct snb_LUN
{
	uint16_t id;
	FILE *file;
	uint32_t nblock;
	uint32_t size;
	snb_command_pipe_t rw_pipe;
	int thread_run;
} snb_LUN_t;


void snb_LUN_service_init();

void snb_LUN_service_push_command(uint16_t LUN, snb_command_t* cmd);

#endif