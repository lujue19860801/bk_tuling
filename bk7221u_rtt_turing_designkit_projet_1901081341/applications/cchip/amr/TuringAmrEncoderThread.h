#ifndef __TURING_AMR_ENCODER_THREAD_H__
#define __TURING_AMR_ENCODER_THREAD_H__

#include <rtdef.h>
#include "TuringCommon.h"
#include "MediaService.h"
#include "RingFiFo.h"
#include <amrnb_encoder.h>

#define AENC_DEBUG_PRTF                     1

#define AENC_WARNING_PRINTF                 rt_kprintf
#define AENC_LOG(...)

#if AENC_DEBUG_PRTF
#define AENC_PRINTF                         rt_kprintf
#else
#define AENC_PRINTF(...)
#endif //AENC_DEBUG_PRTF

#define AENC_SUCCESS                        0
#define AENC_FAILURE                        1

#define AENC_CHANNEL                        (1)
#define AENC_IN_BUF_LEN                     (AENC_CHANNEL * 2 * 160)    
#define AENC_IN_CELL_LEN                    (AMRNB_ENCODER_SAMPLES_PER_FRAME)
#define AENC_OUT_BUF_LEN                    (500)

#define AENC_UPLOAD_HEADER                  "#!AMR\n"
#define AENC_UPLOAD_HEADER_LEN              6 /*sizeof(AENC_UPLOAD_HEADER)*/

typedef struct _aenc_buf_
{
	uint8_t in[AENC_IN_BUF_LEN];
	int16_t in_cell[AENC_IN_CELL_LEN];	
	uint8_t out[AENC_OUT_BUF_LEN];
}AENC_BUF_T;

typedef enum
{
    ENCODER_STATE_NONE = 0,
    ENCODER_STATE_ONGING,
    ENCODER_STATE_DONE,
} EncoderState;

typedef struct
{
    int state;
    uint8_t *buf;
    int size;
    int len;

	uint32_t is_canceling;

    RingBuffer *fifo;

    rt_mutex_t mutex;
    rt_sem_t sema_start;
} EncoderControl;

typedef struct TuringEncoderService
{
    MediaService Based;
    int wifiConnected;
    int toneEnable;
    int sdMounted;
}  TuringEncoderService;

void EncoderFiFoInit(void);
void EncoderFiFoDeinit(void);
void StartTuringEncoder(void);
uint32_t aenc_upload_buf_init(void);
void aenc_upload_buf_deinit(void);
int aenc_is_finished(void);
void aenc_waiting_for_finish(void);
unsigned char *aenc_upload_buf_get(int *len);
int EncoderFiFoPut(char *buf,  unsigned int len);
uint32_t aenc_upload_buf_put(unsigned char *buf,  unsigned int len);
TuringEncoderService *TuringEncoderServiceCreate(void);
#endif
// eof

