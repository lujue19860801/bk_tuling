#include "mw_ringBuffer.h"
#include "asr_common.h"
#include "aplayer.h"

static aplayer_t mPlayer;

aplayer_t* get_audio_player()
{
	return &mPlayer;
}

void free_play_item(void* pv)
{
	play_item* item = (play_item*)pv;
	if(item){
		mw_free(item->file);		
		mw_free(item->fileName);
		mw_free(item);
	}
}


static int play_media_file(aplayer_t *player,play_item* item)
{
	int offset;
	if(!player||!item)
		return -1;
	
	rt_kprintf("Start play uri :%s\n",item->file);
	player->playing_flag = 1;
	player->decode_over = 0;
    if(player->ecb){
	    player->ecb(PLAY_EVENT_START);
    }
	if(item->status_cb){
		item->status_cb(item,0,APLAYER_PLAYING);
	}
	
	if(player->playing_flag==0){
		if(item->status_cb){
			offset = aplayer_get_playing_offset()/1000;
			item->status_cb(item,offset,APLAYER_PAUSED);
		}
						
		if(player->ecb){
			aplayer_purge();
			player->ecb(PLAY_EVENT_CANCEL);
	    }
    }else{
    	if(item->status_cb){
			offset = aplayer_get_playing_offset()/1000;
			item->status_cb(item,offset,APLAYER_FINISHED);
		}
	    if(player->ecb){
			player->ecb(PLAY_EVENT_FINISH);
	    }
    }
	player->decode_over = 1;
	rt_kprintf("End play uri :%s\n",item->file);
	return 0;
}

static void play_media_url(aplayer_t *player,play_item* item)
{
}


static void* play_proc(void *arg)
{
	aplayer_t* player = (aplayer_t*)arg;

	while(player->is_running)
	{
	}
	return NULL;
}

static void* audio_task(void *arg)
{
	aplayer_t* player = (aplayer_t*)arg;
	while(player->is_running)
	{
		player->playing_item = (play_item*)sim_list_pop_data(&player->play_queue);
		if(player->playing_item){
			if(player->playing_item->type == MEDIA_TYPE_URL){		
				play_media_url(player,player->playing_item);
			}
			if(player->playing_item->type == MEDIA_TYPE_FILE){
				play_media_file(player,player->playing_item);
			}						
			
			free_play_item(player->playing_item);
			player->playing_item = NULL;
		}else{
			rt_thread_mdelay(10);
		}
	}
	return NULL;
}


int aplayer_init(aplayer_pcm_cb pcm_cb,aplayer_event_cb event_cb)
{
	aplayer_t* player = get_audio_player();
	init_pst(player);

	player->is_running = 1;
	player->decode_over = 1;
	sim_list_init(&player->play_queue, free_play_item);	

	pthread_create(&player->process_thread, NULL, audio_task, player);
	pthread_create(&player->play_thread, NULL, play_proc, player);
	player->cb = pcm_cb;
	player->ecb = event_cb;
	player->is_init = 1;
	return 0;
}

void aplayer_deinit()
{
	aplayer_t* player = get_audio_player();
	player->is_init = 0;
	player->is_running = 0;
	player->cb = NULL;
	player->ecb = NULL;
	player->decode_over = 1;
	player->block_flag = 0;	
	player->playing_flag = 0;
	pthread_join(player->play_thread, NULL);
	pthread_join(player->process_thread, NULL);
	mw_ring_buffer_deinit(&player->play_rb);
	sim_list_free(&player->play_queue);
}

long aplayer_get_playing_offset()
{
	return 0;
}

void aplayer_purge()
{
	aplayer_t* player = get_audio_player();

	mw_ring_buffer* rb = &player->play_rb;
	mw_ring_buffer_reset(rb);
}
