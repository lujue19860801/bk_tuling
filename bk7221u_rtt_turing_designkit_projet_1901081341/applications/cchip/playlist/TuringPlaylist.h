#ifndef __TURING_PLAYLIST_H__
#define __TURING_PLAYLIST_H__
#include "PlaylistStruct.h"
#include "TuringStruct.h"
#include <player_app.h>
#include <list_player.h>

enum {
	TURING_AUDIO_TYPE_WECHAT = 0,
	TURING_AUDIO_TYPE_SERVER,
	TURING_AUDIO_TYPE_LOCAL,
};

extern music_list_t Turing_Local_list;
extern int play_num ;

void TuringPlayListInit();
TuringMusic *GetTuringMusic(char *url);
int TuringPlaylistInsert(MusicItem *music, int play, void *pv);
int TuringPlaylistPlayUrl(char *filename, int playlistid, char *url, int progress, void *pv);
int TuringPlaylistPlayIndex(char *file, int playlistid, int index, int progress, void *pv) ;
int TuringLocalPlaylistCreate(void);
int IsTuringLocalPlaylistplaying(void );
void TuringLocalPlaylistplay(void );
int TuringLocalPlaylistadd_nextplay(const char *name,const char *url);
#endif
// eof

