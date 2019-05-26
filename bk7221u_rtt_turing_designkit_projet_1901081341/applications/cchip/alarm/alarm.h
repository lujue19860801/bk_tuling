#ifndef ALARM_H_INCLUDED
#define ALARM_H_INCLUDED

#include "AlarmService.h"

#define ALA_DEBUG_PRTF                     0

#define ALA_FATAL_PRINTF                   rt_kprintf
#define ALA_WARNING_PRINTF                 rt_kprintf
#define ALA_LOG_PRINTF(...)

#if ALA_DEBUG_PRTF
#define ALA_PRINTF                         rt_kprintf
#else
#define ALA_PRINTF(...)
#endif //ALA_DEBUG_PRTF

int computeTimeDifference(alarm_info *node);
int alarmTrigger(alarm_info *node, int * p);
int updatePerAlarm(alarm_info *node);
int getNodeKey(alarm_info *node);
char *getAlarmMsg(alarm_info *node);

#endif // ALARM_H_INCLUDED
