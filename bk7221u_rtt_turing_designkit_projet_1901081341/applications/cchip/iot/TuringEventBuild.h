
#ifndef __TURING_EVENT_BUILD_H__
#define __TURING_EVENT_BUILD_H__

#include "TuringCommon.h"

#define KEB_DEBUG_PRTF                     1

#define KEB_WARNING_PRINTF                 rt_kprintf

#if KEB_DEBUG_PRTF
#define KEB_PRINTF                         rt_kprintf
#else
#define KEB_PRINTF(...)
#endif //KEB_DEBUG_PRTF

void IotGetTopic(void *pv, int type);
void IotAudioStatusReport(void *pv, int type);
void IotGetDataReport(void *pv, int type);
void IotWeChatReport(void *pv, int type);
void IotDeviceStatusReport(void *pv, int type);
void IotEmergStatusReport(void *pv, int type);
void IotGetAudio(void *pv, int type, int arg);
void IotCollectSong(void *pv, int id, int type);
void IotSynchronizeLocalMusicReport(int operate,char *names, int type);
void IotSynchronizeLocalStoryReport(int operate,char *names, int type);
void IotDeviceVendorStatusQuery(void *pv, int type);
void IotDeviceVendorAuthorize(void *pv, int type);
void IotDeviceBind(void *pv,char *openid, int type);
#endif
//  eof

