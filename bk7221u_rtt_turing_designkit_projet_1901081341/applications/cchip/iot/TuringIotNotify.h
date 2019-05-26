#ifndef __TUNRING_IOT_NOTIFY_H__
#define __TUNRING_IOT_NOTIFY_H__
#include "TuringCommon.h"



#include "cJSON.h"
#include "MediaService.h"

enum {
	NOTIFY_DEVICE_STATUS_REPORT = 0 , // report device status;
	NOTIFY_DEVICE_STATUS_UNBUNDING, 
	NOTIFY_DEVICE_STATUS_BUNDING,  
};


int ParseNotifyData(cJSON *message , MediaService *service);







#endif

