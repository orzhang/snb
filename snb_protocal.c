
#include <snb_common.h>
#include <snb_protocal.h>
#include <snb_session.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int snb_pack_head(uint8_t* buffer, snb_command_t *pcmd);

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

int snb_pack_command(uint8_t * buffer, size_t size, snb_command_t * pcmd)
{
	if(size < pcmd->length)
		return -1;
	if(buffer == NULL)
		return -1;
	switch(pcmd->id)
	{
		case SNB_CMD_ID_LIST:
			if(snb_pack_head(buffer, pcmd) ||
				snb_pack_list_command(buffer + SNB_COMMAND_HEAD_SIZE, (snb_command_list_t*)pcmd))
			{
				return -1;
			}
			return 0;			
			break;
		case SNB_CMD_ID_INFO:
			if(snb_pack_head(buffer, pcmd) ||
				snb_pack_info_command(buffer + SNB_COMMAND_HEAD_SIZE, (snb_command_info_t*)pcmd))
			{
				return -1;
			}
			return 0;
			break;
		case SNB_CMD_ID_PING:
			if(snb_pack_head(buffer, pcmd) ||
				snb_pack_ping_command(buffer + SNB_COMMAND_HEAD_SIZE, (snb_command_ping_t*)pcmd))
			{
				return -1;
			}
			return 0;
			break;
		case SNB_CMD_ID_LOGIN:
			if(snb_pack_head(buffer, pcmd) ||
				snb_pack_login_command(buffer + SNB_COMMAND_HEAD_SIZE, (snb_command_login_t*)pcmd))
			{
				return -1;
			}
			return 0;
			break;
		case SNB_CMD_ID_RW:
			if(snb_pack_head(buffer, pcmd) ||
				snb_pack_rw_command(buffer + SNB_COMMAND_HEAD_SIZE, (snb_command_rw_t*)pcmd))
			{
				return -1;
			}
			return 0;
			break;
		default:
		return -1;
	}
	return -1;
}

int snb_pack_head(uint8_t * buffer, snb_command_t *pcmd)
{
	uint32_t * pbuffer_32;
	if(buffer == NULL || pcmd == NULL)
		return -1;
	buffer[0] = pcmd->id;
	buffer[1] = pcmd->type;
	buffer[2] = pcmd->proxy;
	buffer[3] = pcmd->rc;
	pbuffer_32 = (uint32_t*) &buffer[4];
	pbuffer_32[0] = htonl(pcmd->seq_num);
	pbuffer_32[1] = htonl(pcmd->length);
	return 0;
}

int snb_pack_rw_command(uint8_t * buffer, snb_command_t *pcmd)
{
	snb_command_rw_t * cmd = (snb_command_rw_t*)pcmd;
	uint8_t* pbuffer = buffer;
	uint16_t* pbuffer_16 = (uint16_t*)(buffer + 8);
	uint32_t* pbuffer_32 = (uint32_t*)(buffer);
	uint32_t pa_size = 0;

	if (buffer == NULL)
		return -1;

	if (pcmd->type == ask)
	{
		pbuffer_32[0] = ntohl(cmd->offset);
		pbuffer_32[1] = ntohl(cmd->size);
		pbuffer_16[0] = ntohs(cmd->LUN);
		pbuffer += 10;
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
		pbuffer_32[0] = ntohl(cmd->offset);
		pbuffer_32[1] = ntohl(cmd->size);
		pbuffer_16[0] = ntohs(cmd->LUN);
		pbuffer += 10;
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
	uint32_t pa_size = 0;
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
		pbuffer += 2;
		memcpy(pbuffer, cmd->usr, len1);
		pbuffer += len1;
		memcpy(pbuffer, cmd->passwd, len2);
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
	uint32_t pa_size = 0;
	uint16_t* pbuffer;
	int i = 0;

	if (buffer == NULL)
		return -1;
	
	if(pcmd->type == ask)
	{
		snb_command_info_t* cmd = (snb_command_info_t*)pcmd;
		
		pbuffer[0] = htons(cmd->size);
		pbuffer++;
		for (i = 0; i < cmd->size; i++)
		{
			pbuffer[i] = htons(cmd->LUNs[i]);
		}
		return 0;	
	}
	else
	{
		snb_command_info_ack_t* cmd = (snb_command_info_ack_t*)pcmd;
		pbuffer[0] = htons(cmd->size);
		pbuffer++;
		for (i = 0; i < cmd->size; i++)
		{
			pbuffer[i] = htons(cmd->LUNs[i]);
			pbuffer[i + cmd->size] = htons(cmd->block_num[i]);
			pbuffer[i + (cmd->size * 2)] = htons(cmd->block_size[i]);
		}
		return 0;
	}
	return -1;
}

int snb_pack_list_command(uint8_t * buffer, snb_command_t *pcmd)
{
	uint32_t pa_size = 0;
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
		pbuffer[0] = htons(cmd->size);
		pbuffer++;
		for (i = 0; i < cmd->size; i++)
		{
			pbuffer[i] = htons(cmd->LUNs[i]);
		}
		return 0;
	}
	return -1;
}

