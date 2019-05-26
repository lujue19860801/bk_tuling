#ifndef _TURING_DISPATCHER_H_
#define _TURING_DISPATCHER_H_
#include "TuringCommon.h"
#include "MediaService.h"
//#include <rthw.h>
//#include <rtthread.h>
//#include <rtdevice.h>

typedef enum{
	TURING_DISPATCHER_WIFI_CONNECTED = 0,
	TURING_DISPATCHER_REQUEST_TOPIC ,
	TURING_DISPATCHER_REPORT_DEVICE_STATUS,
	TURING_DISPATCHER_REPORT_AUDIO_STATUS,
	TURING_DISPATCHER_REPROT_IOT_STATUS,
	TURING_DISPATCHER_UPLOAD_FILE,
	TURING_DISPATCHER_SEND_MESSAGE,
	TURING_DISPATCHER_GET_AUDIO,
	TURING_DISPATCHER_GET_DATA,
	TURING_DISPATCHER_GET_STAUTS,
	TURING_DISPATCHER_MQTT_START,
	TURING_DISPATCHER_AIRDIS_START,
	TURING_DISPATCHER_VENDOR_AUTHORIZE,
	TURING_DISPATCHER_QUERY_DEVICE_STATUS,
	TURING_DISPATCHER_COLLECT_SONG,
	TURING_DISPATCHER_DEVICE_BIND,
	TURING_DISPATCHER_NEXT_SONG,
	TURING_DISPATCHER_PREV_SONG,
	TURING_DISPATCHER_GET_MQTT_INFO,
}TuringDispatcherEvent;

typedef struct DispatcherMsg {
		int type;
		char *data;
} DispatcherMsg;


void TuringDispatcherQueueSend(int event);


typedef struct TuringDispatherService { //extern from TreeUtility
    /*relation*/
    MediaService Based;
    /*private*/
    int wifiConnected;
    int toneEnable;
    int sdMounted;
	int blocking;
} TuringDispatherService;


int TuringMenuPlaylistCreate(void);
int TuringLocalPlaylistCreate(void);
int TuringMenuPlaylistadd(const char *name,const char *url);

TuringDispatherService *TuringDispatcherServiceCreate();
int startplayBootMusic();

int	stopplayBootMusic();
//void dispatcher_do_work(struct rt_work* work);

#endif

