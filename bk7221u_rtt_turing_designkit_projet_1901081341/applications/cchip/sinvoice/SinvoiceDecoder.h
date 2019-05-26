#ifndef __SINVOICE_DECODER_H__
#define __SINVOICE_DECODER_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "MediaService.h"

typedef struct DecoderData {
	char *ssid;
	char *pwd;
	char *customData;
}DecoderData;
void VoiceDecoderExit();

extern void DestoryVoiceDecoderPthread(void);
extern void CreateVoiceDecoderPthread(void);

typedef struct SinvoiceDecoderControl {
	SemaphoreHandle_t mux;
	SemaphoreHandle_t cond;
	SemaphoreHandle_t sinvoiceCond;
}SinvoiceDecoderControl;

unsigned int VoiceDecoderPut(unsigned char *buf, unsigned int len);

typedef struct SinvoiceDecoderService {
    /*relation*/
    MediaService Based;
    /*private*/
    int wifiConnected;
    int toneEnable;
    int sdMounted;
} SinvoiceDecoderService;


SinvoiceDecoderService *SinvoiceDecoderServiceCreate();

#endif
