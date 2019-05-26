#ifndef _APLAYER_H_
#define _APLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sim_list.h"

enum{
	MEDIA_TYPE_FILE = 0,
	MEDIA_TYPE_URL
};
typedef enum{
	APLAYER_IDLE = 0,
	APLAYER_BUFFERING,
	APLAYER_PLAYING,
	APLAYER_PAUSED,
	APLAYER_FINISHED
}APLAYER_STATUS;

enum{
	PLAY_EVENT_START = 0,
	PLAY_EVENT_CANCEL,
	PLAY_EVENT_FINISH,	// decode finish
	PLAY_EVENT_NOT_START,
	PLAY_EVENT_PLAY_FINISH,
	PLAY_EVENT_CONTINUE,
};


typedef void (*play_status_cb)(void* userdata,int offset,int status);
typedef void(*aplayer_pcm_cb)(uint8_t *pcm_data, uint16_t pcm_len);
typedef void(*aplayer_event_cb)(int event);

typedef struct{
	int type;
	char* file;	
	char* fileName;
	
	play_status_cb status_cb;
}play_item;


typedef struct{
	uint8_t is_running;
	uint8_t is_init;
	uint8_t ready_flag;
	uint8_t playing_flag;	
	uint8_t decode_over;
	uint8_t block_flag;	

	aplayer_pcm_cb cb;
	aplayer_event_cb ecb;

	mw_ring_buffer play_rb;
	sim_list_s play_queue;
	pthread_t process_thread;
	pthread_t play_thread;
	play_item* playing_item;
}aplayer_t;



int aplayer_init(aplayer_pcm_cb pcm_cb,aplayer_event_cb event_cb);
void aplayer_deinit();
long aplayer_get_playing_offset();
void aplayer_purge();


#ifdef __cplusplus
}
#endif

#endif
