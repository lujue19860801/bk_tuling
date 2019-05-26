#ifndef __TURING_POWER_H__
#define __TURING_POWER_H__

#include "MediaService.h"
typedef enum {
	TURING_POWER_STATUS_NONE,
	TURING_POWER_STATUS_COLLECT,	
}PowerStatus;
typedef enum {
	TURING_POWER_STATE_WORK = 0,
	TURING_POWER_STATE_STANDBY,
	TURING_POWER_STATE_SUSPEND,
}TuringPowerState;
	
typedef enum {
	TURING_POWER_EVENT_UPDATE = 0,
	TURING_POWER_EVENT_START,
	TURING_POWER_EVENT_STOP,
	TURING_POWER_EVENT_SUSPEND,
	TURING_POWER_EVENT_COLUMN,
	TURING_POWER_EVENT_COLLECT,
	TURING_POWER_EVENT_SD_MOUNTED,
	TURING_POWER_EVENT_SD_UNMOUNTED,
	TURING_POWER_EVENT_LOCK,
	TURING_POWER_EVENT_SHUTDOWN,
}TuringPowerEvent;

typedef struct TuringPowerService //extern from TreeUtility
{
    /*relation*/
    MediaService Based;
	int wifiConnected;
    int toneEnable;
    int sdMounted;
	int blocking;
} TuringPowerService;

TuringPowerService *TuringPowerServiceCreate();

int   IsTuringLocked(void *pv);
void TuringPowerQueueSend(int event);
static void TuringPowerSuspend();
void SetTuringPowerState(int state);
void SetTuringPowerStatus(int status);
int IsTuringCollecting();
int   IsOtaLocked(void *pv);
#endif