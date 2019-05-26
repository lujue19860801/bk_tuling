
#ifndef __ALERT_HANDLER_H__

#define __ALERT_HANDLER_H__

#include "AlertHead.h"



extern AlertHandler *alerthandler_new(void);

extern void alerthandler_start_alert(AlertHandler *handler, AlertManager *manager);
extern void alerthandler_stop_alert(AlertHandler *handler, AlertManager *manager);

extern AlertState alerthandler_get_state(AlertHandler *handler);
extern void  alerthandler_set_state(AlertHandler *handler, AlertState state);

extern void alerthandler_free(AlertHandler *handler);

extern int alerthandler_is_alarming(AlertHandler *handler);
extern  mytimer_t * alertscheduler_get_timer(AlertScheduler * alertScheduler);



#endif

