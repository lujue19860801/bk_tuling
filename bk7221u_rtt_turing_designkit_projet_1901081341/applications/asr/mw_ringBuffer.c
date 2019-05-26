#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mw_ringBuffer.h"

int mw_ring_buffer_init(mw_ring_buffer *rb, long n)
{
	if(!rb)
		return -1;
	rb->buffer = (unsigned char *)sdram_malloc(n);
	if(!rb->buffer)
		return -1;
	pthread_mutex_init(&rb->mutex, NULL);
	rb->capacity = n;
	rb->r_pos = 0;
	rb->w_pos = 0;
	memset(rb->buffer,0,n);
	return 0;
}

void mw_ring_buffer_deinit(mw_ring_buffer *rb)
{
	if(rb){
		if(rb->buffer)
			sdram_free(rb->buffer);
		pthread_mutex_destroy(&rb->mutex);
		rb->r_pos = 0;
		rb->w_pos = 0;
	}
}

int mw_ring_buffer_push(mw_ring_buffer *rb, char *buffer, unsigned int length)
{
	unsigned int len1=0,len2=0;
	if(!rb)
		return -1;

	RING_BUFFER_LOCK(rb);
	if(rb->capacity - rb->w_pos >= length){
		len1 = length;
	}else{
		len1 = rb->capacity  - rb->w_pos;
		len2 = length - len1;
	}
	memcpy(rb->buffer+rb->w_pos,buffer,len1);
	rb->w_pos += len1;
	rb->w_pos %= rb->capacity;
	if(len2>0){
		//printf("Push read pos:%ld,write pos:%ld\n",rb->r_pos,rb->w_pos);
		memcpy(rb->buffer+rb->w_pos,buffer+len1,len2);
		rb->w_pos += len2;
		rb->w_pos %= rb->capacity;
	}
	RING_BUFFER_UNLOCK(rb);
	
	return 0;
}

int mw_ring_buffer_pop(mw_ring_buffer* rb,char* output,unsigned int output_len)
{
	unsigned int len1=0,len2=0;
	if(!rb || output_len ==0)
		return -1;

	RING_BUFFER_LOCK(rb);
	if(rb->w_pos >= rb->r_pos)
		len1 = output_len;
	else{
		if(rb->capacity-rb->r_pos>=output_len)
			len1 = output_len;
		else{
			len1 = rb->capacity - rb->r_pos;
			len2 = output_len - len1;
		}
	}
	memcpy(output,rb->buffer+rb->r_pos,len1);
	rb->r_pos += len1;
	rb->r_pos %= rb->capacity;
	if(len2){
		memcpy(output+len1,rb->buffer+rb->r_pos,len2);
		rb->r_pos += len2;
		rb->r_pos %= rb->capacity;
	}
	RING_BUFFER_UNLOCK(rb);

	return 0;
}
