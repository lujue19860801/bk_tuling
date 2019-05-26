

#ifndef __ALERTS_FILE_DATA_STORE_H__
#define __ALERTS_FILE_DATA_STORE_H__

#include "debug.h"
#include "AlertHead.h"

#define CRONTAB_ALARM_FILE  "/sdcard/alarms.json"


#define SECONDS_AFTER_PAST_ALERT_EXPIRES  1800


extern int crontab_wirte_to_disk(AlertManager *manager);


extern  int alertfiledatastore_wirte_to_disk(AlertManager *manager);
extern  int alertfiledatastore_load_from_disk(AlertManager *manager);


/* return 0 successd ,otherwise failed */
//void alertfiledatastore_load_from_disk(AlertManager *manager)
//extern int crontab_load_from_disk();
extern int crontab_load_from_disk(AlertManager *manager);

#endif




