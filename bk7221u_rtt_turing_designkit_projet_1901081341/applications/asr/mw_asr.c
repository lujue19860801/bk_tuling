#include <pthread.h>
#include <sys/time.h>
#include "asr_common.h"
#include "asr_config.h"
#include "mw_asr.h"
#include "asr_client.h"
#include "mw_ringBuffer.h"
#include "mw_asrVad.h"

enum
{
    VAD_BEGIN,
    VAD_END
};

#define   TEN_MS_PROCESS      		2
#define   VAD_FRAME_SIZE			160
#define   PROCESS_SAMPLES   		(TEN_MS_PROCESS * VAD_FRAME_SIZE)
#define   TEN_MS_SIZE				(VAD_FRAME_SIZE * sizeof(int16_t))

#define 	MAX_SPEECH_FRMS				(1000)
#define 	MIN_SPEECH_FRMS				(100)
#define 	MIN_SILENCE_FRMS				(10)

#define  	ONE_SESSION_TIMEOUT			(5)
#define		ALL_SESSION_TIMEOUT			(60)
#define  	AUDIO_CAPTURE_BUFSIZE     		(1600)

typedef struct{
	uint8_t asr_init;
	uint8_t is_running;
	uint8_t wakeup_flag;
	uint8_t multiturn;

	uint8_t vad_flag;
	uint8_t last_vad;
	uint8_t session_flag;
	uint8_t in_session;
	uint8_t busy_flag;
	uint8_t allow_multiturn;

	int session_seq;
	int idle_ms;
	int silence_cnt;
	int speech_cnt;
	time_t wakeup_time;
	time_t active_time;
	time_t current_time;

	pthread_t audio_thread;
	pthread_t process_thread;
	pthread_mutex_t session_mutex;
	mw_ring_buffer capture_rb;
    asr_client_t asr_client;
	void* pAsrVad;
	MwAsrClientCbs callbacks;
}MwAsrContext;

static MwAsrContext asr_context;

static MwAsrContext* getAsrContext()
{
	return &asr_context;
}

void end_session(int active_flag,int multiturn_stop)
{
	MwAsrContext* pRobot = getAsrContext();
	pthread_mutex_lock(&pRobot->session_mutex);
	if(active_flag){
		pRobot->active_time = pRobot->current_time;
	}
	if(multiturn_stop)
		pRobot->multiturn = 0;
	pRobot->wakeup_flag = 0;
	pRobot->session_flag = 0;
	pRobot->session_seq += 1;
	pRobot->in_session = 0;
	pRobot->speech_cnt = 0;
	
	pthread_mutex_unlock(&pRobot->session_mutex);
}

static long long GetTimeSec(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec;
}


void set_wakeup_state()
{
	MwAsrContext* pRobot = getAsrContext();
	pthread_mutex_lock(&pRobot->session_mutex);
	pRobot->wakeup_flag = 1;
	if(pRobot->allow_multiturn){
		pRobot->multiturn = 1;
	}
	pRobot->in_session = 0;
	pRobot->session_seq += 1;
	pRobot->wakeup_time = pRobot->active_time = pRobot->current_time = GetTimeSec();
	pthread_mutex_unlock(&pRobot->session_mutex);
}

void audio_StatusChanged(void *userp, int type)
{
    MwAsrContext* pRobot = getAsrContext();
    switch(type)
    {
        case VAD_BEGIN:
            asr_client_start_recognize(&pRobot->asr_client,pRobot->session_seq);
            break;

        case VAD_END:
            asr_client_stop_recognize(&pRobot->asr_client);	    
            break;
    }
}


static void* audio_proc(void *arg)
{
	long content_bytes;
    char buffer[PROCESS_SAMPLES * sizeof(int16_t)]; 	
    int i, vad;    	
    MwAsrContext *pRobot = (MwAsrContext*)arg;		
	short echobuf[VAD_FRAME_SIZE];
    mw_ring_buffer *capture_rb = &pRobot->capture_rb;
	static int frms = 0;

	while (pRobot->is_running){
		while (pRobot->is_running && (content_bytes = mw_ring_buffer_content(capture_rb)) < PROCESS_SAMPLES * sizeof(int16_t))
        {
            rt_thread_mdelay(10);
        }

        if (!pRobot->is_running)
        {
            break;
        }
		mw_ring_buffer_pop(capture_rb, buffer, PROCESS_SAMPLES * sizeof(int16_t));
		for(i = 0; i < TEN_MS_PROCESS; i++){
			AsrVadProcess(pRobot->pAsrVad, (int16_t *)(buffer + TEN_MS_SIZE * i), echobuf, VAD_FRAME_SIZE,&vad);
			
			if(0 == vad)
            {
                pRobot->silence_cnt += 1;

                if(pRobot->silence_cnt > MIN_SILENCE_FRMS)
                {
                    pRobot->vad_flag = 0;
                }
            }
            else
            {
                pRobot->vad_flag = 1;
                pRobot->silence_cnt = 0;
            }

			if(pRobot->vad_flag && pRobot->wakeup_flag)                        		            
            {
				pRobot->session_flag = 1;
				pRobot->speech_cnt++;						
				if(pRobot->asr_init){						
					asr_client_do_recognize(&pRobot->asr_client, (char *)echobuf, TEN_MS_SIZE);						
				}					
                		
                if(pRobot->in_session == 0)
                {
                    pRobot->in_session ++;
                    audio_StatusChanged(pRobot, VAD_BEGIN);
                    printf("Speech start\n");
                }
            }else if(!pRobot->vad_flag && pRobot->speech_cnt>=MIN_SPEECH_FRMS)
            {
            	pRobot->speech_cnt = 0;
		        pRobot->wakeup_flag = 0;				   
		        audio_StatusChanged(pRobot, VAD_END);
		        printf("Speech end\n");
            }

            if(pRobot->last_vad != pRobot->vad_flag)
            {
                pRobot->last_vad = pRobot->vad_flag;
            }
		}
	}
	return NULL;
}

