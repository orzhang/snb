#ifndef SNB_COMMON_H
#define SNB_COMMON_H


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

typedef struct snb_command snb_command_t;
typedef struct snb_session snb_session_t;
#endif