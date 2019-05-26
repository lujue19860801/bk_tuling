#ifndef ASR_CLIENT_H_
#define ASR_CLIENT_H_
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


#include "ring_buffer.h"
#include "mw_asr.h"
#include "asr_client.h"
#include "cJSON.h"

#define   ASR_AUDIO_RB_FRAMES			30
#define 	TEN_MS_BYTES				(320)

typedef enum{
	CLIENT_ROBOO = 0,
	CLIENT_TULING,
	CLIENT_BAIDU
}ASR_CLIENT;

enum{
	RECOGNIZE_IDLE = 0,
	RECOGNIZE_START,
	RECOGNIZE_DOING,
	RECOGNIZE_STOP,
	RECOGNIZE_ERROR
};

enum{
	VOICE_START = 0,
	VOICE_SPEAKING,
	VOICE_OVER
};

typedef enum{
	TTS_CHINESE = 0,
	TTS_ENGLISH
}TTS_LANGUAGE;


typedef struct{
	int (*client_init)(const char* config_file,asr_message_callback cb);	
	int (*client_post)(char* data,int length,int flag,int session_seq);
	int (*client_deinit)();	
}asr_client_ops;

typedef struct{
	ASR_CLIENT client_type;
	uint8_t is_running;
	uint8_t recognize_flag;
	int recognize_state;
	int session_seq;
	pcm_ring_buffer audio_rb;
	pthread_t recognize_thread;
	asr_client_ops* ops;
}asr_client_t;

#define   DECLARE_ASR_CLIENT(client)	extern asr_client_ops client##_ops;
#define   ASR_CLIENT(client)			(&(client##_ops))


int asr_client_init(const char* config_file,ASR_CLIENT client_type,asr_client_t* asr_client,asr_message_callback cb);

int asr_client_deinit(asr_client_t* asr_client);

int asr_client_start_recognize(asr_client_t* asr_client,int session_seq);

int asr_client_stop_recognize(asr_client_t* asr_client);

int asr_client_do_recognize(asr_client_t* asr_client,char *pData, int length);

void free_asr_scene_result(MwAsrSceneResult* pResult);
typedef void (*scene_parse_func)(cJSON* root,MwAsrSceneResult* pResult);

#endif
