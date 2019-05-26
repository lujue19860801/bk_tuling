#ifndef _MW_RINGBUFFER_H_
#define _MW_RINGBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef short int16_t;

typedef struct {
	long r_pos;
	long w_pos;

	long capacity;
	unsigned char *buffer;
	pthread_mutex_t mutex;
}mw_ring_buffer;

#define mw_ring_buffer_empty(B) ((B)->r_pos == (B)->w_pos)
#define mw_ring_buffer_full(B) ( ((B)->w_pos+1)%(B)->capacity == (B)->r_pos)
#define RING_BUFFER_LOCK(rb)		pthread_mutex_lock(&((rb)->mutex))
#define RING_BUFFER_UNLOCK(rb)		pthread_mutex_unlock(&((rb)->mutex))

int mw_ring_buffer_init(mw_ring_buffer *rb, long n);
void mw_ring_buffer_deinit(mw_ring_buffer *rb);
int mw_ring_buffer_push(mw_ring_buffer *rb, char *buffer, unsigned int length);
int mw_ring_buffer_pop(mw_ring_buffer* rb,char* output,unsigned int output_len);

static inline void mw_ring_buffer_reset(mw_ring_buffer *rb)
{
	RING_BUFFER_LOCK(rb);
	rb->r_pos = 0;
	rb->w_pos = 0;
	RING_BUFFER_UNLOCK(rb);
}

static inline long mw_ring_buffer_content(mw_ring_buffer *rb)
{
	long size;
	RING_BUFFER_LOCK(rb);
	if (rb->w_pos>=rb->r_pos)
	{
		size = rb->w_pos - rb->r_pos;
	}else{
		size = rb->capacity + rb->w_pos - rb->r_pos;
	}
	RING_BUFFER_UNLOCK(rb);

	return size;
}

static inline long mw_ring_buffer_avail(mw_ring_buffer* rb)
{
	long avail;
	avail = rb->capacity -1 - mw_ring_buffer_content(rb);
	return avail;
}
#ifdef __cplusplus
}
#endif

#endif