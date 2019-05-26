#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "asr_client.h"
#include "asr_common.h"
#include "mw_asr.h"
#include "mw_ringBuffer.h"
#include "cJSON.h"
#include "aiwifi.h"


#define REQUEST_REALTIME		(1)
#define REQUEST_TEN_MS			(5)
#define REQUEST_BLOCK_SIZE		(TEN_MS_BYTES*REQUEST_TEN_MS)

#define TULING_HOST 		"smartdevice.ai.tuling123.com"

typedef struct{
	int current_seq;
	int asr_type;
	char identify[64];
	#if (REQUEST_REALTIME==1)
	char request_data[REQUEST_BLOCK_SIZE];
	#endif
	int request_index;
	asr_message_callback message_cb;
	mw_ring_buffer buffer;
}tuling_client_t;

typedef struct{
	int code;
	int type;
	scene_parse_func scene_func;
}tuling_service_t;


static int is_init = 0;
static tuling_client_t tuling_client;

static tuling_client_t* get_tuling_client()
{
	return &tuling_client;
}

static void load_param(const char* config_file)
{
	char user_id_1[64] = "ai00000000000001";
	char aes_key_1[64] = "Qxi966291z7Qm0cW";
	char api_key_1[64] = "2ce8f6bde3a04e36a06dceea349e7f1e";
	aiwifi_init(AIWIFI_COMMON,user_id_1, aes_key_1, api_key_1, "");

	char user_id_2[64] = "ai00000000000001";
	char aes_key_2[64] = "6r282S33W3mn2f5y";
	char api_key_2[64] = "9e84f7f998664ee48749bdfbc2f29fcd";
	aiwifi_init(AIWIFI_TRANSLATE,user_id_2, aes_key_2, api_key_2, "");
	is_init = 1;
}

int operate_type_by_code(int operate_code)
{
	int code = MUSIC_OP_PLAY;
	switch(operate_code){
		case 1000:
		case 2005:
		case 2006:
			code = MUSIC_OP_PLAY;
			break;
		case 2002:
			code = MUSIC_OP_STOP;
			break;
		case 1200:
			code = MUSIC_OP_PAUSE;
			break;
		case 1300:
			code = MUSIC_OP_RESUME;
			break;
		case 3002:
			code = MUSIC_OP_REPEAT;
			break;
	}
	return code;
}

void scene_music_parse_tuling(cJSON* root,MwAsrSceneResult* pResult)
{
	cJSON* jfunc;
	cJSON* nlp_objs;
	cJSON* nlp_obj;
	cJSON* jobj;
	int num;

	pResult->sceneData.scene_music.in_album = 1;
	nlp_objs = cJSON_GetObjectItem(root,"nlp");
	if(nlp_objs){
		num = cJSON_GetArraySize(nlp_objs);
		if(num>0){
			nlp_obj = cJSON_GetArrayItem(nlp_objs, 0);
			if(nlp_obj->type==cJSON_String){
				pResult->sceneData.scene_music.prompt = mw_strdup(nlp_obj->valuestring);
			}
		}
	}
	jfunc = cJSON_GetObjectItem(root, "func");
	if(jfunc){
		jobj = cJSON_GetObjectItem(jfunc, "url");
		if(jobj&&jobj->type==cJSON_String){
			pResult->sceneData.scene_music.url = mw_strdup(jobj->valuestring);
		}
		jobj = cJSON_GetObjectItem(jfunc, "title");
		if(jobj&&jobj->type==cJSON_String){
			pResult->sceneData.scene_music.title = mw_strdup(jobj->valuestring);
		}
		jobj = cJSON_GetObjectItem(jfunc, "singer");
		if(jobj&&jobj->type==cJSON_String){
			pResult->sceneData.scene_music.singer = mw_strdup(jobj->valuestring);
		}
		jobj = cJSON_GetObjectItem(jfunc, "operate");
		if(jobj&&jobj->type==cJSON_Number){
			pResult->sceneData.scene_music.type = operate_type_by_code(jobj->valueint);
		}
	}
}


