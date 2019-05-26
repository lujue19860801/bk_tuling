#ifndef __PLAYLIST_STRUCT_H__
#define __PLAYLIST_STRUCT_H__

typedef struct MusicItem
{
	char *pContentURL;
	char *pTitle;
	char *pArtist;
	char *pTip;

	char *pId;
	char *pAlbum;
	char *pCoverURL;
	//char *pLrcURL;

	int type;
}MusicItem;



void FreeMusicItem(void *music);
MusicItem * NewMusicItem(void);
int MusicItemCmp(void *cmp1, void *cmp2);
MusicItem *DupMusicItem(MusicItem *src);




#endif

