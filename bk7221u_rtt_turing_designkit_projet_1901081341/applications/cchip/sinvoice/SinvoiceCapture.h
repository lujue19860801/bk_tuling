#ifndef __SINVOICE_CAPTURE_H__
#define __SINVOICE_CAPTURE_H__


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "RingFiFo.h"
#include "CaptureFiFo.h"
#include "MediaService.h"

typedef struct {
	int state;
	int vad;
	int type;//0:开vad 1:关vad

	CaptureFiFo *fifo;
	SemaphoreHandle_t mux;
	SemaphoreHandle_t cond;
}SinvoiceControl;

typedef struct SinvoiceCaptureService { //extern from TreeUtility
    /*relation*/
    MediaService Based;
    /*private*/
    int wifiConnected;
    int toneEnable;
    int sdMounted;
} SinvoiceCaptureService;

SinvoiceCaptureService *SinvoiceCaptureServiceCreate();

void StartSinvoiceCapture();
void SinvoiceFiFoDeinit();
void SinvoiceFiFoInit();
int SinvoiceFiFoSeek(unsigned char *buf,  unsigned int len);
int SinvoiceFiFoLength();
int SinvoiceFiFoPut(unsigned char *data, int len);
#endif