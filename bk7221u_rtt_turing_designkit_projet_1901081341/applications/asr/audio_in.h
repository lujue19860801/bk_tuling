#ifndef _AUDIO_IN_H_
#define _AUDIO_IN_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtdevice.h>
#include <finsh.h>
#include <drivers/audio.h>
#include <pthread.h>

typedef void (*audio_callback)(void *buffer,rt_uint32_t size);

int mw_audio_stream_init(audio_callback cb);
void mw_audio_stream_deinit();

#endif
