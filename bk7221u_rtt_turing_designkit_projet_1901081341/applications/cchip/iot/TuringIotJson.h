#ifndef __TURING_IOT_JSON_H__
#define __TURING_IOT_JSON_H__

#include "TuringCommon.h"
#include "MediaService.h"

#define IOTJ_DEBUG_PRTF                     1

#define IOTJ_FATAL_PRINTF                   rt_kprintf
#define IOTJ_WARNING_PRINTF                 rt_kprintf
#define IOTJ_LOG_PRINTF(...)

#if IOTJ_DEBUG_PRTF
#define IOTJ_PRINTF                         rt_kprintf
#else
#define IOTJ_PRINTF(...)
#endif //IOTJ_DEBUG_PRTF

enum
{
    MQTT_MESSAGE_CHAT_TYPE = 0, //chat ,
    MQTT_MESSAGE_AUDIO_TYPE , //audio ,
    MQTT_MESSAGE_CONTROL_TYPE,
    MQTT_MESSAGE_NOTIFY_TYPE,
    MQTT_MESSAGE_NONE_TYPE ,
};

int ParseDataFromTuring(const char *json, void *service);
int ProcessIotJson(char *json, int type);
#endif

