#ifndef __ALERT_TIME_CONVERT_H__
#define __ALERT_TIME_CONVERT_H__

#include <time.h>



time_t alert_convert_scheduledtime(char *scheduledTime);
time_t alert_convert_scheduledtime_s(char *scheduledTime);
void alert_convert_scheduledtime_tm(char *scheduledTime, struct tm **time);
unsigned long  alert_get_time_diff(char *time);

#endif