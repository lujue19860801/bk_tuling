#ifndef _AI_WIFI_H_
#define _AI_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "asr_config.h"
#if (ASR_SERVICE_TYPE==ASR_SERVICE_TULING)
#include "cJSON.h"

#define  CHAT_CODE     		20000
#define  SLEEP_CODE		20001
#define  VOLUME_CODE		20002
#define  WEATHER_CODE		20003
#define  TIME_CODE		20004
#define  DATE_CODE 		20005
#define  CALC_CODE		20006
#define  MUSIC_CODE		20007
#define  STORY_CODE		20008
#define  POETRY_CODE		20009
#define  AITICLE_CODE		20010
#define  ANIMAL_CODE		20011
#define  WIKI_CODE		20012
#define  PHONE_CODE		20013
#define  SOUND_CODE		20014
#define  TRANSLATE_CODE	20015
#define DANCE_CODE		20016
#define ENGLISH_CODE		20018
#define INSTRUMENT_CODE	20019
#define NATURE_CODE		20020
#define BRIGHTNESS_CODE	20021
#define BATTARY_CODE		20022
#define ROBOT_CODE		20023
#define PHOTO_CODE		20024
#define  ALARM_CODE             20025
#define APPLICATION_CODE	20026
#define FAQ_CODE			20027
#define  GOSSIP_CODE		29998
#define  PROMPT_CODE		29999

enum{
	AIWIFI_COMMON = 0,
	AIWIFI_TRANSLATE,
	AIWIFI_END
};

typedef struct{
	int operate;
	int argument;
}volume_func;

typedef struct{
	char* title;
	char* singer;
	char* url;
}music_func;

typedef struct{
	char* title;
	char* author;
	char* url;
}story_func;

typedef struct{
	char* title;
	char* author;
}poetry_func;

typedef struct{
	char* animal;
	char* url;
}animal_func;

typedef struct{
	char* name;
	char* phone_number;
}telephone_func;

typedef union{
	volume_func volume;
	music_func music;
	story_func story;
	poetry_func poetry;
	animal_func animal;
	telephone_func telephone;
}tuling_func_body;

typedef struct{
	int code;
	tuling_func_body content;
}tuling_func_content;

int aiwifi_init(int aiwifi_index,const char *userId,const char *aes_key,const char *api_key,const char *token);
void aiwifi_deinit(int aiwifi_index);
int get_aiwifi_channel();
void updateTokenValue(int aiwifi_index,const char *token);

void set_aiwifi_channel(int channel);
void get_rand_str(char s[], int number);
void socketRequest(int aiwifi_index,char *file_data, int len, int asr_type, int realtime, int index, char *identify, const char *host,char** response);

#endif

#ifdef __cplusplus
}
#endif

#endif
