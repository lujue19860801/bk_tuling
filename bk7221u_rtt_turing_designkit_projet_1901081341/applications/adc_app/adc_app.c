#include "include.h"
#include "adc_app.h"
#include "saradc_intf.h"
#include "gpio_pub.h"
#include "offline_player.h"
#include "cchip_key_check.h"
#include "stdlib.h"
#include "string.h"
#include "rtos_pub.h"
#include "error.h"
#include "mode_mgmt.h"
#include "cchip_led_control.h"
#include "flash_music_list.h"



enum{
	ADC_CHN_KEY = 0,
	ADC_CHN_VOL,
	ADC_CHN_NUM
};

enum{
	ADC_KEY_NEXT = 0,
	ADC_KEY_DIR,
	ADC_KEY_PAUSE,
	ADC_KEY_WECHAT,
	ADC_KEY_AIWIFI,
	ADC_KEY_COUNT
};

#define		MAX_KEY_NUMBER				(30)
#define		MAX_KEY_VOLTAGE				(2400)
#define 	LONG_KEY_PRESS_FLAG			(0x80000000)
#define 	LONG_KEY_RELEASE_FLAG		(0x40000000)
#define  	PRESS_VALUE(key)			(1<<key)
#define  	LONG_PRESS_VALUE(key)		(LONG_KEY_PRESS_FLAG|(1<<key))
#define  	LONG_RELEASE_VALUE(key)		(LONG_KEY_RELEASE_FLAG|(1<<key))

typedef struct{
	uint8_t allowContinue;
	uint8_t allowLongRelease;
	uint16_t multiTrig;
	uint16_t longPressThres;
}KeyConfig;

typedef struct{
	uint32_t keyValue;
	uint32_t longPress;
}KeyValue;

typedef struct{
	uint16_t key;
	uint16_t voltage;
	KeyConfig config;
}KeyAdcInfo;

typedef struct{
	uint8_t keyPressFlag;
	uint8_t longPressFlag;
	uint16_t longPressLast;
	uint16_t keyPressTime;
	uint16_t longPressThres;	
	uint16_t trigNum;
	time_t lastTrigTime;
}KeyScanInfo;


ADC_OBJ my_adc[ADC_CHN_NUM];
UINT32 my_chn[ADC_CHN_NUM]={1,7};
static KeyValue keyValue;

KeyAdcInfo adcKeys[ADC_KEY_COUNT]={
{ADC_KEY_NEXT,1354,{0,0,0,15}},
{ADC_KEY_DIR,972,{0,0,0,15}},
{ADC_KEY_PAUSE,640,{0,0,0,15}},
{ADC_KEY_WECHAT,320,{0,0,0,15}},
{ADC_KEY_AIWIFI,0,{0,0,0,15}},
};

static KeyScanInfo adcKeyInfos[ADC_KEY_COUNT] = {0};

time_t get_current_time()
{
	return 0;
}

int16_t get_key_by_voltage(int voltage)
{
	int i;
	uint16_t ceil,floor;
	for(i=0;i<ADC_KEY_COUNT;i++){
		if(i==0){
			ceil = MAX_KEY_VOLTAGE;
		}else{
			ceil = (adcKeys[i].voltage+adcKeys[i-1].voltage)/2;
		}

		if(i==ADC_KEY_COUNT-1){
			floor = 0;
		}else{
			floor = (adcKeys[i].voltage+adcKeys[i+1].voltage)/2;
		}

		if(voltage>=floor && voltage<ceil)
			break;
	}
	if(i<ADC_KEY_COUNT)
		return adcKeys[i].key;
	else
		return -1;
}

void adc_key_button_handler(KeyValue *pKey)
{
	static uint16_t led_display_on = 1;
	static LED_STATE led_state_backup;
	
	//offline mode
	if(mmgmt_is_offline_mode())
	{
		switch(pKey->keyValue){
			case PRESS_VALUE(ADC_KEY_DIR):
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_SONG_PREV);
				break;		
			case LONG_PRESS_VALUE(ADC_KEY_DIR):
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_CHANGE_DIR);
				break;
			case PRESS_VALUE(ADC_KEY_NEXT):
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_SONG_NEXT);
				break;
			case PRESS_VALUE(ADC_KEY_PAUSE):
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_PLAY_PAUSE);
				break;
			case LONG_PRESS_VALUE(ADC_KEY_PAUSE):
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_MODE_CHANGE);
				break;
			default:
				break;
		}
	}

	//cloud mode
	if(mmgmt_is_cloud_mode())
	{
		switch(pKey->keyValue)
		{
			case PRESS_VALUE(ADC_KEY_NEXT):
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_SONG_NEXT);
				break;
			case PRESS_VALUE(ADC_KEY_DIR):
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_SONG_PREV);
				break;
			case PRESS_VALUE(ADC_KEY_PAUSE):
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_PAUSE_PAUSE);
				break;
			case LONG_PRESS_VALUE(ADC_KEY_PAUSE):
				if(sd_is_online()){
					cchip_key_button_event_handler(CLOUD_KEYHANDLE_MODE_CHANGE);
				}
				break;
			case PRESS_VALUE(ADC_KEY_WECHAT):
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_WECHAT_MSG);
				break;
			case LONG_PRESS_VALUE(ADC_KEY_WECHAT):
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_WECHAT);
				break;
			case PRESS_VALUE(ADC_KEY_AIWIFI):
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_VOICE);
				break;
			case LONG_PRESS_VALUE(ADC_KEY_AIWIFI):
				
				break;
			default:
				break;
		}
	}

	//flash mode
	if(mmgmt_is_flash_mode())
	{
		switch(pKey->keyValue){
			case PRESS_VALUE(ADC_KEY_DIR):
				flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_SONG_PREV);
				break;
			case PRESS_VALUE(ADC_KEY_NEXT):
				flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_SONG_NEXT);
				break;
			case PRESS_VALUE(ADC_KEY_PAUSE):
				flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_PLAY_PAUSE);
				break;
			default:
				break;
		}
	}

	//other general key
	if(pKey->keyValue == LONG_PRESS_VALUE(ADC_KEY_NEXT)){
		cchip_key_button_event_handler(CLOUD_KEYHANDLE_NET_CONFIG);
		rt_kprintf("**********************************************\n");
		rt_kprintf("Enter network configure mode\n");
		rt_kprintf("**********************************************\n");
	}
}

