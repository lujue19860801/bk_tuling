#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "ring_buffer.h"

int robot_ring_buffer_init(pcm_ring_buffer *prb, int nitem)
{
	prb->nitem = 0;
	prb->item_array = (item_data *)sdram_malloc(sizeof(item_data) * nitem);
	if (!prb->item_array)
	{
		return RB_ENOMEM;
	}
	memset(prb->item_array, 0, sizeof(item_data) * nitem);
	prb->r_pos = 0;
	prb->w_pos = 0;
	prb->nitem = nitem;

	return 0;
}

void robot_ring_buffer_deinit(pcm_ring_buffer *prb)
{
	if (prb->nitem)
	{
		int i;
		for (i=0; i<prb->nitem; i++)
		{
			if (prb->item_array[i].buffer)
			{
				sdram_free(prb->item_array[i].buffer);

				prb->item_array[i].buffer = NULL;
				prb->item_array[i].length = 0;
			}
		}

		sdram_free(prb->item_array);
		prb->r_pos = 0;
		prb->w_pos = 0;
		prb->nitem = 0;
	}
}

int robot_ring_buffer_push(pcm_ring_buffer *prb, char *buffer, int length)
{
	if (robot_ring_buffer_full(prb))
	{
		return RB_EFULL;
	}

	item_data *item = &prb->item_array[prb->w_pos];

	if(!item->buffer)
	{
		item->buffer = (char*)sdram_malloc(length);
		if (!item->buffer)
		{
			return RB_ENOMEM;
		}

		item->length = length;
	}

	memcpy(item->buffer, buffer, length);

	prb->w_pos = (prb->w_pos+1)%prb->nitem;
	//printf("Push--writePos:%d,readPos:%d\n",prb->w_pos,prb->r_pos);
	return 0;
}


