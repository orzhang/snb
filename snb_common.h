#ifndef SNB_COMMON_H
#define SNB_COMMON_H
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#ifdef KERNEL
#define SNB_MALLOC malloc
#define SNB_FREE free
#define SNB_LOCK_MUTEX(x)
#define SNB_UNLOCK_MUTEX(x)
#else
#define SNB_MALLOC malloc
#define SNB_FREE free
#define SNB_LOCK_MUTEX(x)
#define SNB_UNLOCK_MUTEX(x)
#endif
struct snb_command;
struct snb_session;
typedef struct snb_command snb_command_t;
typedef struct snb_session snb_session_t;
typedef struct snb_msg_buf snb_msg_buf_t;
#endif