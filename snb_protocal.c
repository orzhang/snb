#include <stdlib.h>
#include <string.h>
#include <snb_protocal.h>

static int sub_pack_head(uint8_t* buffer, snb_command_t *pcmd);

static int snb_pack_rw_command(uint8_t* buffer, snb_command_t *pcmd);

static int snb_pack_login_command(uint8_t* buffer, snb_command_t *pcmd);

static int snb_pack_ping_command(uint8_t* buffer, snb_command_t *pcmd);

static int snb_pack_info_command(uint8_t* buffer, snb_command_t *pcmd);

static int snb_pack_list_command(uint8_t* buffer, snb_command_t *pcmd);

static int snb_unpack_rw_command(const uint8_t* buffer, const snb_command_t * base,
	snb_command_t ** pcmd);
static int snb_unpack_login_command(const uint8_t* buffer, const snb_command_t * base,
	snb_command_t ** pcmd);
static int snb_unpack_ping_command(const uint8_t* buffer, const snb_command_t * base,
	snb_command_t ** pcmd);
static int snb_unpack_info_command(const uint8_t* buffer, const snb_command_t * base,
	snb_command_t ** pcmd);
static int snb_unpack_list_command(const uint8_t* buffer, const snb_command_t * base,
	snb_command_t ** pcmd);

int snb_pack_command(uint8_t ** buffer, snb_command_t * pcmd)
{
	uint8_t * cmd_buff;
	cmd_buff = SNB_MALLOC(pcmd->length);
	if(buffer == NULL || cmd_buff == NULL)
		return -1;
	*buffer = cmd_buff
	switch(pcmd->type)
	{
		case SNB_CMD_ID_LIST:
			if(sub_pack_head(cmd_buff, pcmd) ||
				snb_pack_list_command(cmd_buff + SNB_COMMAND_HEAD_SIZE, pcmd))
			{
				SNB_SNB_FREE(cmd_buff);
				return -1;
			}
			return 0;			
			break;
		case SNB_CMD_ID_INFO:
			if(sub_pack_head(cmd_buff, pcmd) ||
				sub_pack_info_command(cmd_buff + SNB_COMMAND_HEAD_SIZE, pcmd))
			{
				SNB_SNB_FREE(cmd_buff);
				return -1;
			}
			return 0;
			break;
		case SNB_CMD_ID_PING:
			if(sub_pack_head(cmd_buff, pcmd) ||
				sub_pack_ping_command(cmd_buff + SNB_COMMAND_HEAD_SIZE, pcmd))
			{
				SNB_SNB_FREE(cmd_buff);
				return -1;
			}
			return 0;
			break;
		case SNB_CMD_ID_LOGIN:
			if(sub_pack_head(cmd_buff, pcmd) ||
				sub_pack_login_command(cmd_buff + SNB_COMMAND_HEAD_SIZE, pcmd))
			{
				SNB_SNB_FREE(cmd_buff);
				return -1;
			}
			return 0;
			break;
		case SNB_CMD_ID_RW:
			if(sub_pack_head(cmd_buff, pcmd) ||
				sub_pack_rw_command(cmd_buff + SNB_COMMAND_HEAD_SIZE, pcmd))
			{
				SNB_SNB_FREE(cmd_buff);
				return -1;
			}
			return 0;
			break;
		default:
		return -1;
	}
	return -1;
}

int sub_pack_head(uint8_t * buffer, snb_command_t *pcmd)
{
	uint64_t * pbuffer_64;
	if(buffer == NULL || pcmd == NULL)
		return -1;
	buffer[0] = pcmd->id;
	buffer[1] = pcmd->type;
	buffer[2] = pcmd->proxy;
	buffer[3] = pcmd->rc;
	pbuffer_64 = (uint64_t*) &buffer[4];
	pbuffer_64[0] = htonll(pcmd->seq_num);
	pbuffer_64[1] = htonll(pcmd->length);
	return 0;
}

