#include <snb_lun_service.h>
#include <pthread.h>
static snb_LUN_t * lun_list;

static snb_block_config_t* config_list = NULL;
static uint32_t snb_block_config_num = 0;
void snb_LUN_service_thread(void* args);

int snb_get_luns()
{
	return snb_block_config_num;
}

int snb_get_lun_id(uint16_t *array, int size)
{
	int i = 0;
	if (size < snb_block_config_num)
		return -1;
	if (array == NULL)
		return -1;
	for (i = 0 ; i < snb_block_config_num; i++) {
		array[i] = lun_list[i].id;
	}
	return 0;
}

snb_block_config_t * snb_get_lun_info(uint16_t id)
{
	if(id > snb_block_config_num)
		return NULL;
	return lun_list[id].config;
}

int snb_parse_config(const char* file)
{
	char* line = NULL;
	char* pline;
	char* token;
	size_t len = 0;
	ssize_t read;
	snb_block_config_t * tmp_config;
	snb_block_config_t * cur_config = config_list;
	FILE *fp = fopen(file, "r");
	if(fp == NULL) {
		return -1;
	}
	while ((read = getline(&line, &len, fp)) != -1)
	{
		if((tmp_config = SNB_MALLOC(sizeof(*tmp_config))) == NULL)
		{
			SNB_TRACE("can not alloc new snb_block_config\n");
			fclose(fp);
			return -1;
		}
		pline = line;
		pline[strlen(pline) - 1 ] = '\0';
		token = strtok(pline, ":");
		tmp_config->name = strdup(token);
		token = strtok(NULL, ":");
		tmp_config->nblock = atoi(token);
		token = strtok(NULL, ":");
		tmp_config->size = atoi(token);
		token = strtok(NULL, ":");
		tmp_config->path = strdup(token);
		tmp_config->next = NULL;
		snb_block_config_num++;
		SNB_TRACE("name=%s, path=%s, nblock=%d, size=%d\n",
			tmp_config->name,
			tmp_config->path,
			tmp_config->nblock,
			tmp_config->size);

		if(cur_config == NULL)
		{
			cur_config = tmp_config;
			config_list = tmp_config;
		}
		else
		{
			cur_config->next = tmp_config;
			cur_config = cur_config->next;
		}

	}
	free(line);
	fclose(fp);
	return 0;
}

int snb_LUN_service_init(const char* file)
{
	int rc = 0;
	int i = 0;
	char path_buf[255];
	snb_block_config_t * cur_config;
	rc = snb_parse_config(file);
	cur_config = config_list;
	if(rc < 0)
		return rc;
	lun_list = malloc(sizeof(snb_LUN_t) * snb_block_config_num);
	for(i = 0 ; i < snb_block_config_num; i++) {
		lun_list[i].config = cur_config;
		lun_list[i].thread_run = 1;
		bzero(path_buf, sizeof(path_buf));
		sprintf(path_buf, "%s/%s", cur_config->path, cur_config->name);
		lun_list[i].file = fopen(path_buf, "w+");
		if(lun_list[i].file == NULL)
		{
			rc = -1;
		}
		cur_config = cur_config->next;
		lun_list[i].id = i;
		pthread_create(&lun_list[i].pid,
			NULL, snb_LUN_service_thread, &lun_list[i]);
	}
	return rc;
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
			ack_cmd = (snb_command_rw_t*)snb_create_command_rw_ack(lun->id, pdata, 
			offset, size, cmd->rw_mask,snb_sucess);
			free(pdata);
		}
		else if (SNB_IS_W_CMD(cmd->rw_mask))
		{
			fwrite(pdata, size, 1, lun->file);
			ack_cmd = (snb_command_rw_t*)snb_create_command_rw_ack(lun->id, pdata, 
			offset, size, cmd->rw_mask,snb_sucess);
		}
	}
}