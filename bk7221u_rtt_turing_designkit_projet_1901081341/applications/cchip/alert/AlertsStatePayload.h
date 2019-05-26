#ifndef  __ALERTS_STATE_PAYLOAD_H__
#define  __ALERTS_STATE_PAYLOAD_H__

#include <stdlib.h>
#include <stdio.h>
#include "AlertHead.h"
#include "debug.h"
#include "TuringCommon.h"

#ifndef ENABLE_CJSON
#include <json-c/json.h>
#else
#include "cJSON.h"
#endif


extern AlertsStatePayload *alertsstatepayload_new(list_t *allAlerts ,list_t *activeAlerts);

extern list_t *alertsstatepayload_get_all_alerts(AlertsStatePayload *payload);

extern list_t *alertsstatepayload_get_active_alerts(AlertsStatePayload *payload);


extern void alertsstatepayload_free(AlertsStatePayload *payload);

#ifndef ENABLE_CJSON
extern      json_object * alertsstatepayload_to_allalerts(AlertsStatePayload *payload);
extern      json_object * alertsstatepayload_to_activealerts(AlertsStatePayload *payload);
#else
extern     cJSON * alertsstatepayload_to_allalerts(AlertsStatePayload *payload);
extern     cJSON * alertsstatepayload_to_activealerts(AlertsStatePayload *payload);
#endif
#endif




