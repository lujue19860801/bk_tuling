

#ifndef __TURING_WE_CHAT_H__
#define __TURING_WE_CHAT_H__
#include <rtthread.h>
#include <rtdef.h>
#include "MediaService.h"

typedef enum {
	WECHAT_STATE_NONE = 0,
	WECHAT_STATE_ONGING,
	WECHAT_STATE_DONE,	
	WECHAT_STATE_CANCLE,	
}WeChatState;

typedef struct {
	int state;
	int vad;

	unsigned char *buf;
	int size;
	int len;
	int type;
	rt_mutex_t mux;
	rt_sem_t cond;
} WeChatControl;

typedef struct TuringWeChatService { //extern from TreeUtility
    /*relation*/
    /*private*/
	MediaService Based;
    int wifiConnected;
    int toneEnable;
    int sdMounted;
	int blocking;
} TuringWeChatService;

int    IsWeChatFinished(void);
void WaitForWeChatFinshed(void *pv);




void StartTuringWeChat();
void EndWechat(void *pv);


TuringWeChatService *TuringWeChatServiceCreate();




#endif