void check_adc_keys(KeyValue* keyAdc,int key)
{
	static int adcKey = -1;
	KeyScanInfo *pInfo = NULL;
	KeyAdcInfo *pAdc = NULL;

	if(adcKey>=0 && (key==-1 ||key!=adcKey))
	{
		pInfo = &adcKeyInfos[adcKey];
		pAdc = &adcKeys[adcKey];
		if(pAdc->config.allowLongRelease && pInfo->longPressLast){
			keyAdc->keyValue |= (1<<adcKey);
			keyAdc->keyValue |= LONG_KEY_RELEASE_FLAG;
			rt_kprintf("*********************************\n");
			rt_kprintf("Long Release %d KEY\n",adcKey);
			rt_kprintf("*********************************\n");
		}
		if(pInfo->keyPressFlag){
			if(keyAdc->longPress & (1<<adcKey)){
				keyAdc->longPress &= ~(1<<adcKey);
				keyAdc->keyValue &= ~(1<<adcKey);
			}else if(!pInfo->longPressFlag && pInfo->keyPressTime>=1&&!pInfo->longPressLast){
				if(pAdc->config.multiTrig){
					if(get_current_time()-pInfo->lastTrigTime<=2){
						pInfo->trigNum ++;
						if(pInfo->trigNum>=pAdc->config.multiTrig){
							keyAdc->keyValue |= (1<<adcKey);
							pInfo->trigNum = 0;
						}
					}else{
						pInfo->trigNum =1;							
					}
					pInfo->lastTrigTime = get_current_time();
				}else{
					keyAdc->keyValue |= (1<<adcKey);
				}
				rt_kprintf("*********************************\n");
				rt_kprintf("Press %d KEY\n",adcKey);
				rt_kprintf("*********************************\n");
			}
			pInfo->keyPressFlag = 0;
			pInfo->keyPressTime = 0;
			pInfo->longPressFlag = 0;
			pInfo->longPressLast = 0;
			pInfo->longPressThres = pAdc->config.longPressThres;
		}
		adcKey = -1;
	}
	if(key>=0){
		pInfo = &adcKeyInfos[key];
		pAdc = &adcKeys[key];
		pInfo->keyPressFlag = 1;
		pInfo->keyPressTime +=1;
		adcKey = key;
		rt_kprintf("Num:%d,target:%d\n",pInfo->keyPressTime,pInfo->longPressThres);
		if(pInfo->keyPressTime>=pInfo->longPressThres && pInfo->longPressFlag ==0){
			keyAdc->longPress |= (1<<key);
			keyAdc->keyValue |= (1<<key);
			keyAdc->keyValue |= LONG_KEY_PRESS_FLAG;
			rt_kprintf("*********************************\n");
			rt_kprintf("Long press %d KEY\n",key);
			rt_kprintf("*********************************\n");
			pInfo->keyPressFlag = 0;
			pInfo->keyPressTime = 0;
			pInfo->longPressLast = 1;
			if(pAdc->config.allowContinue){
				pInfo->longPressFlag = 0;
				pInfo->longPressThres = pAdc->config.longPressThres/2;
			}else{
				pInfo->longPressFlag = 1;
			}
		}
	}
}


void my_adc_obj_callback(int new_mv, void *user_data)
{
	int volume = 0,key;
	static int last_volume = 0;
	ADC_OBJ* pObj = (ADC_OBJ*)user_data;
	if(pObj->channel == my_chn[ADC_CHN_VOL]){
		volume = new_mv*100/2400;
		if(abs(volume-last_volume)>=6){
			last_volume = volume;
			//audio_device_set_volume(volume);
		}
	}else if(pObj->channel == my_chn[ADC_CHN_KEY]){
		//rt_kprintf("Key adc:%d mv\n",new_mv);
		key = get_key_by_voltage(new_mv);
		keyValue.keyValue = 0;
		check_adc_keys(&keyValue, key);
		adc_key_button_handler(&keyValue);
	}
	/*int value;
	static int mic_in = 0;
	value = gpio_input(8);
	if(value==0 && !mic_in){
		mic_in = 1;
		rt_kprintf("Mic in\n");
		gpio_output(9,0);
	}else if(value && mic_in){
		mic_in = 0;
		rt_kprintf("Mic out\n");
		gpio_output(9,1);
	}*/
}


void adc_button_app_init(void)
{
	int i;

	for(i=ADC_CHN_VOL;i<ADC_CHN_NUM;i++){
		adc_obj_init(&my_adc[i],my_adc_obj_callback,my_chn[i],&my_adc[i]);
		adc_obj_start(&my_adc[i]);
	}
	for(i=0;i<ADC_KEY_COUNT;i++){
		adcKeyInfos[i].longPressThres = adcKeys[i].config.longPressThres;
	}
	//gpio_config(8,GMODE_INPUT_PULLUP);
	//gpio_config(9,GMODE_OUTPUT);
}


void adc_button_app_deinit(void)
{
}
