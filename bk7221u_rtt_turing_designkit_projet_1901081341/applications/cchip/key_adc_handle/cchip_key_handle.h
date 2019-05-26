#ifndef _TURING_KEYHANDLE_H_
#define _TURING_KEYHANDLE_H_
#include "TuringCommon.h"
#include "MediaService.h"
#include "TuringDispatcher.h"

typedef enum
{
    TURING_KEYHANDLE_PLAY = 0,
    TURING_KEYHANDLE_PAUSE,
    TURING_KEYHANDLE_CAPTURE_START,
    TURING_KEYHANDLE_CAPTURE_END,
    TURING_KEYHANDLE_WECHAT_START,
    TURING_KEYHANDLE_WECHAT_END,
    TURING_KEYHANDLE_SONG_NEXT,
    TURING_KEYHANDLE_SONG_PREV,
    TURING_KEYHANDLE_VOLUME_ADD,
    TURING_KEYHANDLE_VOLUME_REDUCE,
    TURING_KEYHANDLE_VOLUME_ADJUST,
    TURING_KEYHANDLE_NETWORK_CONNECTTING,
    TURING_KEYHANDLE_MENU,
    TURING_KEYHANDLE_001,
    TURING_KEYHANDLE_MODE_CHANGE,
    TURING_KEYHANDLE_TF_STATUS_ON,
    TURING_KEYHANDLE_TF_STATUS_OFF,
    TURING_KEYHANDLE_SHUTDOWN,
    TURING_KEYHANDLE_POWER_OFF,
    TURING_KEYHANDLE_BAT_EVENT,
    TURING_KEYHANDLE_COLLECT_EVENT,
    TURING_KEYHANDLE_WECHAT_EVENT,

} TuringKeyHandleEvent;

typedef struct Turingkeyhandle  //extern from TreeUtility
{
    int type;
    char *name;
} Turingkeyhandle;

typedef struct TuringKeyhandleService   //extern from TreeUtility
{
    MediaService Based;

    int wifiConnected;
    int toneEnable;
    int sdMounted;
} TuringKeyhandleService;

int send_key_handle_event(int event);
#endif

