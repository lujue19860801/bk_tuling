#ifndef _M_CHAT_H_
#define _M_CHAT_H_

#define MCHAT_DEBUG_PRTF               1

#define MCHAT_FATAL_PRINTF             rt_kprintf
#define MCHAT_WARNING_PRINTF           rt_kprintf
#define MCHAT_LOG_PRINTF(...)

#if MCHAT_DEBUG_PRTF
#define MCHAT_PRINTF                         rt_kprintf
#else
#define MCHAT_PRINTF(...)
#endif //MCHAT_DEBUG_PRTF

#define MCHAT_SUCCESS                 (0)
#define MCHAT_FAILURE                 (1)

enum{
	MCHAT_STOP = 0,
	MCHAT_ENTER = 1,
	MCHAT_KEEP_UP = 2
};

uint32_t mchat_start(void);
uint32_t mchat_continue(void);
uint32_t mchat_stop(void);
uint32_t mchat_is_otg(void);
#endif // _M_CHAT_H_
