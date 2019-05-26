#ifndef RING_BUFFER_H
#define RING_BUFFER_H


#include "stdio.h"
#include "stdlib.h"

#define RB_ERR_OK		0
#define RB_ENOMEM		12
#define RB_EFULL		28


typedef struct {
	char *buffer;
	int length;
} item_data;


typedef struct {
	int r_pos;
	int w_pos;

	int nitem;
	item_data *item_array;
}pcm_ring_buffer;


#define robot_ring_buffer_empty(B) ((B)->r_pos == (B)->w_pos)
#define robot_ring_buffer_full(B) ( ((B)->w_pos+1)%(B)->nitem == (B)->r_pos)


int robot_ring_buffer_init(pcm_ring_buffer *prb, int nitem);
void robot_ring_buffer_deinit(pcm_ring_buffer *prb);

int robot_ring_buffer_push(pcm_ring_buffer *prb, char *buffer, int length);

static item_data * robot_ring_buffer_peek(pcm_ring_buffer *prb)
{
	int pos = prb->r_pos;
	if (robot_ring_buffer_empty(prb))
	{
		return NULL;
	}
	//printf("Peek--writePos:%d,readPos:%d\n",prb->w_pos,prb->r_pos);
	return (&prb->item_array[pos]);
}

static void robot_ring_buffer_pop(pcm_ring_buffer *prb)
{
	prb->r_pos = (prb->r_pos+1)%prb->nitem;
}

static void robot_ring_buffer_reset(pcm_ring_buffer *prb)
{
	prb->r_pos = 0;
	prb->w_pos = 0;
}

static int robot_ring_buffer_available(pcm_ring_buffer *prb)
{
	if (prb->w_pos>=prb->r_pos)
	{
		return prb->w_pos - prb->r_pos;
	}

	return prb->nitem + prb->w_pos - prb->r_pos;
}


#endif
