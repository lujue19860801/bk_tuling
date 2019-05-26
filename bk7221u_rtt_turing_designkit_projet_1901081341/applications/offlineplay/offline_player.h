#ifndef _OFFLINE_PLAYER_H_
#define _OFFLINE_PLAYER_H_

#define OLP_DEBUG_PRTF               1

#define OLP_FATAL_PRINTF             rt_kprintf
#define OLP_WARNING_PRINTF           rt_kprintf
#define OLP_LOG_PRINTF(...)

#if OLP_DEBUG_PRTF
#define OLP_PRINTF                  rt_kprintf
#else
#define OLP_PRINTF(...)
#endif //OLP_DEBUG_PRTF


#define OFFLINE_PLAYER_KEY_BUTTON_DEBUG   1
#define OFFLINE_PLAYER_DEBUG              1
#define OFFLINE_VOLUME_DEFAULT_STEP      (5)

typedef enum
{
	OFFLINE_PLAYER_KEYHANDLE_PLAY_PAUSE = 1,
	OFFLINE_PLAYER_KEYHANDLE_MODE_CHANGE = 2,
	OFFLINE_PLAYER_KEYHANDLE_SONG_NEXT = 3,
	OFFLINE_PLAYER_KEYHANDLE_SONG_PREV = 4,
	OFFLINE_PLAYER_KEYHANDLE_VOLUME_ADD = 5,
	OFFLINE_PLAYER_KEYHANDLE_VOLUME_REDUCE = 6,
	OFFLINE_PLAYER_SD_STATUS_INSTER = 7,
	OFFLINE_PLAYER_SD_STATUS_REMOVE = 8,
	OFFLINE_PLAYER_LINEIN_STATUS_INSTER = 9,
	OFFLINE_PLAYER_LINEIN_STATUS_REMOVE = 10,
	OFFLINE_PLAYER_KEYHANDLE_CHANGE_DIR = 11,
	OFFLINE_PLAYER_KEYHANDLE_UNDEFINE = 0xFF,
}OfflinePlayerKeyHandleEvent;

void offline_player_key_button_event_handler(OfflinePlayerKeyHandleEvent event);
void offline_player_sd_status_change_event(int event);
int offline_player_init(void);
#endif
// eof

