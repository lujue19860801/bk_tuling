#ifndef __TURING_PLAYER_STATUS_H__
#define __TURING_PLAYER_STATUS_H__
#include "MediaService.h"


typedef struct TuringPlayerService //extern from TreeUtility
{
    /*relation*/
    MediaService Based;
} TuringPlayerService;

TuringPlayerService *TuringPlayerServiceCreate();

#endif