
#ifndef __TURING_IOT_AUDIO_H__
#define __TURING_IOT_AUDIO_H__

#include "TuringCommon.h"

#include "cJSON.h"

typedef struct audio_status 
{
	int type; 		// 
	int id;
	char *title; 	
	char *mediaId;  //mediaId
	char *url;		//
	char *tip;		//
	unsigned int duration;   //
	int play;		//playstatus 0-stop;1-play;2-pause;4-resume;
	AudioStatus state;     	//
	unsigned int progress;	//
	PlayerWorkingMode workingMode;
	PlayMode playMode;
}TuringAudioStatus;

	

enum {	
	PLAY_STATUS_STOP = 0,  
	PLAY_STATUS_PLAY, 
	PLAY_STATUS_PAUSE, 
	PLAY_STATUS_RESUME, 
};


TuringAudioStatus * newTuringAudioStatus(void);
void freeTuringAudioStatus(TuringAudioStatus **status);
int TuringGetAudioInfo(void *pv, TuringAudioStatus *audioStatus);
int dupAudioStatus(TuringAudioStatus *dest, TuringAudioStatus *src);
TuringAudioStatus *JsonStrToTuringAudioStatus(char *data);
char *TuringAudioStatusToJsonStr(TuringAudioStatus *status);


int ParseAudioData(cJSON *message, void *service);







#endif



