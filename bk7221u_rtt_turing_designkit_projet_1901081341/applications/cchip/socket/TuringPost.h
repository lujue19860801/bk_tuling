#ifndef __TURING_POST_H__
#define __TURING_POST_H__


#define TPOST_DEBUG_PRTF                     1

#define TPOST_WARNING_PRINTF                 rt_kprintf

#if TPOST_DEBUG_PRTF
#define TPOST_PRINTF                         rt_kprintf
#else
#define TPOST_PRINTF(...)
#endif //TPOST_DEBUG_PRTF

int TuringHttpPost(char *host, char *path, char *body, int type);
#endif

