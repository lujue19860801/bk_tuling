#ifndef __TURING_INTERRECV_H__
#define __TURING_INTERRECV_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <rtdef.h>
#include "MediaService.h"



typedef struct TuringInterRecvService { //extern from TreeUtility
    /*relation*/
    MediaService Based;
    /*private*/
    int wifiConnected;
    int toneEnable;
    int sdMounted;
} TuringInterRecvService;

TuringInterRecvService *TuringInterRecvServiceCreate();



#endif
