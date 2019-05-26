#include "asr_client.h"
#include "asr_common.h"
#include "asr_config.h"


#if (ASR_SERVICE_TYPE==ASR_SERVICE_TULING)
DECLARE_ASR_CLIENT(CLIENT_TULING)
#endif

static void* recognize_proc(void *arg)
{
	asr_client_t* pClient = (asr_client_t*)arg;
	pcm_ring_buffer *ring_buf = &pClient->audio_rb;
	unsigned int nostream_cnt = 0,stream_cnt = 0;
	item_data *item;
	char dummy[TEN_MS_BYTES]={0};

	if(pClient->ops == NULL || pClient->ops->client_post == NULL)
		goto exit;
	while(pClient->is_running){
		while(pClient->recognize_flag == VOICE_SPEAKING){
			switch(pClient->recognize_state){
				case RECOGNIZE_IDLE:
					rt_thread_mdelay(10);
					break;
				case RECOGNIZE_START:
					stream_cnt = 0;
					item = robot_ring_buffer_peek(ring_buf);
					if(item){
						pClient->ops->client_post(item->buffer,item->length,VOICE_START,pClient->session_seq);
						
						pClient->recognize_state = RECOGNIZE_DOING;
						robot_ring_buffer_pop(ring_buf);
					}else{
						rt_thread_mdelay(5);
					}
					break;
				case RECOGNIZE_DOING:
					item = robot_ring_buffer_peek(ring_buf);
					if(item){
						stream_cnt += 1;
						pClient->ops->client_post(item->buffer,item->length,VOICE_SPEAKING,pClient->session_seq);
						
						robot_ring_buffer_pop(ring_buf);
						nostream_cnt = 0;
					}else{
						nostream_cnt ++;
						if(nostream_cnt>30 && stream_cnt>0){
							nostream_cnt = 0;
							pClient->recognize_flag = VOICE_OVER;
						}
						rt_thread_mdelay(10);
					}
					break;
				case RECOGNIZE_STOP:
					pClient->recognize_flag = VOICE_OVER;
					break;
				default:
					break;
			}
		}

		if(pClient->recognize_flag == VOICE_OVER){
			item = robot_ring_buffer_peek(ring_buf);
			while(item)
			{
				stream_cnt += 1;
				pClient->ops->client_post(item->buffer,item->length,VOICE_SPEAKING,pClient->session_seq);
				
				robot_ring_buffer_pop(ring_buf);
			
				item = robot_ring_buffer_peek(ring_buf);
			}
			stream_cnt += 1;
			pClient->ops->client_post(dummy,TEN_MS_BYTES,VOICE_OVER,pClient->session_seq);
			stream_cnt = 0;

			pClient->recognize_flag = VOICE_START;
			pClient->recognize_state = RECOGNIZE_IDLE;
		}else{
			rt_thread_mdelay(5);
		}
	}
exit:
	return NULL;
}


int asr_client_init(const char* config_file,ASR_CLIENT client_type,asr_client_t* asr_client,asr_message_callback cb)
{
	int result  = -1;
	
	if(!asr_client)
		goto exit;
	asr_client->client_type = client_type;
	asr_client->is_running = 1;
	asr_client->recognize_state = RECOGNIZE_IDLE;
	asr_client->recognize_flag = VOICE_START;


	switch(client_type){	
		case CLIENT_TULING:
			asr_client->ops = ASR_CLIENT(CLIENT_TULING);
			break;
		default:
			asr_client->ops = NULL;
			break;
	}

	result = robot_ring_buffer_init(&asr_client->audio_rb, ASR_AUDIO_RB_FRAMES);
	if(result!=0)
		goto exit;

    pthread_create(&asr_client->recognize_thread, NULL, recognize_proc, asr_client);

	if(asr_client->ops){
		asr_client->ops->client_init(config_file,cb);
	}
exit:
	return result;
}

int asr_client_deinit(asr_client_t* asr_client)
{
	int result = -1;
	if(!asr_client)
		goto exit;
	if(asr_client->ops){
		asr_client->ops->client_deinit();
	}
	asr_client->is_running = 0;
	asr_client->recognize_flag = VOICE_START;
	pthread_join(asr_client->recognize_thread, NULL);
	robot_ring_buffer_deinit(&asr_client->audio_rb);
exit:
	return result;
}

int asr_client_start_recognize(asr_client_t* asr_client,int session_seq)
{
	if(!asr_client)
		return -1;
	if(asr_client->is_running){
		asr_client->session_seq = session_seq;
		asr_client->recognize_flag = VOICE_SPEAKING;
		asr_client->recognize_state = RECOGNIZE_START;
	}
	return 0;
}

int asr_client_stop_recognize(asr_client_t* asr_client)
{
	if(!asr_client)
		return -1;
	if(asr_client->is_running){
		asr_client->recognize_state = RECOGNIZE_STOP;
	}
	return 0;
}

int asr_client_do_recognize(asr_client_t* asr_client,char *pData, int length)
{
	if(!asr_client)
		return -1;
	robot_ring_buffer_push(&(asr_client->audio_rb), pData, length);
	return 0;
}

void free_asr_scene_result(MwAsrSceneResult* pResult)
{
	int i;
	if(!pResult)
		return;
	switch(pResult->scene){
        case ASR_SCENE_CHAT:
            break;
		case ASR_SCENE_MUSIC:
			mw_free(pResult->sceneData.scene_music.prompt);
			mw_free(pResult->sceneData.scene_music.url);
			mw_free(pResult->sceneData.scene_music.title);
			mw_free(pResult->sceneData.scene_music.singer);
			break;
		case ASR_SCENE_CALL:
			mw_free(pResult->sceneData.scene_call.contact);
			mw_free(pResult->sceneData.scene_call.phoneNumber);
			mw_free(pResult->sceneData.scene_call.prompt);
			break;
		case ASR_SCENE_CLOCK:
			for(i=0;i<pResult->sceneData.scene_clock.num;i++){
				mw_free(pResult->sceneData.scene_clock.nodes[i].content);
			}
			mw_free(pResult->sceneData.scene_clock.nodes);
			mw_free(pResult->sceneData.scene_clock.prompt);
			break;
		case ASR_SCENE_VOLUME:
			mw_free(pResult->sceneData.scene_volume.prompt);
			break;
        case ASR_SCENE_SLEEP:
            break;
		case ASR_SCENE_OTHER:
			mw_free(pResult->sceneData.scene_common.prompt);
			mw_free(pResult->sceneData.scene_common.url);
			break;
		default:
			break;
	}
}

