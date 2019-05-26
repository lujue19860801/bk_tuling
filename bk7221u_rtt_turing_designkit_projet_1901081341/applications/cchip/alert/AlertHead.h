
#ifndef __ALERT_HEAD_H__
#define __ALERT_HEAD_H__

#include <stdlib.h>
#include <stdio.h>


#include "hashmap.h"
#include "hashset.h"
#include "mylist.h"
#include "AlertTimer.h"
#include <pthread.h>

#define ACTIVE_USE_LIST
//#define USE_FREERTOS

//typedef char * AlertType;

typedef enum {
	ALERT_PLAYING,
	ALERT_INTERRUPTED,
	ALERT_FINISHED,
}AlertState;

typedef enum {
	ALERT_TYPE_ALARM = 0,
	ALERT_TYPE_SLEEP,
	ALERT_TYPE_WAKEUP,
	ALERT_TYPE_TIMEOFF,
}AlertType;

typedef struct alert_handler_st {
	volatile AlertState alertState;
	pthread_mutex_t aMutex;
	pthread_t tid;	
	void *data;
}AlertHandler;


typedef struct alert_s{
	char *token;
	//int id;
	char *scheduledTime;
	char *repate;
	int type;
	int actived;
}Alert;

typedef struct alert_manager_s {
	AlertHandler *handler;
	map_t schedulers; //
#ifndef ACTIVE_USE_LIST
	map_t activeAlerts; //
#else
	list_t *activeAlerts;
#endif
	//hashset_t activeAlerts;  //active alerts token set
	pthread_t tid;
	pthread_mutex_t aMutex; 
	void *data;
}AlertManager;


typedef struct alert_scheduler_st {
	Alert *alert;
#ifndef USE_CRONTAB
	mytimer_t *timer;
#endif
	AlertManager *handler;
	pthread_mutex_t aMutex;
	int active;
}AlertScheduler;



typedef struct alerts_state_payload_st {
    list_t *allAlerts;
    list_t *activeAlerts;
} AlertsStatePayload;

#define SECONDS_AFTER_PAST_ALERT_EXPIRES  1800

#define ALARM_FILE  "/etc/crontabs/root"



#endif







