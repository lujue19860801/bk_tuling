#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>


#include "debug.h"
#include "AlertHead.h"

 extern time_t get_now_time();

 /* create a Alert  instance */
extern Alert * alert_new(char *token, char * scheduledTime ,char *repate, int type);




extern char *alert_get_token(Alert *alert);

extern char * alert_get_repate(Alert *alert);

extern char *alert_get_scheduledtime(Alert *alert);

///extern time_t alert_convert_scheduledtime(Alert *alert);

//extern void alert_convert_scheduledtime_tm(Alert *alert, struct tm **time);

extern time_t get_now_time();

extern int alert_get_type(Alert *alert);

extern int alert_equals(Alert *alert, Alert *otherAlert);


/* free  Alert   */
extern void alert_free(Alert *alert);





#endif
