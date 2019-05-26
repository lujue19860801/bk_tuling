#ifndef _CCHIP_KEY_HANDLE_H_
#define __CCHIP_KEY_HANDLE_H_

#include "sys_rtos.h"
#include "rtos_pub.h"
#include "turingconfig.h"
#include "gpio_pub.h"

#define KEY_DEBUG

#define PIN_LOW                 0x00
#define PIN_HIGH                0x01
#define PIN_MODE_OUTPUT         0x00

#define CCHIP_KEY_PLAY_PAUSE_PIN				4//P4 42
#define CCHIP_KEY_VOLUME_ADD 					3//P3 41
#define CCHIP_KEY_VOLUME_REDUCE				    2//P2 43
#define CCHIP_KEY_VOICE						    7//P7 45
#define CCHIP_WECHAT							6//P6 44
//#define CCHIP_BAT_DET							23

typedef enum
{
    CLOUD_KEYHANDLE_UNDEFINE = 0,
    CLOUD_KEYHANDLE_PAUSE_PAUSE = 1,
    CLOUD_KEYHANDLE_SONG_NEXT = 2,
    CLOUD_KEYHANDLE_SONG_PREV = 3,
    CLOUD_KEYHANDLE_VOICE = 4,
    CLOUD_KEYHANDLE_WECHAT = 5,
    CLOUD_KEYHANDLE_VOLUME_ADD = 6,
    CLOUD_KEYHANDLE_VOLUME_REDUCE = 7,
    CLOUD_KEYHANDLE_MODE_CHANGE = 8,
    CLOUD_KEYHANDLE_MUSIC_COLLECT = 9,
    CLOUD_KEYHANDLE_NET_CONFIG = 10,
    CLOUD_KEYHANDLE_WECHAT_MSG = 11,
} CloudKeyHandleEvent;


#ifdef dalianmao
#define USB_DET    GPIO30 //13,  gpio13 is reset pin, if set to 0, chip may reset
//#define CHG_DET    31
#endif

#ifdef k1
#define LED_R    27
#define LED_G    26
#define LED_B    25
#define LED_W    24
#define LED_DET    23

#endif



#ifdef KEY_DEBUG
#define KEY_PRT       os_printf
#else
#define APP_PRT       os_null_printf
#endif


void cchip_key_init(void);

void cchip_key_button_event_handler(CloudKeyHandleEvent event);


#endif