int snb_pack_rw_command(uint8_t * buffer, snb_command_t *pcmd)
{
	snb_command_rw_t * cmd = (snb_command_rw_t*)pcmd;
	uint8_t * pbuffer = buffer;
	uint16_t *pbuffer_16 = buffer + 16;
	uint64_t * pbuffer_64 = buffer;
	uint64_t pa_size = 0;

	if (buffer == NULL)
		return -1;

	if (pcmd->type == ask)
	{
		pbuffer_64[0] = ntohll(cmd->offset);
		pbuffer_64[1] = ntohll(cmd->size);
		pbuffer_16[0] = ntohs(cmd->LUN);
		pbuffer += 18;
		*pbuffer = cmd->rw_mask;
		pbuffer++;
		if (SNB_IS_W_CMD(cmd->rw_mask))
		{
			if(cmd->data == NULL)
				return -1;
			memcpy(pbuffer, cmd->data, cmd->size);
		}
		return 0;
	}
	else
	{
		pbuffer_64[0] = ntohll(cmd->offset);
		pbuffer_64[1] = ntohll(cmd->size);
		pbuffer_16[0] = ntohs(cmd->LUN);
		pbuffer += 18;
		*pbuffer = cmd->rw_mask;
		pbuffer++;
		if (SNB_IS_R_CMD(cmd->rw_mask))
		{
			if(cmd->data == NULL)
				return -1;
			memcpy(pbuffer, cmd->data, cmd->size);
		}
		return 0;
	}
	return -1;
}

int snb_pack_login_command(uint8_t * buffer, snb_command_t *pcmd)
{
	uint8_t len1= 0, len2 = 0;
	uint64_t pa_size = 0;
	if (buffer == NULL)
		return -1;

	if(pcmd->type == ask)
	{
		uint8_t * pbuffer = buffer;
		snb_command_login_t * cmd = (snb_command_login_t*)pcmd;
		len1 = strlen(cmd->usr);
		len2 = strlen(cmd->passwd);
		pbuffer[0] = len1;
		pbuffer[1] = len2;
		pbuffer += 2
		memcpy(buffer, cmd->usr, len1);
		pbuffer += len1;
		memcpy(buffer, cmd->passwd, len2);
		return 0;
	}
	else
	{
		return 0;
	}
	return -1;
}

int snb_pack_ping_command(uint8_t * buffer, snb_command_t *pcmd)
{
	
	if (buffer == NULL)
		return -1;
	
	if(pcmd->type == ask)
	{
		snb_command_ping_t * cmd = (snb_command_ping_t*)pcmd;
		buffer[0] = cmd->type;
		return 0;
	}
	else
	{
		return 0;
	}
	return -1;
}

int snb_pack_info_command(uint8_t * buffer, snb_command_t *pcmd)
{
	uint64_t pa_size = 0;
	uint16_t* pbuffer;
	int i = 0;

	if (buffer == NULL)
		return -1;
	
	if(pcmd->type == ask)
	{
		snb_command_info_t* cmd = (snb_command_info_t*)pcmd;
		
		pbuffer[0] = ltons(pcmd->size);
		pbuffer++;
		for (i = 0; i < pcmd->size; i++)
		{
			pbuffer[i] = ltons(pcmd->LUNs[i]);
		}
		return 0;	
	}
	else
	{
		snb_command_info_ack_t* cmd = (snb_command_info_ack_t*)pcmd;
		pbuffer[0] = ltons(pcmd->size);
		pbuffer++;
		for (i = 0; i < pcmd->size; i++)
		{
			pbuffer[i] = ltons(pcmd->LUNs[i]);
			pbuffer[i + pcmd->size] = ltons(pcmd->block_num[i]);
			pbuffer[i + (pcmd->size * 2)] = ltons(pcmd->block_size[i]);
		}
		return 0;
	}
	return -1;
}

