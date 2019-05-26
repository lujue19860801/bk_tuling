#ifndef _MENU_LIST_H
#define _MENU_LIST_H

typedef enum{
	FLASH_PLAYER_KEYHANDLE_PLAY_PAUSE = 1,
	FLASH_PLAYER_KEYHANDLE_SONG_NEXT = 2,
	FLASH_PLAYER_KEYHANDLE_SONG_PREV = 3,
}FlashPlayerKeyHandleEvent;

void flash_player_enter(void);
#endif
