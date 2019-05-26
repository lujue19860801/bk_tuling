#ifndef __TURING_WECHAT_LIST__
#define __TURING_WECHAT_LIST__

#include <list_player.h>
#include <time.h>

#define WCL_DEBUG_PRTF                     1

#define WCL_FATAL_PRINTF                   rt_kprintf
#define WCL_WARNING_PRINTF                 rt_kprintf
#define WCL_LOG_PRINTF(...)

#if WCL_DEBUG_PRTF
#define WCL_PRINTF                         rt_kprintf
#else
#define WCL_PRINTF(...)
#endif //WCL_DEBUG_PRTF

#define WCL_SUCCESS                       0
#define WCL_FAILURE                       1

typedef music_list_t WECHAT_LIST_T;
typedef struct _wechat_msg_t_
{
	struct music_info info;
	uint32_t status;
}WECHAT_MSG_T;

enum
{
	MSG_STATUS_RAW = 0x0,
	MSG_STATUS_PLAYED = 0x01
};

void wechat_list_deinit(void);
int wechat_list_init(void);
int wechat_list_play(void *pv);
uint32_t wechat_get_play_id(void);
int wechat_set_played_status(uint32_t id);
int wechat_have_raw_msg(void);
int wechat_msg_deinit(WECHAT_MSG_T *msg);
WECHAT_MSG_T * wechat_msg_init(const char *name,const char *url);
uint32_t wechat_get_oldest_raw_msg_id(void);
int wechat_list_add_url(const char *name,const char *url);
#endif
// eof
