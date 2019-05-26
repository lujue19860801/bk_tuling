#ifndef __WECHAT_TIMER_H__
#define __WECHAT_TIMER_H__
#include "AlarmService.h"
#include <time.h>

#define WECHAT_MAX_TIMER                  16

#define WCT_DEBUG_PRTF                     0

#define WCT_FATAL_PRINTF                   rt_kprintf
#define WCT_WARNING_PRINTF                 rt_kprintf
#define WCT_LOG_PRINTF(...)

#if WCT_DEBUG_PRTF
#define WCT_PRINTF                         rt_kprintf
#else
#define WCT_PRINTF(...)
#endif //WCT_DEBUG_PRTF

enum {
	TIMER_DELETE,
	TIMER_INIT,
	TIMER_PLAYING,
	TIMER_STOP,
};
	
enum {
	TIMER_TYPE_NORMAL,
	TIMER_TYPE_BED,
	TIMER_TYPE_WAKE,
	TIMER_TYPE_TIMEOFF,
	TIMER_TYPE_REMIND,
};
	
typedef struct {
	uint8_t status;
    uint8_t week_id;       /**< week id */
    alarm_handle handle[7]; /**< every wechat timer have 7 handle*/
	time_t dtime;
	char *prompt; 
	char *repeat; 
} wechat_timer;

typedef struct {
    uint8_t wake_status;       /**< Do Not Disturb status*/
	uint8_t bed_status;
    alarm_handle bed_time_hd;    /**< the bed time of DND timer handle*/
    alarm_handle wake_time_hd;    /**< the wake time of DND timer handle*/
	char *wake_prompt; 
	char *wake_repeat; 
	char *bed_prompt; 
	char *bed_repeat; 
	time_t wake_dtime;
	time_t bed_dtime;
}  DND_timer;

typedef struct {
    uint8_t status;       
    alarm_handle time_hd;    
	char *prompt; 
	char *repeat; 
	time_t dtime;
} SM_timer;

typedef struct {
	uint8_t status;
	uint8_t index;
    alarm_handle handle; /**< every wechat timer have 7 handle*/
	time_t dtime;
	uint8_t cycleType; 
} remind_timer;

enum{
	CYCLE_TYPE_ONE_SHOT = 0,
	CYCLE_TYPE_DAY,
	CYCLE_TYPE_WEEK
};

int AlarmAdd(int id, time_t dtime, char * repeat, char * prompt, int type);
int AlarmDelete(int id, int type);
int AlarmNew(int id, time_t dtime, char * repeat, char * prompt, int type);
int RemindNew(time_t dtime, int cycleType);
int RemindAdd(time_t dtime, int cycleType);
#endif
// eof

