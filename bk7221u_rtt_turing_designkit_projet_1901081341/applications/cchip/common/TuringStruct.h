#ifndef __TURING_STRUCT_H__
#define __TURING_STRUCT_H__



typedef struct TuringMusic
{
	char *pContentURL;
	char *pTitle;
	char *pArtist;
	char *pTip;

	char *pId;
	char *pAlbum;
	char *pCoverURL;

	int type;
}TuringMusic;
typedef struct _Turing_MusicList
{
	int Num;	
	int Index;			
	TuringMusic* *pMusicList;
}TuringMusicList;




int FreeTuringMusic(TuringMusic **ppmusic);
int FreeTuringMusicList(TuringMusicList **ppmusiclist);
TuringMusic * NewTuringMusic(void);



#endif