void scene_volume_parse_tuling(cJSON* root,MwAsrSceneResult* pResult)
{
	cJSON* jfunc;
	cJSON* jobj;
	cJSON* nlp_objs;
	cJSON* nlp_obj;
	int num;

	nlp_objs = cJSON_GetObjectItem(root,"nlp");
	if(nlp_objs){
		num = cJSON_GetArraySize(nlp_objs);
		if(num>0){
			nlp_obj = cJSON_GetArrayItem(nlp_objs, 0);
			if(nlp_obj->type==cJSON_String){
				pResult->sceneData.scene_volume.prompt = mw_strdup(nlp_obj->valuestring);
			}
		}
	}
	jfunc = cJSON_GetObjectItem(root, "func");
	if(jfunc){
		jobj = cJSON_GetObjectItem(jfunc, "operate");
		if(jobj&&jobj->type==cJSON_Number)
			pResult->sceneData.scene_volume.op = jobj->valueint>0?VOLUME_UP:VOLUME_DOWN;
		jobj = cJSON_GetObjectItem(jfunc, "arg");
		if(jobj && jobj->type==cJSON_Number)
			pResult->sceneData.scene_volume.param = jobj->valueint;
		jobj = cJSON_GetObjectItem(jfunc, "volumn");
		if(jobj && jobj->type==cJSON_Number){
			if(jobj->valueint == 1){
				pResult->sceneData.scene_volume.op = VOLUME_MAXIMIZE;
			}else if(jobj->valueint == -1){
				pResult->sceneData.scene_volume.op = VOLUME_MINIMIZE;
			}
		}
	}
}

void scene_common_parse_tuling(cJSON* root,MwAsrSceneResult* pResult)
{
	cJSON* jfunc;
	cJSON* jobj;
	cJSON* nlp_objs;
	cJSON* nlp_obj;
	int num;

	nlp_objs = cJSON_GetObjectItem(root,"nlp");
	if(nlp_objs){
		num = cJSON_GetArraySize(nlp_objs);
		if(num>0){
			nlp_obj = cJSON_GetArrayItem(nlp_objs, 0);
			if(nlp_obj->type==cJSON_String){
				pResult->sceneData.scene_common.prompt = mw_strdup(nlp_obj->valuestring);
			}
		}
	}
	jfunc = cJSON_GetObjectItem(root, "func");
	if(jfunc){
		jobj = cJSON_GetObjectItem(jfunc, "url");
		if(jobj&&jobj->type==cJSON_String)
			pResult->sceneData.scene_common.url = mw_strdup(jobj->valuestring);
	}
}

tuling_service_t tuling_service_table[]={
	{MUSIC_CODE,ASR_SCENE_MUSIC,scene_music_parse_tuling},
	{STORY_CODE,ASR_SCENE_MUSIC,scene_music_parse_tuling},
	{VOLUME_CODE,ASR_SCENE_VOLUME,scene_volume_parse_tuling},
	{SLEEP_CODE,ASR_SCENE_SLEEP,NULL},
};


void handle_asr_no_response()
{
	MwAsrErrorResult result;
	tuling_client_t* pClient = get_tuling_client();
	result.error = ASR_RESULT_NO_RESPONSE;
	if(pClient->message_cb){
		pClient->message_cb(ASR_STATE_ERROR,&result);
	}
}

