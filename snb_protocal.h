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
#define SNB_IS_W_CMD (x) ((x & SNB_CMD_ID_RW_MASK_W) == 1)
#define SNB_IS_R_CMD (x) ((x & SNB_CMD_ID_RW_MASK_R) == 0)
#define SNB_IS_FW_CMD (x) ((x & SNB_CMD_ID_RW_MASK_W) && (x & SNB_CMD_ID_RW_MAKS_FLUSH))

#define SNB_COMMAND_HEAD_SIZE (20)

#include <snb_common.h>

typedef enum command_type {
	ask = 0,
	answer = 1
} command_type_t;

typedef enum ping_type {
	server = 0,
	device = 1
} ping_type_t;

typedef enum snb_command_rc {
	snb_sucess = 0,
	snb_no_device = 1,
	snb_no_session = 2,
	snb_login_error = 3,
	snb_rw_error = 4,
	snb_block_out_of_range = 5
} snb_command_rc_t;

typedef struct snb_command {
	struct snb_session_t* parent;
	struct snb_command* next;
	uint8_t id;
	uint8_t type; 
	uint8_t proxy;
	snb_command_rc_t rc;
	uint64_t seq_num;
	uint64_t length;
} snb_command_t;

typedef struct snb_command_no_params
{
	snb_command_t base;
} snb_command_no_params_t;

typedef snb_command_no_params_t snb_command_list_t;

typedef struct snb_command_list_ack {
	snb_command_t base;
	uint16_t size;
	uint16_t * LUNs;
} snb_pdu_list_ack_t;


typedef struct snb_command_info {
	snb_command_t base;
	uint16_t size;
	uint16_t *LUNs;
} snb_command_info_t;

typedef struct snb_command_info_ack {
	snb_command_t base;
	uint16_t size;
	uint16_t *LUNs;
	uint64_t *block_num;
	uint64_t *block_size;
} snb_command_info_ack_t;

typedef struct snb_command_ping {
	snb_command_t base;
	ping_type_t type;
} snb_command_ping_t;

typedef snb_command_no_params_t snb_command_ping_ack_t;


typedef struct snb_command_rw {
	snb_command_t base;
	uint64_t offset;
	uint64_t size;
	uint16_t LUN;
	uint8_t rw_mask;
	uint8_t * data;
} snb_command_rw_t;

typedef snb_command_rw_t snb_command_rw_ack_t;

typedef struct snb_command_login
{
	snb_command_t base;
	char * usr;
	char * passwd;
} snb_command_login_t;

typedef snb_command_no_params_t snb_command_login_ack_t;

snb_command_t* snb_command_list();
snb_command_t* snb_command_list_ack(uint16_t * LUNs, uint16_t size, snb_command_rc_t rc);

snb_command_t* snb_command_info(uint16_t * LUNs, uint16_t size);
snb_command_t* snb_command_info_ack(uint16_t size, uint16_t* LUNs,
	uint16_t* block_num, uint16_t* block_size, snb_command_rc_t rc);

snb_command_t* snb_command_ping(ping_type_t opt);
snb_command_t* snb_command_ping_ack(snb_command_rc_t rc);

snb_command_t* snb_command_login(unsigned char *usr, unsigned char *passwd);
snb_command_t* snb_command_login_ack(snb_command_rc_t rc);

snb_command_t* snb_command_rw(uint8_t * data, uint64_t offset, uint64_t size, uint8_t rw_mask);
snb_command_t* snb_command_rw_ack(snb_command_rc_t rc);


int snb_session_push_command(snb_session_t* session, snb_command_t* cmd);
snb_command_t* snb_session_pop_command(snb_session_t* session);

int snb_pack_command(uint8_t ** buffer, snb_command_t * pcmd);
int snb_unpack_command(uint8_t * buffer, snb_command_t ** pcmd);
#endif