#ifndef _AIRKISS_LAN_RAW_H_
#define _AIRKISS_LAN_RAW_H_

#define DEBUG_PRINTF     rt_kprintf("[airkiss_lan] ");rt_kprintf

#define malloc            rt_malloc
#define realloc           rt_realloc
#define free              rt_free

#define DEFAULT_LAN_PORT            (12476)
#define AIRKISS_LAN_BUF_LEN         (1024)

#endif // _AIRKISS_LAN_RAW_H_
// eof