int snb_unpack_command(uint8_t * buffer, size_t size, snb_command_t ** pcmd)
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
	base.seq_num = ntohl(*((uint32_t *)(buffer + 4)));
	base.length = ntohl(*((uint32_t *)(buffer + 8)));
	pbuffer += 12;
	
	switch(base.id)
	{
		case SNB_CMD_ID_LIST:
			rc = snb_unpack_list_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_INFO:
			rc = snb_unpack_info_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_LOGIN:
			rc = snb_unpack_login_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_PING:
			rc = snb_unpack_ping_command(pbuffer, &base, &cmd);
			break;
		case SNB_CMD_ID_RW:
			rc = snb_unpack_rw_command(pbuffer, &base, &cmd);
		default:
			rc = -1;
		break;
	}
	*pcmd = cmd;
	return rc;
}

int snb_unpack_rw_command(const uint8_t * buffer, const snb_command_t * base,
	snb_command_t ** pcmd)
{
	uint32_t offset = 0, size = 0;
	uint8_t rw_mask = 0;
	uint32_t LUN = 0;
	uint8_t *pdata = NULL;
	uint8_t *rw_data = NULL;

	offset = ntohl(*((uint32_t*)(buffer)));
	size = ntohl(*((uint32_t*)(buffer + 4)));

	LUN = ntohs(*(buffer + 8));
	rw_mask = *(buffer + 10);
	pdata = (buffer + 11);

	if(base->type == ask)
	{
		snb_command_rw_t * cmd = SNB_MALLOC(sizeof(snb_command_rw_t));
		if(cmd == NULL)
			return -1;
		if(SNB_IS_W_CMD(rw_mask))
		{
			if ((rw_data = SNB_MALLOC(size) == NULL))
			{
				SNB_FREE(cmd);
				return -1;
			}
			memcpy(rw_data, pdata, size);
		}
		cmd->offset = offset;
		cmd->size = size;
		cmd->rw_mask = rw_mask;
		cmd->data = rw_data;
		*pcmd = cmd;
		return 0;
	}
	else
	{
		snb_command_rw_ack_t * cmd = SNB_MALLOC(sizeof(snb_command_rw_ack_t));
		if (cmd == NULL)
			return -1;

		if (SNB_IS_R_CMD(rw_mask))
		{
			if ((rw_data = SNB_MALLOC(size) == NULL))
			{
				SNB_FREE(cmd);
				return -1;
			}
			memcpy(rw_data, pdata, size);
		}
		cmd->offset = offset;
		cmd->size = size;
		cmd->rw_mask = rw_mask;
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
		cmd->passwd[len2] = '\0';

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
		snb_command_ping_ack_t * cmd = SNB_MALLOC(sizeof(snb_command_ping_ack_t));
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
	uint16_t* LUNs = NULL;
	uint16_t* block_num = NULL;
	uint16_t* block_size = NULL;
	uint16_t* pbuffer = buffer;
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
		snb_command_info_ack_t * cmd = SNB_MALLOC(sizeof(snb_command_info_ack_t));
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
			SNB_FREE(LUNs);
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
	int i = 0;
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
		cmd->size = ntohs(*pbuffer++);
		if ((cmd->LUNs = SNB_MALLOC(sizeof(uint16_t) * cmd->size)) == NULL)
			return -1;
		for(i = 0; i < cmd->size; i++)
		{
			cmd->LUNs[i] = ntohs(pbuffer[i]);
		}
		return 0;
	}
	return 0;
}

snb_command_t* snb_create_command_list()
{
	snb_command_list_t* cmd;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_list_t))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_LIST;
	cmd->base.type = ask;
	cmd->base.next = 0;
	cmd->base.length = SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_create_command_list_ack(uint16_t * LUNs, uint16_t size, snb_command_rc_t rc)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_list_ack_t * cmd;

	if (LUNs == NULL || size == 0)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_list_ack_t))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_LIST;
	cmd->base.type = answer;
	cmd->base.rc = rc;
	cmd->base.next = NULL;
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