int snb_pack_list_command(uint8_t * buffer, snb_command_t *pcmd)
{
	uint64_t pa_size = 0;
	uint16_t* pbuffer;
	int i = 0;
	if (buffer == NULL)
		return -1;
	
	if(pcmd->type == ask)
	{
		snb_command_list_t* cmd = (snb_command_list_t*)pcmd;
		return 0;
	}
	else
	{
		snb_command_list_ack_t* cmd = (snb_command_list_ack_t*)pcmd;
		pbuffer[0] = ltons(pcmd->size);
		pbuffer++;
		for (i = 0; i < pcmd->size; i++)
		{
			pbuffer[i] = ltons(pcmd->LUNs[i]);
		}
		return 0;
	}
	return -1;
}

int snb_unpack_command(uint8_t * buffer, snb_command_t ** pcmd)
{
	int i = 0;
	int rc = 0;
	uint8_t * pbuffer = buffer;
	snb_command_t base;
	snb_command_t* cmd = NULL;
	base.next = NULL;
	base.id = *buffer;
	base.type = *(buffer + 1);
	base.proxy = *(buffer + 2);
	base.rc = *(buffer + 3);
	base.seq_num = ntohll(*((uint64_t *)(buffer + 4)));
	base.length = ntohll(*((uint64_t *)(buffer + 12)));
	pbuffer += 20;
	
	switch(base.id)
	{
		case SNB_CMD_ID_LIST:
			rc = snb_recv_list_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_INFO:
			rc = snb_recv_info_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_LOGIN:
			rc = snb_recv_login_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_PING:
			rc = snb_recv_ping_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_RW:
			rc = snb_recv_rw_command(pbuffer, &base, &cmd);
		default:
			rc = -1;
		break;
	}
	return rc;
}

