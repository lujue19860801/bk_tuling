#ifndef _lcthw_RingBuffer_h
#define _lcthw_RingBuffer_h

#include "stdint.h"
typedef struct {
    unsigned char *address;     
    unsigned int capacity;    /**< memory capacity in bytes */
    unsigned int wp;          /**< write point in bytes     */
    unsigned int rp;          /**< read point in bytes      */
    unsigned int need_alloc;
} RingBuffer;

RingBuffer *RingBuffer_create(int length, int type);

void RingBuffer_destroy(RingBuffer * buffer);

int RingBuffer_read(RingBuffer * buffer, char *target, int amount);

int RingBuffer_write(RingBuffer * buffer, char *data, int length);

int RingBuffer_empty(RingBuffer * buffer);
void RingBuffer_clear(RingBuffer *rb);

int RingBuffer_full(RingBuffer * buffer);

int RingBuffer_available_data(RingBuffer * buffer);

int RingBuffer_available_space(RingBuffer * buffer);

#define RingBuffer_full(B) (RingBuffer_available_space(B) == 0)
#define RingBuffer_empty(B) (RingBuffer_available_data((B)) == 0)


#endif
