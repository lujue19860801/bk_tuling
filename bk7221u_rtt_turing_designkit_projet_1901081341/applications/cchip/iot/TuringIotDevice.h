#ifndef __TURING_IOT_DEVICE_H__
#define __TURING_IOT_DEVICE_H__
#include <stdio.h>
#include "MediaService.h"
typedef struct sleep_status
{
	int on;
	long bed;
	long wake;
}SleepStatus;

typedef struct device_status
{
	int vol; 			// volume  0 - 100
	int battery; 		// battery 0 - 100
	int sfree; 			// storagecurrent 
	int stotal;			// storagetotal 
	int shake; 			// shakeswitch
	int power;			// lowpowervalue
	int	bln;	 		// blnswitch;
	int play;			//playmode
	int lbi;
	int tcard;
	SleepStatus sleepStatus;
}TuringDeviceStatus;

void GetTuringDeviceStatus(TuringDeviceStatus *status, void  *pv);

#endif