#ifndef __CCHIP_ADCCHECK_H_
#define __CCHIP_ADCCHECK_H_
#include "typedef.h"

void cchip_adccheck_callback( void * arg );

UINT32 adc_get(void);

int volume_adc_get_last_percent(void);

void usb_charging_detect_open(void);

void turing_adc_create(void);


#endif

