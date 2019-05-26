#ifndef __CAPTURE_FIFO_H__
#define __CAPTURE_FIFO_H__
#include <rtdef.h>

typedef struct capture_fifo {
	unsigned char *buffer;	/* the buffer holding the data */
	unsigned int size;		/* the size of the allocated buffer */
	unsigned int wrsize;	/* the size of the start to write to file */
	unsigned int in;		/* data is added at offset (in % size) */
	unsigned int out;		/* data is extracted from off. (out % size) */
	rt_mutex_t mux;
}CaptureFiFo;

void CaptureFiFoClear(CaptureFiFo *fifo);
unsigned int CaptureFiFoPut(CaptureFiFo *fifo,  unsigned char *buffer, unsigned int len);
unsigned int CaptureFiFoGet(CaptureFiFo *fifo, unsigned char *buffer, unsigned int offset, unsigned int len);
unsigned int CaptureFiFoGetLenth(CaptureFiFo *fifo);
unsigned int CaptureFiFoSeek(CaptureFiFo *fifo,unsigned char *buffer, unsigned int offset, unsigned int len);
unsigned int CaptureFiFoSetOffset(CaptureFiFo *fifo,unsigned int offset);

CaptureFiFo *CaptureFiFoAlloc(unsigned int size);
void CaptureFiFoFree(CaptureFiFo *fifo);







#endif