snb_command_t* snb_create_command_info(uint16_t * LUNs, uint16_t size)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_info_t * cmd;

	if (LUNs == NULL || size == 0)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_info_t))) == 0)
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

snb_command_t* snb_create_command_info_ack(uint16_t size, uint16_t* LUNs,
	uint16_t* block_num, uint16_t* block_size, snb_command_rc_t rc)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_info_ack_t * cmd;

	if (LUNs == NULL || size == 0)
		return NULL;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_info_ack_t))) == 0)
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

snb_command_t* snb_create_command_ping(ping_type_t opt)
{
	int i = 0;
	uint64_t plen = 0;
	snb_command_ping_t * cmd;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_ping_t))) == 0)
		return NULL;

	cmd->base.id = SNB_CMD_ID_PING;
	cmd->base.type = ask;
	cmd->base.next = 0;
	cmd->type = opt;
	plen += sizeof(opt);
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_create_command_ping_ack(snb_command_rc_t rc)
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

snb_command_t* snb_create_command_login(unsigned char *usr, unsigned char *passwd)
{
	snb_command_login_t * cmd;
	uint64_t plen = 0;
	uint8_t len1, len2;
	if ((cmd = SNB_MALLOC(sizeof(snb_command_login_t))) == 0)
		return NULL;

	cmd->base.id = SNB_CMD_ID_LOGIN;
	cmd->base.type = ask;
	cmd->base.next = NULL;
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
	plen += len1 + len2;
	plen += 2;//len1,len2
	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t* snb_create_command_login_ack(snb_command_rc_t rc)
{
	snb_command_ping_ack_t * cmd;
	uint64_t plen = 0;

	if ((cmd = SNB_MALLOC(sizeof(snb_command_ping_ack_t))) == 0)
		return NULL;
	cmd->base.id = SNB_CMD_ID_LOGIN;
	cmd->base.type = answer;
	cmd->base.next = 0;
	cmd->base.rc = rc;

	cmd->base.length = plen + SNB_COMMAND_HEAD_SIZE;
	return ((snb_command_t*)cmd);
}

snb_command_t*  snb_create_command_rw(uint16_t LUN, uint8_t * data, uint32_t offset,
	uint32_t size, uint8_t rw_mask)
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
			SNB_FREE(cmd);
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

snb_command_t* snb_create_command_rw_ack(uint16_t LUN, uint8_t * data, uint32_t offset,
	uint32_t size, uint8_t rw_mask, snb_command_rc_t rc)
{
	snb_command_rw_ack_t * cmd;
	uint32_t plen = 0;

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
			SNB_FREE(cmd);
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

snb_msg_buf_t* snb_alloc_msg_buf(uint32_t size)
{
	snb_msg_buf_t* buf = malloc(sizeof(*buf));
	if(buf == NULL)
		return NULL;
	if ((buf->buf = malloc(size)) == NULL)
	{
		free(buf);
		return NULL;
	}
	buf->pbuf = buf->buf;
	buf->expect = 0;
	buf->state = SNB_MSG_RECV_STATE_HEAD;
	buf->size = size;
	return buf;
}

snb_msg_buf_t* snb_realloc_msg_buf(snb_msg_buf_t* buf, uint32_t size)
{
	snb_msg_buf_t* new_buf = NULL;
	if(buf == NULL)
		return NULL;
	if(buf->size >= size)
	{
		buf->pbuf = buf->buf;
		buf->expect = 0;
		buf->state = SNB_MSG_RECV_STATE_HEAD;
		return buf;
	}
	else
	{
		if((new_buf = malloc(sizeof(*buf))) == NULL)
			return NULL;
		if ((new_buf->buf = malloc(size)))
		{
			free(new_buf);
			return NULL;
		}
		new_buf->pbuf = buf->buf;
		new_buf->expect = 0;
		new_buf->state = SNB_MSG_RECV_STATE_HEAD;
		new_buf->size = size;
		free(buf->buf);
		free(buf);
		return new_buf;
	}
	return new_buf;
}

void snb_free_msg_buf(snb_msg_buf_t** buf)
{
	snb_msg_buf_t* tbuf;
	if(*buf == NULL)
		return;
	tbuf = *buf;
	free(tbuf->buf);
	tbuf->buf = NULL;
	free(tbuf);
	*buf = NULL;
}