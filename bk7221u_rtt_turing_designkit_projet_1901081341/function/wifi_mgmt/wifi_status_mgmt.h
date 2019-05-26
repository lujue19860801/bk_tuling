#ifndef _WIFI_STATUS_MGMT_H_
#define _WIFI_STATUS_MGMT_H_

#define WFM_DEBUG_PRTF                           1

#if WFM_DEBUG_PRTF
#define WFM_PRINTF                               rt_kprintf
#else
#define WFM_PRINTF(...)
#endif //WFM_DEBUG_PRTF

#define CONNECT_FAILED_MAX_COUNT_OF_INITIAL_MODE       (4)

extern void wfm_callback_register(void);
extern uint32_t wfm_get_wifi_connect_status(void);
extern int wfm_assoc_fail_too_many_times(void);
extern void set_factory_ssid(uint32_t flag);
#endif // _WIFI_STATUS_MGMT_H_
// eof

