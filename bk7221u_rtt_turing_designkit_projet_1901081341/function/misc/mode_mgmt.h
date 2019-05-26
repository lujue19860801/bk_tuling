#ifndef _MODE_MGMT_H_
#define _MODE_MGMT_H_

#define MMGMT_DEBUG_PRTF               1

#define MMGMT_FATAL_PRINTF             rt_kprintf
#define MMGMT_WARNING_PRINTF           rt_kprintf
#define MMGMT_LOG_PRINTF(...)

#if MMGMT_DEBUG_PRTF
#define MMGMT_PRINTF                   rt_kprintf
#else
#define MMGMT_PRINTF(...)
#endif //MMGMT_DEBUG_PRTF

#define MMGMT_SUCCESS                 (0)
#define MMGMT_FAILURE                 (1)

typedef enum{
	MODE_UNKNOWN = 0x00,
	MODE_OFFLINE_PLAYER = 0x01,
	MODE_CLOUD_PLAYER = 0x02,
	MODE_FLASH_PLAYER = 0x03,
	MODE_FACTORY_TEST = 0x04,
	MODE_MAX_COUNT
}SYS_MODE_E;
	
enum{
	MODE_PHASE_UNCERTAIN = 0x00,
	MODE_PHASE_DECIDED = 0x03,
};	
	
enum{
	MODE_FLAG_NO_WIFI_CONNECT = 0x01,
	MODE_FLAG_WIFI_TRY_CONNECT = 0x02,
	MODE_FLAG_WIFI_ATTEMPTED_CONNECT
};

uint32_t mmgmt_initial_mode_confirm(void);
uint32_t mmgmt_set_mode(SYS_MODE_E md);
SYS_MODE_E mmgmt_get_mode(void);
uint32_t mmgmt_determine_initial_offline_mode(void);
uint32_t mmgmt_is_offline_mode(void);
uint32_t mmgmt_is_cloud_mode(void);
uint32_t mmgmt_is_flash_mode(void);
int mmgmt_swith_to_flash_mode(void);
uint32_t mmgmt_is_factory_test_mode(void);
uint32_t mmgmt_switch2_factory_test(void);
#endif // _MODE_MGMT_H_