void asr_message_cb(MwAsrStatus status,void* info)
{
	MwAsrContext* pRobot = getAsrContext();

	if(status==ASR_STATE_OK){
		MwAsrSceneResult* pResult = (MwAsrSceneResult*)info;
		if(pResult->sceneContext.session_seq>0 && pRobot->session_seq!=pResult->sceneContext.session_seq)
			return;
	}else if(status==ASR_STATE_ERROR){
		end_session(0,0);
	}
	if(pRobot->callbacks.asrstate_cb){
		pRobot->callbacks.asrstate_cb(status,info);
	}
}


int mw_asr_client_create(const char* config_file,int multiturn,MwAsrClientCbs cbs)
{
	int ret = 0;
	
	MwAsrContext* pRobot = getAsrContext();
	if(pRobot->asr_init)
		return -1;
	memset(pRobot,0,sizeof(MwAsrContext));

	pRobot->is_running = 1;
	pRobot->callbacks.asrstate_cb = cbs.asrstate_cb;
	pRobot->allow_multiturn = multiturn;
	pthread_mutex_init(&pRobot->session_mutex, NULL);
	mw_ring_buffer_init(&pRobot->capture_rb, AUDIO_CAPTURE_BUFSIZE);
	AsrVadCreate(&pRobot->pAsrVad, 16000);
    pthread_create(&pRobot->audio_thread, NULL, audio_proc, pRobot);
	
	
	#if (ASR_SERVICE_TYPE==ASR_SERVICE_TULING)
	ret |= asr_client_init(config_file,CLIENT_TULING,&pRobot->asr_client,asr_message_cb);
	#endif
	pRobot->asr_init = 1;
exit:
	return ret;
}

int mw_asr_client_attach(const char* account)
{
	int ret = 0;
	return ret;
}

void mw_asr_client_touch_wakeup()
{
	set_wakeup_state();
}

void mw_asr_client_write_audio(unsigned char* pData,unsigned int length)
{
	int waitTimes = 0,canWrite = 0,frame_ms,diff=0;
	time_t active_time;
	long avail_bytes = 0;
	
	MwAsrContext* pRobot = getAsrContext();
	pRobot->current_time = GetTimeSec();
	active_time = pRobot->active_time;
	diff = pRobot->current_time - active_time;

	if(pRobot->current_time - pRobot->wakeup_time > ALL_SESSION_TIMEOUT){
		if(pRobot->multiturn){
			pRobot->multiturn = 0;
		}
	}
	if(pRobot->multiturn && !pRobot->session_flag){
		if(diff > ONE_SESSION_TIMEOUT){
			if(pRobot->callbacks.asrstate_cb){
				pRobot->callbacks.asrstate_cb(ASR_STATE_FINISH,NULL);
				mw_asr_client_end_session(0,1);
			}
		}else if(!pRobot->wakeup_flag){
			pRobot->wakeup_flag = 1;
			pRobot->wakeup_time = pRobot->current_time;
			if(pRobot->callbacks.asrstate_cb){
				pRobot->callbacks.asrstate_cb(ASR_STATE_RESTART,NULL);
			}
			printf("***************************************\n");
			printf("Restart listening\n");
			printf("***************************************\n");
		}
	}
	if(pRobot->asr_init){
		if(pRobot->wakeup_flag){
				frame_ms = length*10/TEN_MS_SIZE;
				while(pRobot->is_running && waitTimes++ < 300)
		        	{
		        		if(pRobot->idle_ms>0){
		        			pRobot->idle_ms -= frame_ms;
						if(pRobot->idle_ms)
							return;
		        		}
		            	avail_bytes = mw_ring_buffer_avail(&pRobot->capture_rb);

		            	if(avail_bytes >= length)
		            	{
		                	canWrite = 1;
		                	break;
		            	}

		            	rt_thread_mdelay(1);
		        	}

		        	if(canWrite)
		        	{
		            	mw_ring_buffer_push(&pRobot->capture_rb, (char*)pData, length);
		        	}
			}
	}
}


void mw_asr_client_set_busy(int busy)
{
	MwAsrContext* pRobot = getAsrContext();
	pRobot->busy_flag = busy;
}

void mw_asr_client_end_session(int active_flag,int multiturn_stop)
{
	end_session(active_flag,multiturn_stop);
}

void mw_asr_client_destroy()
{
	MwAsrContext* pRobot = getAsrContext();
	if(!pRobot->asr_init)
		return;
	asr_client_deinit(&pRobot->asr_client);
	pRobot->asr_init = 0;

	pRobot->is_running = 0;	
	pthread_join(pRobot->audio_thread, NULL);
	
	AsrVadFree(pRobot->pAsrVad);
	mw_ring_buffer_deinit(&pRobot->capture_rb);
	
	pthread_mutex_destroy(&pRobot->session_mutex);
}

