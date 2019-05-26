#include "audio_in.h"

#define AUDIO_SAMPLE_RATE		(8000)
#define AUDIO_SAMPLE_CHN		(1)
typedef struct{
	int running;
	int sample_rate;
    int n_channel;
	rt_device_t adc_device;
	pthread_t audio_thread;
	audio_callback cb;
}audio_stream_context;

static audio_stream_context audio_context;


audio_stream_context* get_audio_context()
{
	return &audio_context;
}

rt_size_t my_mic_sound_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_size_t read_bytes = 0;

    while (read_bytes < size)
    {
        rt_size_t rb = rt_device_read(dev, pos, (void *)((char *)buffer + read_bytes), size - read_bytes);

        if (rb == 0)
            break;

        read_bytes += rb;
    }

    return read_bytes;
}


static void* capture_proc(void* arg)
{
	audio_stream_context* pContext = (audio_stream_context*)(arg);
	int rate, channel;
	rt_uint32_t read_bytes = 0;
	rt_uint8_t *buffer = sdram_malloc(AUDIO_SAMPLE_RATE/50);
	if(!buffer){
		goto OUT;
	}
	rate = pContext->sample_rate;
    channel = pContext->n_channel;
	pContext->adc_device = rt_device_find("mic");
    if (!pContext->adc_device)
    {
        rt_kprintf("audio mic not found \n");
        goto OUT;
    }
    rt_device_control(pContext->adc_device, CODEC_CMD_SAMPLERATE, (void *)&rate); 
    rt_device_control(pContext->adc_device, CODEC_CMD_SET_CHANNEL, (void *)&channel); 
	rt_device_open(pContext->adc_device, RT_DEVICE_OFLAG_RDONLY);
	while(pContext->running){
		read_bytes = my_mic_sound_read(pContext->adc_device, 0, buffer, AUDIO_SAMPLE_RATE/50);
		if(read_bytes>0 && pContext->cb){
			pContext->cb(buffer,read_bytes);
		}
		rt_thread_mdelay(5);
	}
	rt_device_close(pContext->adc_device);
	pContext->adc_device = RT_NULL;
	sdram_free(buffer);
OUT:
	return NULL;
}

int mw_audio_stream_init(audio_callback cb)
{
	audio_stream_context* pContext = get_audio_context();
	memset(pContext,0,sizeof(audio_stream_context));
	pContext->running = 1;
	pContext->cb = cb;
	pContext->n_channel = AUDIO_SAMPLE_CHN;
	pContext->sample_rate = AUDIO_SAMPLE_RATE;
	pthread_create(&pContext->audio_thread,NULL,capture_proc,pContext);
}

void mw_audio_stream_deinit()
{
	audio_stream_context* pContext = get_audio_context();
	pContext->running = 0;
	pthread_join(pContext->audio_thread, NULL);
}
