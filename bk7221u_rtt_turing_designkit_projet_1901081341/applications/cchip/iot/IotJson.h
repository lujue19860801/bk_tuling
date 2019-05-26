#ifndef __IOT_JSON_H__
#define __IOT_JSON_H__


#include "TuringIotDevice.h"
#include "TuringIotAudio.h"
#include "cJSON.h"


enum {	
	STATUS_DEVICE_TYPE = 0,
	STATUS_AUDIO_TYPE,
	STATUS_EMERG_TYPE,
	STATUS_MUSIC_TYPE,
	STATUS_STORY_TYPE,
};


char *CreateDeviceUnbindMessageReportJson(char *key, char *deviceId);
char *CreateIntercomMessageReportJson(char *key, char *deviceId, char *fromUser, char *mediaId);

char *CreateAudioStatusReportJson(char *key, char *deviceId, TuringAudioStatus *audioStatus);

char *CreateStatusGetJson(char *key, char *deviceId, int type);
char *CreateEmergStatusReportJson(char *key, char *deviceId, int battery);

char *CreateSynchronizeLocalMusicReportJson(char *key, char *deviceId, int operate,char *names);
char *CreateSynchronizeLocalStoryReportJson(char *key, char *deviceId, int operate,char *names);


char *CreateIotDataReportJson(char *key, char *deviceId);
char *CreateGetDataReportJson(char *key, char *deviceId, int type);
char *CreateTopicJson(char *key, char *deviceId);
char *CreateDeviceStatusReportJson(char *key, char *deviceId, TuringDeviceStatus  *deviceStatus);

char *CreateCollecSongJson(char *key, char *deviceId, int id);
char *CreateAudioGetJson(char *key, char *deviceId, int type);
char *CreateDeviceVendorAuthorizeJson(char *key, char *productId, char *deviceId, char *mac);
char *CreateDeviceVendorStatusQueryJson(char *key, char *deviceId);
void IotDeviceVendorAuthorize(void *pv, int type);
char *CreateDeviceVendorStatusQueryJson(char *key, char *deviceId);

char *CreateDeviceBindJson(char *key, char *deviceId, char *openId);

char *pJsonToString(cJSON *cjson);
void printJsonString(const char *pData, cJSON *cjson) ;

#endif