int snb_unpack_rw_command(const uint8_t * buffer, const snb_command_t * base,
	snb_command_t ** pcmd)
{
	uint64_t offset = 0, size = 0;
	uint8_t rw_maks = 0;
	uint16_t LUN = 0;
	uint8_t *pdata = NULL;
	uint8_t *rw_data = NULL;

	offset = ntohll(*((uint64_t*)(buffer));
	size = ntohll(*((uint64_t*)(buffer + 8));

	LUN = ntohs(*(buffer + 16));
	rw_mask = *(buffer + 18);
	pdata = (buffer + 19);

	if(base->type == ask)
	{
		snb_command_rw_t * cmd = SNB_MALLOC(sizeof(snb_command_rw_t));
		if(cmd == NULL)
			return -1;
		if(SNB_IS_W_CMD(rw_mask))
		{
			if ((rw_data = SNB_MALLOC(size) == NULL)
			{
				SNB_FREE(cmd);
				return -1;
			}
			memcpy(rw_data, pdata);
		}
		cmd->offset = offset;
		cmd->size = size;
		cmd->rw_maks = rw_maks;
		cmd->data = rw_data;
		*pcmd = cmd;
		return 0;
	}
	else
	{
		snb_command_rw_ack_t * cmd = SNB_MALLOC(sizeof(snb_command_rw_ack_t));
		if(cmd == NULL)
			return -1;

		if(SNB_IS_R_CMD(rw_mask)) == 0)
		{
			if ((rw_data = SNB_MALLOC(size) == NULL)
			{
				SNB_FREE(cmd);
				return -1;
			}
			memcpy(rw_data, pdata);
		}
		cmd->offset = offset;
		cmd->size = size;
		cmd->rw_maks = rw_maks;
		cmd->data = rw_data;
		*pcmd = cmd;
		return 0;
	}
	return 0;
}

int snb_unpack_login_command(const uint8_t * buffer, const snb_command_t * base,
	snb_command_t ** pcmd)
{
	if(base->type == ask)
	{
		uint8_t * pbuffer = buffer;
		uint8_t len1,len2;
		snb_command_login_t * cmd = SNB_MALLOC(sizeof(snb_command_login_t));
		if(cmd == NULL)
			return -1;
		cmd->base = *base;
		len1 = *pbuffer++;
		len2 = *pbuffer++;
		if ((cmd->usr = SNB_MALLOC(len1 + 1)) == NULL)
		{
			SNB_FREE(cmd);
			return -1;
		}

		if ((cmd->passwd = SNB_MALLOC(len2 + 1)) == NULL)
		{
			SNB_FREE(cmd->usr);
			SNB_FREE(cmd);
			return -1;
		}

		memcpy(cmd->usr, pbuffer, len1);
		cmd->usr[len1] = '\0';
		pbuffer += len1;

		memcpy(cmd->passwd, pbuffer, len2);
		cmd->usr[len2] = '\0';

		*pcmd = cmd;
		return 0;
	}
	else
	{
		snb_command_login_ack_t * cmd = SNB_MALLOC(sizeof(snb_command_login_ack_t));
		if(cmd == NULL)
			return -1;
		cmd->base = *base;
		*pcmd = cmd;
		return 0;	
	}
	return 0;
}

int snb_unpack_ping_command(const uint8_t * buffer, const snb_command_t * base,
	snb_command_t ** pcmd)
{
	if(base->type == ask)
	{
		snb_command_ping_t * cmd = SNB_MALLOC(sizeof(snb_command_ping_t));
		if(cmd == NULL)
			return -1;
		cmd->base = *base;
		cmd->type = buffer[0];
		*pcmd = cmd;
		return 0;
	}
	else
	{
		snb_command_ping_ack * cmd = SNB_MALLOC(sizeof(snb_command_ping_ack));
		if(cmd == NULL)
			return -1;
		cmd->base = *base;
		*pcmd = cmd;
		return 0;	
	}
	return 0;
}

int snb_unpack_info_command(const uint8_t * buffer, const snb_command_t * base,
	snb_command_t ** pcmd)
{
	int i = 0;
	uint16_t * LUNs = NULL, block_num = NULL, block_size = NULL;
	uint16_t * pbuffer = buffer;
	if(base->type == ask)
	{
		snb_command_info_t * cmd = SNB_MALLOC(sizeof(snb_command_info_t));
		if (cmd == NULL)
		{
			return -1;
		}
		cmd->base = *base;
		cmd->size = ntohs(*pbuffer++);
		if ((LUNs = SNB_MALLOC(sizeof(uint16_t) * cmd->size)) == NULL)
		{
			SNB_FREE(cmd);
			return -1;
		}

		for(i = 0; i < cmd->size; i++)
		{
			LUNs[i] = ntohs(*pbuffer++);
		}
		cmd->LUNs = LUNs;
		*pcmd = cmd;
		return 0;
	}
	else
	{
		snb_command_info_ack * cmd = SNB_MALLOC(sizeof(snb_command_info_ack));
		if (cmd == NULL)
		{
			return -1;
		}
		cmd->base = *base;
		cmd->size = ntohs(*pbuffer++);
		if (((LUNs = SNB_MALLOC(sizeof(uint16_t) * cmd->size)) == NULL) ||
			((block_num = SNB_MALLOC(sizeof(uint16_t) * cmd->size)) == NULL) ||
			((block_size = SNB_MALLOC(sizeof(uint16_t) * cmd->size)) == NULL))
		{
			SNB_FREE(cmd);
			SNB_FREE(LUNs)
			SNB_FREE(block_size);
			SNB_FREE(block_num);
			return -1;
		}

		for(i = 0; i < cmd->size; i++)
		{
			LUNs[i] = ntohs(pbuffer[i]);
			block_num[i] = ntohs(pbuffer[i + cmd->size]);
			block_size[i] = ntohs(pbuffer[i + (cmd->size * 2)]);
		}

		cmd->LUNs = LUNs;
		cmd->block_num = block_num;
		cmd->block_size = block_size;
		*pcmd = cmd;
		return 0;
	}
	return 0;
}

int snb_unpack_list_command(const uint8_t * buffer, const snb_command_t * base,
	snb_command_t ** pcmd)
{
	uint16_t* pbuffer = buffer;
	if(base->type == ask)
	{
		snb_command_list_t * cmd = SNB_MALLOC(sizeof(snb_command_list_t));
		if (cmd == NULL)
		{
			return -1;
		}
		cmd->base = *base;
		*pcmd = cmd;
		return 0;
	}
	else
	{
		snb_command_list_ack_t * cmd = SNB_MALLOC(sizeof(snb_command_list_ack_t));
		if(cmd == NULL)
		{
			return -1;
		}
		*pcmd = cmd;
		cmd->base = *base;
		cmd->size = ntols(*pbuffer++);
		if ((cmd->LUNs = SNB_MALLOC(sizeof(uint16_t) * cmd->size)) == NULL)
			return -1;
		for(i = 0; i < cmd->size; i++)
		{
			cmd->LUNs[i] = ntols(pbuffer[i]);
		}
		return 0;
	}
	return 0;
}

snb_command_t* snb_command_list(snb_session_t* session)
{
	
	snb_command_list* cmd;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_list))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_LIST;
	cmd->base.type = ask;
	cmd->base.next = 0;
	cmd->base.length = SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_list_ack(snb_session_t* session, uint16_t * LUNs, uint16_t size, snb_command_rc_t rc)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_list_ack * cmd;

	if (LUNs == NULL || size == 0)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_list_ack))) == 0)
		return NULL;
	cmd->base->id = SNB_CMD_ID_LIST;
	cmd->base->type = answer;
	cmd->base->rc = rc;
	cmd->base->next = 0;
	cmd->size = size;
	cmd->LUNs = SNB_MALLOC(sizeof(uint8_t) * size);

	if(cmd->LUNs == NULL)
	{
		SNB_FREE(cmd)
		return NULL;
	}

	for(i = 0; i < size;i++)
	{
		cmd->LUNs[i] = LUNs[i];
	}

	plen += sizeof(cmd->size);
	plen += sizeof(LUNs[0]) * size;
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;

	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_info(uint16_t * LUNs, uint16_t size)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_info * cmd;

	if (LUNs == NULL || size == 0)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_info))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_INFO;
	cmd->base.type = ask;
	cmd->base.next = 0;
	cmd->size = size;
	cmd->LUNs = SNB_MALLOC(sizeof(uint8_t) * size);
	if(cmd->LUNs == NULL)
	{
		SNB_FREE(cmd);
		return NULL;
	}
	for(i = 0; i < size;i++)
	{
		cmd->LUNs[i] = LUNs[i];
	}

	plen += sizeof(cmd->size);
	plen += sizeof(LUNs[0]) * size;
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_info_ack(uint16_t size, uint16_t* LUNs,
	uint16_t* block_num, uint16_t* block_size, snb_command_rc_t rc)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_info_ack * cmd;

	if (LUNs == NULL || size == 0)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_info_ack))) == 0)
		return NULL;
	
	cmd->base.id = SNB_CMD_ID_INFO;
	cmd->base.type = answer;
	cmd->base.rc = rc;
	cmd->base.next = 0;
	cmd->size = size;
	cmd->LUNs = SNB_MALLOC(sizeof(uint8_t) * size);
	cmd->block_num = SNB_MALLOC(sizeof(uint8_t) * size);
	cmd->block_size = SNB_MALLOC(sizeof(uint8_t) * size);
	
	if(cmd->LUNs == NULL ||
		cmd->block_num == NULL ||
		cmd->block_size == NULL)
	{
		SNB_FREE(cmd->LUNs);
		SNB_FREE(cmd->block_num);
		SNB_FREE(cmd);
		return NULL;
	}

	for(i = 0; i < size;i++)
	{
		cmd->LUNs[i] = LUNs[i];
		cmd->block_num[i] = block_num[i];
		cmd->block_size[i] = block_size[i];
	}

	plen += sizeof(cmd->size);
	plen += sizeof(LUNs[0]) * size * 3;
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_ping(ping_type_t opt)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_ping * cmd;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_ping))) == 0)
		return NULL;

	cmd->base.id = SNB_CMD_ID_PING;
	cmd->base.type = ask;
	cmd->base.next = 0;
	cmd->type = opt;
	plen += sizeof(opt);
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_ping_ack(snb_command_rc_t rc)
{
	uint64_t plen = 0;
	snb_command_ping_ack_t * cmd;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_ping_ack_t))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_PING;
	cmd->base.type = answer;
	cmd->base.rc = rc;
	cmd->base.next = 0;
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_login(unsigned char *usr, unsigned char *passwd)
{
	snb_command_ping_ack * cmd;
	uint64_t plen = 0;
	uint8_t len1, len2;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_ping_ack))) == 0)
		return NULL;

	cmd->base.id = SNB_CMD_ID_LOGIN;
	cmd->base.type = ask;
	cmd->base.next = 0;
	len1 = strlen(usr);
	len2 = strlen(passwd);

	cmd->usr = SNB_MALLOC(len1 + 1);
	if(cmd->usr == NULL)
	{
		SNB_FREE(cmd);
		return NULL;
	}
	
	cmd->passwd = SNB_MALLOC(len2 + 1);
	if(cmd->passwd == NULL)
	{
		SNB_FREE(cmd->usr);
		SNB_FREE(cmd);
		return NULL;
	}

	strcpy(cmd->usr, usr);
	strcpy(cmd->passwd, passwd);
	plen += len1 + len2	
	plen += 2;//len1,len2
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_login_ack(snb_command_rc_t rc)
{
	snb_command_ping_ack * cmd;
	uint64_t plen = 0;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_ping_ack))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_LOGIN;
	cmd->base.type = answer;
	cmd->base.next = 0;
	cmd->base.rc = rc;

	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t*  snb_command_rw(uint16_t LUN, uint8_t * data, uint64_t offset,
	uint64_t size, uint8_t rw_mask)
{
	snb_command_rw_t * cmd;
	uint64_t plen = 0;

	if(SNB_IS_W_CMD(rw_mask) && data == NULL)
		return -1;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_rw_t))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_RW;
	cmd->base.type = ask;
	cmd->base.next = 0;
	cmd->rw_mask = rw_mask;
	cmd->offset = offset;
	cmd->size = size;
	cmd->LUN = LUN;
	if(SNB_IS_W_CMD(rw_mask))
	{
		cmd->data = SNB_MALLOC(sizeof(uint8_t) * size);
		if(cmd->data == NULL)
		{
			SNB_FREE(cmd)
			return NULL;
		}
		memcpy(cmd->data, data, size);
		plen += size * sizeof(*cmd->data);
	}

	plen += sizeof(cmd->rw_mask);
	plen += sizeof(cmd->offset);
	plen += sizeof(cmd->size);
	plen += sizeof(cmd->LUN);
	
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_command_rw_ack(uint16_t LUN, uint8_t * data, uint64_t offset,
	uint64_t size, uint8_t rw_mask, snb_command_rc_t rc)
{
	snb_command_rw_ack_t * cmd;
	uint64_t plen = 0;

	if(SNB_IS_R_CMD(rw_mask) && data == NULL)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_rw_ack_t))) == 0)
		return NULL;
	
	cmd->base.id = SNB_CMD_ID_RW;
	cmd->base.type = answer;
	cmd->base.rc = rc;
	cmd->base.next = 0;
	cmd->rw_mask = rw_mask;
	cmd->offset = offset;
	cmd->size = size;
	cmd->LUN = LUN;
	if(SNB_IS_R_CMD(rw_mask))
	{
		cmd->data = SNB_MALLOC(sizeof(uint8_t) * size);
		if(cmd->data == NULL)
		{
			SNB_FREE(cmd)
			return NULL;
		}
		memcpy(cmd->data, data, size);
		plen += size * sizeof(*cmd->data);
	}

	plen += sizeof(cmd->rw_mask);
	plen += sizeof(cmd->offset);
	plen += sizeof(cmd->size);
	plen += sizeof(cmd->LUN);
	return ((snb_command_t*)cmd);
}