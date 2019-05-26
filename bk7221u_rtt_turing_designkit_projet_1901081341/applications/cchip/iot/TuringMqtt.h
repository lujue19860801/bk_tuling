#ifndef __TURING_MQTT_H__
#define __TURING_MQTT_H__
#include "TuringCommon.h"

typedef enum{

	TURING_MQTT_OFFLINE=0,
	TURING_MQTT_CONNECT,
	TURING_MQTT_ONLINE,
}TuringmqqttcherEvent;


int TuringMqttStart(void *pv);
void TuringMqttStop();
int get_mqtt_status(void);




#endif















