#ifndef SNB_LUN_SERVICE_H
#define SNB_LUN_SERVICE_H
#include <stdio.h>
#include <stdlib.h>
#include <snb_common.h>
#include <snb_protocal.h>
#include <snb_session.h>
#include <pthread.h>
typedef struct snb_block_config {
	struct snb_block_config* next;
	char* name;
	char* path;
	uint32_t nblock;
	uint32_t size;
} snb_block_config_t;

typedef struct snb_LUN
{
	int id;
	pthread_t pid;
	FILE *file;
	snb_block_config_t* config;
	snb_command_pipe_t rw_pipe;
	int thread_run;
} snb_LUN_t;


int snb_LUN_service_init(const char* file);

void snb_LUN_service_push_command(uint16_t LUN, snb_command_t* cmd);

int snb_get_luns();
int snb_get_lun_id(uint16_t *array, int size);
snb_block_config_t * snb_get_lun_info(uint16_t id);
#endif