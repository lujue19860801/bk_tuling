#ifndef __ALERT_INTERFACE_H__
#define __ALERT_INTERFACE_H__
 
#include <stdlib.h>

#include "AlertManager.h"



void AlertAdd( char *token,  char *scheduledTime, char *repate, int type);
void AlertInit(void *data);
void AlertDeinit();
void AlertRemove(char *token);
void AlertFinshed();
void AlertLoad(void);
Alert *GetAlert(char *token);


#endif



