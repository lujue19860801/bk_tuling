
#ifndef __TURING_LOCAL_H__
#define __TURING_LOCAL_H__

#include "TuringCommon.h"
#include "MediaService.h"

enum {
	LOCAL_STATE_NONE= 0,
	LOCAL_STATE_ONGING,
	LOCAL_STATE_DONE,
	LOCAL_STATE_CANCLE,
};


typedef enum{
	TURING_LOCAL_COLLECT = 0,
	TURING_LOCAL_COLUMN,
}TuringLocalEvent;

typedef struct TuringLocalService { //extern from TreeUtility
    /*relation*/
    MediaService Based;
    /*private*/
    int wifiConnected;
    int toneEnable;
    int sdMounted;
} TuringLocalService;

TuringLocalService *TuringLocalServiceCreate();



#endif








