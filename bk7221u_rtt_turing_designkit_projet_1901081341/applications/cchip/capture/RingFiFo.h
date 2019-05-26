#ifndef __RING_FIFO_H__
#define __RING_FIFO_H__

#include "ringbuffer.h"

void RingFiFomutexinit(void);
int RingFiFoAvailableData(RingBuffer * buffer);
int RingFiFoRead(RingBuffer * buffer, char *target, int amount);
int RingFiFoWrite(RingBuffer * buffer, char *data, int length);
int RingFiFoAvailableSpace(RingBuffer * buffer);
RingBuffer *RingFiFoInit(int size);
void RingFiFoDeinit(RingBuffer *buffer);
void RingFiFoClear(RingBuffer *buffer);
int RingFiFoEmpty(RingBuffer *buffer);


RingBuffer *mic_RingFiFoInit(int size);
int mic_RingFiFoWrite(RingBuffer * buffer, char *data, int length);
int mic_RingFiFoRead(RingBuffer * buffer, char *target, int amount);
int mic_RingFiFoAvailableData(RingBuffer * buffer);
int mic_RingFiFoAvailableSpace(RingBuffer * buffer);
void mic_RingFiFoDeinit(RingBuffer *buffer);	
void mic_RingFiFoClear(RingBuffer *buffer);
int mic_RingFiFoEmpty(RingBuffer *buffer);
#endif
// eof

