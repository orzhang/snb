#ifndef SNB_PROTOCAL_H
#define SNB_PROTOCAL_H

#define SNB_CMD_ID_LIST (0)
#define SNB_CMD_ID_INFO (1)
#define SNB_CMD_ID_PING (2)
#define SNB_CMD_ID_LOGIN (3)
#define SNB_CMD_ID_RW (4)

#define SNB_CMD_ID_RW_MASK_R (0x00)
#define SNB_CMD_ID_RW_MASK_W (0x01)
#define SNB_CMD_ID_RW_MAKS_FLUSH (0x02)
#define SNB_IS_W_CMD(x) ((x & SNB_CMD_ID_RW_MASK_W) == 1)
#define SNB_IS_R_CMD(x) ((x & SNB_CMD_ID_RW_MASK_R) == 0)
#define SNB_IS_FW_CMD(x) ((x & SNB_CMD_ID_RW_MASK_W) && (x & SNB_CMD_ID_RW_MAKS_FLUSH))

#define SNB_MSG_MAX_BUFFER (1024*1024*10)
#define SNB_MSG_RECV_STATE_HEAD 0
#define SNB_MSG_RECV_STATE_BODY 1
#define SNB_MSG_RECV_STATE_END 2
#define SNB_MSG_SEND_STATE_START 3 
#define SNB_MSG_SEND_STATE_TX 4
#define SNB_MSG_SEND_STATE_END 5

#define SNB_COMMAND_HEAD_SIZE (12)

#include <snb_common.h>

struct snb_msg_buf
{
	uint8_t* buf;
	uint8_t* pbuf;
	uint32_t expect;
	uint32_t size;
	int state;
};

typedef enum {
	ask = 0,
	answer
} command_type_t;

typedef enum {
	server = 0,
	device
} ping_type_t;

typedef enum {
	snb_sucess = 0,
	snb_no_device,
	snb_no_session,
	snb_login_error,
	snb_rw_error,
	snb_block_out_of_range,
	snb_no_privilage
} snb_command_rc_t;

struct snb_command {
	snb_session_t* parent;
	struct snb_command* next;
	uint8_t id;
	uint8_t type; 
	uint8_t proxy;
	snb_command_rc_t rc;
	uint32_t seq_num;
	uint32_t length;
};

#define SNB_COMMAND_HEAD_ATTR_ID(x) (*((uint8_t*)(x)))
#define SNB_COMMAND_HEAD_ATTR_TYPE(x) (*((uint8_t*)(x + 1)))
#define SNB_COMMAND_HEAD_ATTR_PROXY(x) (*((uint8_t*)(x + 2)))
#define SNB_COMMAND_HEAD_ATTR_RC(x) (*((uint8_t*)(x + 3)))
#define SNB_COMMAND_HEAD_ATTR_SEQNUM(x) (ntohl(*(uint32_t*)(x + 4)))
#define SNB_COMMAND_HEAD_ATTR_LENGTH(x) (ntohl(*(uint32_t*)(x + 8)))


typedef struct snb_command_no_params
{
	snb_command_t base;
} snb_command_no_params_t;

typedef snb_command_no_params_t snb_command_list_t;

typedef struct snb_command_list_ack {
	snb_command_t base;
	uint16_t size;
	uint16_t * LUNs;
} snb_command_list_ack_t;


typedef struct snb_command_info {
	snb_command_t base;
	uint16_t size;
	uint16_t *LUNs;
} snb_command_info_t;

typedef struct snb_command_info_ack {
	snb_command_t base;
	uint16_t size;
	uint16_t *LUNs;
	uint32_t *block_num;
	uint32_t *block_size;
} snb_command_info_ack_t;

typedef struct snb_command_ping {
	snb_command_t base;
	ping_type_t type;
} snb_command_ping_t;

typedef snb_command_no_params_t snb_command_ping_ack_t;


typedef struct snb_command_rw {
	snb_command_t base;
	uint32_t offset;
	uint32_t size;
	uint16_t LUN;
	uint8_t rw_mask;
	uint8_t * data;
} snb_command_rw_t;

typedef struct snb_command_rw snb_command_rw_ack_t;

typedef struct snb_command_login
{
	snb_command_t base;
	char * usr;
	char * passwd;
} snb_command_login_t;

typedef snb_command_no_params_t snb_command_login_ack_t;

snb_msg_buf_t* snb_alloc_msg_buf(uint32_t size);
void snb_free_msg_buf(snb_msg_buf_t** buf);
snb_msg_buf_t* snb_realloc_msg_buf(snb_msg_buf_t* buf, uint32_t size);

snb_command_t* snb_create_command_list();
snb_command_t* snb_create_command_list_ack(uint16_t * LUNs, uint16_t size, snb_command_rc_t rc);

snb_command_t* snb_create_command_info(uint16_t * LUNs, uint16_t size);
snb_command_t* snb_create_command_info_ack(uint16_t size, uint16_t* LUNs,
	uint16_t* block_num, uint16_t* block_size, snb_command_rc_t rc);

snb_command_t* snb_create_command_ping(ping_type_t opt);
snb_command_t* snb_create_command_ping_ack(snb_command_rc_t rc);

snb_command_t* snb_create_command_login(const char *usr, const char *passwd);
snb_command_t* snb_create_command_login_ack(snb_command_rc_t rc);

snb_command_t* snb_create_command_rw(uint16_t LUN, uint8_t * data, uint32_t offset, uint32_t size, uint8_t rw_mask);
snb_command_t* snb_create_command_rw_ack(uint16_t LUN, uint8_t * data, uint32_t offset,
	uint32_t size, uint8_t rw_mask, snb_command_rc_t rc);

int snb_pack_command(uint8_t* buffer, size_t size, snb_command_t * pcmd);
int snb_unpack_command(uint8_t* buffer, size_t size, snb_command_t ** pcmd);

#endif