void parse_tuling_response_as_asr(int aiwifi_index,const char* text,int play_always)
{
	cJSON* jroot = cJSON_Parse(text);
	cJSON* jtoken;
	cJSON* jcode;
	cJSON* jemotion;
	tuling_client_t* pClient = get_tuling_client();
	MwAsrSceneResult result;
	int i;

	int code = 0,status_code = 0;
	if(!jroot)
		return;
	jtoken = cJSON_GetObjectItem(jroot, "token");
	if(jtoken&&jtoken->type==cJSON_String){
		updateTokenValue(aiwifi_index,jtoken->valuestring);
	}
	jcode = cJSON_GetObjectItem(jroot, "code");
	if(jcode&&jcode->type==cJSON_Number){
		code = jcode->valueint;
		status_code = code;
		while(status_code>=100){
			status_code /= 100;
		}
		if(status_code==4){
			return;
		}
	}
	init_pst(&result);
	jemotion = cJSON_GetObjectItem(jroot,"emotion");
	result.scene = ASR_SCENE_OTHER;
	if(play_always)
		result.sceneContext.session_seq= -1;
	else
		result.sceneContext.session_seq = pClient->current_seq;
	if(jemotion&&jemotion->type==cJSON_Number)
		result.sceneContext.emotion = jemotion->valueint;
	for(i=0;i<sizeof(tuling_service_table)/sizeof(tuling_service_table[0]);i++){
		if (tuling_service_table[i].code == code){
			result.scene = tuling_service_table[i].type;
			if(tuling_service_table[i].scene_func){
				tuling_service_table[i].scene_func(jroot,&result);
			}
			break;
		}
	}
	if(result.scene == ASR_SCENE_OTHER){
		scene_common_parse_tuling(jroot,&result);
	}
	if(pClient->message_cb && result.scene > 0){
		pClient->message_cb(ASR_STATE_OK,&result);
		free_asr_scene_result(&result);
	}
}


int CLIENT_TULING_init(const char* config_file,asr_message_callback cb)
{
	tuling_client_t* pClient = get_tuling_client();
	init_pst(pClient);
	load_param(config_file);
	mw_ring_buffer_init(&pClient->buffer,REQUEST_BLOCK_SIZE*5);
	pClient->asr_type = 0;
	pClient->request_index = 0;
	pClient->message_cb = cb;
	return 0;
}

int CLIENT_TULING_post(char* data,int length,int flag,int session_seq)
{
	tuling_client_t* pClient = get_tuling_client();
	long content_len = 0;
	int channel;
	char* response = NULL;
	#if (REQUEST_REALTIME==0)
	uint8_t* pBuf = NULL;
	#endif
	channel = get_aiwifi_channel();
	switch(flag){
		case VOICE_START:
			pClient->current_seq = session_seq;
			memset(pClient->identify, 0, 64);
            get_rand_str(pClient->identify, 19);
			mw_ring_buffer_push(&pClient->buffer,data,length);
			pClient->request_index = 0;
			break;
		case VOICE_SPEAKING:
			mw_ring_buffer_push(&pClient->buffer,data,length);
			#if (REQUEST_REALTIME==1)
			if(mw_ring_buffer_content(&pClient->buffer)>=REQUEST_BLOCK_SIZE){
				pClient->request_index += 1;
				mw_ring_buffer_pop(&pClient->buffer, pClient->request_data, REQUEST_BLOCK_SIZE);
				socketRequest(channel,pClient->request_data, REQUEST_BLOCK_SIZE, pClient->asr_type, 1, pClient->request_index, pClient->identify, TULING_HOST, &response);
			}			
			#endif		
			break;
		case VOICE_OVER:	
			mw_ring_buffer_push(&pClient->buffer,data,length);
			#if (REQUEST_REALTIME==1)
			pClient->request_index += 1;
			content_len = mw_ring_buffer_content(&pClient->buffer);
			mw_ring_buffer_pop(&pClient->buffer, pClient->request_data, content_len);
			socketRequest(channel,pClient->request_data, content_len, pClient->asr_type, 1, -pClient->request_index, pClient->identify, TULING_HOST, &response);
			#endif
											
			if(response != NULL)
			{
				printf("Response:%s\n",response);
				parse_tuling_response_as_asr(channel,response,0);
				mw_free(response);
			}else{
				handle_asr_no_response();
			}
			break;
		default:
			break;
	}
	return 0;
}



int CLIENT_TULING_deinit()
{
	tuling_client_t* pClient = get_tuling_client();
	mw_ring_buffer_deinit(&pClient->buffer);
	aiwifi_deinit(AIWIFI_COMMON);
	aiwifi_deinit(AIWIFI_TRANSLATE);
	return 0;
}


asr_client_ops CLIENT_TULING_ops = {
	.client_init = CLIENT_TULING_init,
	.client_post = CLIENT_TULING_post,
	.client_deinit = CLIENT_TULING_deinit
};

