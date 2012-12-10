#ifndef SNB_COMMON_H
#define SNB_COMMON_H
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#ifdef KERNEL
#define SNB_MALLOC(x) malloc(x)
#define SNB_SNB_FREE(x) SNB_FREE(x)
#define SNB_LOCK_MUTEX(x)
#define SNB_UNLOCK_MUTEX(x)
#else
#define SNB_MALLOC(x) malloc(x)
#define SNB_SNB_FREE(x) SNB_FREE(x)
#define SNB_LOCK_MUTEX(x)
#define SNB_UNLOCK_MUTEX(x)
#endif
struct snb_command;
struct snb_session;
typedef struct snb_command snb_command_t;
typedef struct snb_session snb_session_t;
#endif