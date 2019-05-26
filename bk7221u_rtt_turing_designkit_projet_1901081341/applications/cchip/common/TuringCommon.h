#ifndef __TURING_COMMON_H__
#define __TURING_COMMON_H__

#include <stdio.h>
#include <stdlib.h>

//#define      ENABLE_WRITE_FILE


#define ENABLE_CJSON

typedef int AudioStatus;     	//
typedef int	PlayerWorkingMode;
typedef int	PlayMode;
typedef enum {
	MUSIC_TYPE_PHONE = 0,
	MUSIC_TYPE_SERVER,
	MUSIC_TYPE_TTPOD,
	MUSIC_TYPE_MIGU,
	MUSIC_TYPE_LOCAL ,
}MusicType;
	
#define TURING_PRIMARY_PLAYLIST 1
#define TURING_LOCAL_PLAYLIST      2 //PLAYLIST_IN_CARD_COLL//
#ifdef ENABLE_WRITE_FILE
#define TURING_WECHAT_PLAYLIST 3// PLAYLIST_IN_CARD_02
#else
#define TURING_WECHAT_PLAYLIST 4// PLAYLIST_IN_CARD_02
#endif
#ifdef ENABLE_WRITE_FILE
#define TURING_SERVER_PLAYLIST 5// PLAYLIST_IN_CARD_02
#else
#define TURING_SERVER_PLAYLIST 6// PLAYLIST_IN_CARD_02
#endif
#ifdef ENABLE_WRITE_FILE
#define TURING_MUSIC_PLAYLIST      7//PLAYLIST_IN_CARD_03
#else
#define TURING_MUSIC_PLAYLIST      8//PLAYLIST_IN_CARD_03
#endif
#define      SYSTEM_PLAYLIST_DIR  "/sdcard/__system/"

#define PLAYLSIT_DIR "/sdcard/playlists/"

#define TURING_PLAYLIST_JSON 		"/dev/turingPlayList.json"
#define TURING_PLAYLIST_M3U 		"/dev/turingPlayList.m3u"


#endif