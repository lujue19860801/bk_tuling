
#ifndef __TURING_COLUMN_H__
#define __TURING_COLUMN_H__


typedef enum      {
	COLUMN_TYPE_NURSERY_RHYMES = 0,
	COLUMN_TYPE_SINOLOGY_CLASSIC,
	COLUMN_TYPE_BEDTIME_STORIES,
	COLUMN_TYPE_FAIRY_TALES, 
	COLUMN_TYPE_ENGLISH_ENLIGHTTENMENT,
	COLUMN_TYPE_COLLECTION,
	COLUMN_TYPE_MAX
} ColumeType;



#define COLUMN_NURSERY_RHYMES_DIR   				"nursery/"//"儿歌哆瑞咪"
#define COLUMN_SINOLOGY_CLASSIC_DIR 				"sinology/"//"国学经典"
#define COLUMN_BEDTIME_STORIES_DIR 					"bedtime/"//"睡前故事"
#define COLUMN_FAIRY_TALES_DIR 	 						"fairy/"//"童话故事"
#define COLUMN_ENGLISH_ENLIGHTTENMENT_DIR   "english/"//"英语ABC"
#define COLUMN_COLLECTION_DIR      					"coll/"//"收藏夹"


#define COLUMN_NURSERY_RHYMES_PLAYLIST   				"nursery"
#define COLUMN_SINOLOGY_CLASSIC_PLAYLIST  			"sinology"
#define COLUMN_BEDTIME_STORIES_PLAYLIST  				"bedtime"
#define COLUMN_FAIRY_TALES_PLAYLIST  	 					"fairy"
#define COLUMN_ENGLISH_ENLIGHTTENMENT_PLAYLIST 	"english"
#define COLUMN_COLLECTION_PLAYLIST       				"coll"

#define COLUMN_NURSERY_RHYMES_SPEECH   					"nursery"//"儿歌哆瑞咪"
#define COLUMN_SINOLOGY_CLASSIC_SPEECH  				"sinology"//"国学经典"
#define COLUMN_BEDTIME_STORIES_SPEECH  					"bedtime"//"睡前故事"
#define COLUMN_FAIRY_TALES_SPEECH  	 						"fairy"//"童话故事"
#define COLUMN_ENGLISH_ENLIGHTTENMENT_SPEECH   	"english"//"英语ABC"
#define COLUMN_COLLECTION_SPEECH       			   	"coll"//"收藏夹"

//#define TURING_PLAYLIST_JSON 		"/sdcard/playlists/turingPlayList.json"
//#define TURING_PLAYLIST_M3U 		"/sdcard/playlists/turingPlayList.m3u"
//#define CURRENT_PLAYLIST_JSON  "/sdcard/playlists/currentPlaylist.json"
//#define CURRENT_PLAYLIST_M3U 		"/sdcard/playlists/currentPlaylist.m3u"

#define CURRENT_PLAYLIST "currentPlaylist"
#define LOAF_PLAYLIST_M3U "turingPlayList"

int StartColumn(void *pv);

int ScanCollectionDir(void *pv);
int CreateColumnPlaylistFile(char *columnDir, char *file);
int UpdateLocalPlaylist(void *pv);
int   ColumnAudioInit();
void ColumnAudioDeinit();




#endif








