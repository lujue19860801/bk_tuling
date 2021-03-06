#ifndef _TURING_PLAYTIP_H_
#define _TURING_PLAYTIP_H_
#include "TuringCommon.h"
#include "MediaService.h"

typedef enum{
	TURING_PLAYTIP_WIFI_CONNECTED = 0,
	TURING_PLAYTIP_WIFI_DISCONNECTED,
	TURING_PLAYTIP_WIFI_CONNECT_FAIL,
	TURING_PLAYTIP_WIFI_CONNECT_SUCCESS,
	TURING_PLAYTIP_WIFI_CONNECT_TO_NETWORK,
	TURING_PLAYTIP_WIFI_CONNECTING,  // 5 
	TURING_PLAYTIP_DEVICE_BIND,
	TURING_PLAYTIP_WIFI_CAPTURE_BEGIN, 
	TURING_PLAYTIP_WIFI_WECHAT_BEGIN, 
	TURING_PLAYTIP_RECEIVE_WECHAT,
	TURING_PLAYTIP_PLAY_BOOT_MUSIC,  // 10
	TURING_PLAYTIP_BING_SUCCESSC, 
	TURING_PLAYTIP_UNBING, 
	TURING_INSERT_POWER,
	TURING_LOW_POWER,    // 14
	TURING_POWEROFF,
	TURING_SPEAKER_NOT_CONNECT,
	TURIN_PLAYTIP_WELCOMEBACK,
	TURIN_PLAYTIP_DATERESET,
	TURIN_PLAYTIP_UPGRADED,
	TURIN_PLAYTIP_UPGRADING,   // 20
	TURIN_PLAYTIP_LATEST_VERSION,
	TURING_PLAYTIP_WIFI_CAPTURE_END,
	TURIN_PLAYTIP_CHECK_FIRMWARE,
	TURING_PLAYTIP_RECEIVE_NETWORK_MSG, // 24
	TURING_PLAYTIP_COLLECT_START,
	TURING_PLAYTIP_COLLECT_ING,
	TURING_PLAYTIP_COLLECT_FAILE,
	TURING_PLAYTIP_COLLECT_SUCCESS,
	TURING_PLAYTIP_COLLECT_ALREADY,
	TURING_PLAYTIP_TFCARD_MODE,
	TURING_PLAYTIP_WIFI_MODE,
	TURING_PLAYTIP_NO_WECHAT_MSG,	
}TuringPlaytipEvent;

#define TIP_PLAYING              1
#define TIP_NOPLAY               0

typedef struct TuringPlaytipService { //extern from TreeUtility
   MediaService Based;   
} TuringPlaytipService;

typedef struct TuringPlaytip{ //extern from TreeUtility
    int type;
    char *tipname;
    char * path;
} TuringPlaytip;

extern int playtipflag;
int send_play_tip_event(int event);
int get_tip_status(void);
void WaitForTipPlayFinshed(void);
#endif
