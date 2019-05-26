#include "key_app.h"
#include "rtthread.h"
#include "include.h"
#include "offline_player.h"
#include "cchip_key_check.h"
#include "stdlib.h"
#include "string.h"
#include "rtos_pub.h"
#include "error.h"
#include "key_main.h"
#include "mode_mgmt.h"
#include "cchip_led_control.h"
#include "flash_music_list.h"
#include "TuringInterJson.h"
#include "saradc_intf.h"
#include "mw_asr.h"

struct key_button{
	unsigned short vol_next_key_long_count : 8;
	unsigned short vol_prev_key_long_count : 8;
};

struct key_button kybt = {0};
static uint16_t factory_mask = 0;

uint16_t music_collect_flag = 0;

extern int start_voice_config();
#if (USING_DEMO_BOARD == 1)
static void key_button_event_handler(int keyType)
{
	if(mmgmt_is_offline_mode())
	{
		switch(keyType)
		{
			case _KEY_PLAY_PAUSE_LONG_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_MODE_CHANGE);
				break;
			case _KEY_PLAY_PAUSE_SHORT_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_PLAY_PAUSE);
				break;
			case _KEY_VOLUME_ADD_NEXT_SHORT_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_SONG_NEXT);
				break;
			case _KEY_VOLUME_ADD_NEXT_LONG_PRESS:
			case _KEY_VOLUME_ADD_NEXT_LONG_START_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_VOLUME_ADD);
				break;
			case _KEY_VOLUME_ADD_NEXT_DOUBLE_PRESS:
				break;
			case _KEY_VOLUME_REDUCE_PREV_SHORT_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_SONG_PREV);
				break;
			case _KEY_VOLUME_REDUCE_PREV_LONG_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_VOLUME_REDUCE);
				break;
			case _KEY_VOLUME_REDUCE_PREV_LONG_START_PRESS:
				break;
			case _KEY_VOLUME_REDUCE_PREV_DOUBLE_PRESS:
				break;
			case _KEY_PLAY_VOICE_SHORT_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_UNDEFINE);
				break;
			case _KEY_PLAY_WECHAT_SHORT_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_UNDEFINE);
				break;
			case _KEY_PLAY_WECHAT_LONG_PRESS:
				break;
			case _KEY_PLAY_WECHAT_LONG_START_PRESS:
				break;
			case _KEY_PLAY_WECHAT_DOUBLE_PRESS:
				offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_UNDEFINE);
				break;
			default:
				break;
		}
	}
	else if(mmgmt_is_cloud_mode())
	{
		switch(keyType)
		{
			case _KEY_PLAY_PAUSE_LONG_PRESS:
				if(sd_is_online()&&(0 == music_collect_flag))
					cchip_key_button_event_handler(CLOUD_KEYHANDLE_MODE_CHANGE);
				break;
			case _KEY_PLAY_PAUSE_SHORT_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_PAUSE_PAUSE);
				break;
			case _KEY_VOLUME_ADD_NEXT_SHORT_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_SONG_NEXT);
				break;
			case _KEY_VOLUME_ADD_NEXT_LONG_START_PRESS:
			case _KEY_VOLUME_ADD_NEXT_LONG_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_VOLUME_ADD);
				break;
			case _KEY_VOLUME_ADD_NEXT_DOUBLE_PRESS:
				break;
			case _KEY_VOLUME_REDUCE_PREV_SHORT_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_SONG_PREV);
				break;
			case _KEY_VOLUME_REDUCE_PREV_LONG_START_PRESS:
			case _KEY_VOLUME_REDUCE_PREV_LONG_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_VOLUME_REDUCE);
				break;
			case _KEY_VOLUME_REDUCE_PREV_DOUBLE_PRESS:
				break;
			case _KEY_PLAY_VOICE_SHORT_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_VOICE);
				break;
				
			case _KEY_PLAY_VOICE_LONG_PRESS:
				if(sd_is_online())
					cchip_key_button_event_handler(CLOUD_KEYHANDLE_MUSIC_COLLECT);
				break;
				
			case _KEY_PLAY_WECHAT_SHORT_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_WECHAT);
				break;
			case _KEY_PLAY_WECHAT_LONG_PRESS:
				break;
			case _KEY_PLAY_WECHAT_LONG_START_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_NET_CONFIG);
				break;
			case _KEY_PLAY_WECHAT_DOUBLE_PRESS:
				cchip_key_button_event_handler(CLOUD_KEYHANDLE_WECHAT_MSG);
				break;
			default:
				break;
		}
	}
	else if(mmgmt_is_flash_mode())
	{
		switch(keyType)
		{
			case _KEY_PLAY_PAUSE_SHORT_PRESS:
				flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_PLAY_PAUSE);
				break;
			case _KEY_VOLUME_ADD_NEXT_SHORT_PRESS:
				flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_SONG_NEXT);
				break;
			case _KEY_VOLUME_REDUCE_PREV_SHORT_PRESS:
				flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_SONG_PREV);
				break;	
			default:
				break;			
		}
	}
	
}


static void key_play_pause_short_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_PAUSE_SHORT_PRESS);
}
static void key_play_pause_long_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_PAUSE_LONG_PRESS);
}
static void key_volume_add_next_short_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_VOLUME_ADD_NEXT_SHORT_PRESS);
}

static void key_volume_add_next_double_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif

}

static void key_volume_add_next_long_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	kybt.vol_next_key_long_count++;
	if(kybt.vol_next_key_long_count >= LONG_PRESS_HOLD_FILTERS_TIMES)
	{
		kybt.vol_next_key_long_count = 0;
		key_button_event_handler(_KEY_VOLUME_ADD_NEXT_LONG_PRESS);
	}
}

static void key_volume_reduce_prev_short_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_VOLUME_REDUCE_PREV_SHORT_PRESS);
}

static void key_volume_reduce_prev_double_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	
}

static void key_volume_reduce_prev_long_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	kybt.vol_prev_key_long_count++;
	if(kybt.vol_prev_key_long_count >= LONG_PRESS_HOLD_FILTERS_TIMES)
	{
		kybt.vol_prev_key_long_count = 0;
		key_button_event_handler(_KEY_VOLUME_REDUCE_PREV_LONG_PRESS);
	}
}

static void key_voice_short_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("----[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_VOICE_SHORT_PRESS);
}

static void key_voice_long_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("----[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_VOICE_LONG_PRESS);
}

static void key_wechat_short_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_WECHAT_SHORT_PRESS);
}

static void key_wechat_long_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_WECHAT_LONG_START_PRESS);
}

static void key_wechat_double_press(void *param)
{
#if KEY_BUTTON_DEBUG
	rt_kprintf("---[LOG]%s:%d\n", __FUNCTION__, __LINE__);
#endif
	key_button_event_handler(_KEY_PLAY_WECHAT_DOUBLE_PRESS);
}

#if KEY_BUTTION_TABLE_CFG
typedef struct{
	uint32_t gpio;
	void (*short_press)(void *arg);
	void (*double_press)(void *arg);
	void (*long_press)(void *arg);
	void (*hold_press)(void *arg);
}key_button_item_t;


const key_button_item_t key_button_table[]={
	{MAP_KEY_PLAY_PAUSE_PIN, key_play_pause_short_press, NULL, key_play_pause_long_press,NULL},
	{MAP_KEY_VOLUME_ADD_NEXT, key_volume_add_next_short_press,NULL,key_volume_add_next_double_press, key_volume_add_next_long_press},
	{MAP_KEY_VOLUME_REDUCE_PREV, key_volume_reduce_prev_short_press,NULL,key_volume_reduce_prev_double_press, key_volume_reduce_prev_long_press},
	{MAP_KEY_VOICE, key_voice_short_press, NULL, key_voice_long_press,NULL},
	{MAP_WECHAT, key_wechat_short_press, key_wechat_double_press, key_wechat_long_press,NULL}
	
};

void key_button_app_init(void)
{
#if CFG_ENABLE_BUTTON
	#if (0 == KEY_BUTTION_TABLE_CFG)
		key_item_configure(MAP_KEY_PLAY_PAUSE_PIN, key_play_pause_short_press, NULL,key_play_pause_long_press,NULL );
		key_item_configure(MAP_KEY_VOLUME_ADD_NEXT, key_volume_add_next_short_press,NULL,
								key_volume_add_next_double_press, key_volume_add_next_long_press);
		key_item_configure(MAP_KEY_VOLUME_REDUCE_PREV, key_volume_reduce_prev_short_press,NULL,
								key_volume_reduce_prev_double_press, key_volume_reduce_prev_long_press);
		key_item_configure(MAP_KEY_VOICE, key_voice_short_press, NULL, NULL,NULL);
		key_item_configure(MAP_WECHAT, key_wechat_short_press, key_wechat_double_press, NULL,NULL);
	#elif KEY_BUTTION_TABLE_CFG
		int i;
		for(i=0;i<(sizeof(key_button_table)/sizeof(key_button_item_t));i++)
		{
			if((key_button_table[i].short_press)||(key_button_table[i].double_press)
				||(key_button_table[i].long_press) ||(key_button_table[i].hold_press))
			{
				key_item_configure(key_button_table[i].gpio, key_button_table[i].short_press, 
							key_button_table[i].double_press, key_button_table[i].long_press,
							key_button_table[i].hold_press);
			}
		}
	#endif
#endif
}
#endif

#else

void my_voice_config_callback(int code,char* ssid,char* pwd,char* custom)
{
	char cmd[128]={0};
	if(code==0){
		sprintf(cmd,"wifi w0 join %s %s",ssid,pwd);
		msh_exec(cmd, strlen(cmd));
	}
}

static void key_button_event_handler(int keyType)
{
	static uint16_t child_lock = 0;
	static uint16_t led_display_control = 0;
	static LED_STATE state_backup;
#if FACTORY_TEST_ENABLE
	int factory_mode = mmgmt_is_factory_test_mode();
	if(!factory_mode)
#endif
	{
		if(0 == child_lock)
		{
			if(mmgmt_is_offline_mode())
			{
				switch(keyType)
				{
					case _KEY_NEXT_SHORT_PRESS:
						offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_SONG_NEXT);
						break;
			/////////////////////
				//JUST FOR TEST
					case _KEY_NEXT_LONG_PRESS:
						offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_MODE_CHANGE);
						break;
			////////////////////////////////////////
					case _KEY_PREV_SHORT_PRESS:
						offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_SONG_PREV);			
						break;
					case _KEY_COLLECT_SHORT_PRESS:
						offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_PLAY_PAUSE);
						break;
					default:
						break;
				}
			}
			else if(mmgmt_is_cloud_mode())
			{
				switch(keyType)
				{
					case _KEY_NEXT_LONG_PRESS:
						if(sd_is_online()&&(0 == music_collect_flag))
							cchip_key_button_event_handler(CLOUD_KEYHANDLE_MODE_CHANGE);
						break;
					case _KEY_NEXT_SHORT_PRESS:
						cchip_key_button_event_handler(CLOUD_KEYHANDLE_SONG_NEXT);
						break;

						
					case _KEY_PREV_SHORT_PRESS:
						cchip_key_button_event_handler(CLOUD_KEYHANDLE_SONG_PREV);
						break;
						
					case _KEY_COLLECT_LONG_PRESS:
						if(sd_is_online())
							cchip_key_button_event_handler(CLOUD_KEYHANDLE_MUSIC_COLLECT);
						break;
					case _KEY_COLLECT_SHORT_PRESS:
						cchip_key_button_event_handler(CLOUD_KEYHANDLE_PAUSE_PAUSE);
						break;
						
					case _KEY_WECHAT_SHORT_PRESS:
						cchip_key_button_event_handler(CLOUD_KEYHANDLE_WECHAT_MSG);
						break;
					case _KEY_WECHAT_LONG_PRESS:
						cchip_key_button_event_handler(CLOUD_KEYHANDLE_WECHAT);
						break;
						
					case _KEY_VOICE_SHORT_PRESS:
						//cchip_key_button_event_handler(CLOUD_KEYHANDLE_PAUSE_PAUSE);
						mw_asr_client_touch_wakeup();
						break;

				default:
					break;
			}
		}
		else if(mmgmt_is_flash_mode())
		{
			switch(keyType)
			{
				case _KEY_COLLECT_SHORT_PRESS:
					flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_PLAY_PAUSE);
					break;
				case _KEY_NEXT_SHORT_PRESS:
					flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_SONG_NEXT);
					break;
				case _KEY_PREV_SHORT_PRESS:
					flash_player_key_button_event_handler(FLASH_PLAYER_KEYHANDLE_SONG_PREV);
					break;				
					break;
			}
		}
		if(keyType == _KEY_PREV_LONG_PRESS)//LED control
		{
			if(led_display_control == 0)
			{
				state_backup = get_led_state();
				led_display_control = 1;
				change_led_state(LED_STATE_TURNOFF);
			}
			else
			{
				led_display_control = 0;
				change_led_state(state_backup);
				gpio_output(LED_ED_PIN,0);
			}
		}
		if((0 == airkiss_is_start()) && (_KEY_CONFIG_NET_LONG_PRESS == keyType))
		{
			cchip_key_button_event_handler(CLOUD_KEYHANDLE_NET_CONFIG);
		}
	}
	if(keyType == _KEY_VOICE_LONG_PRESS)//child lock
	{
		child_lock = child_lock?0:1;
	}
	}
#if FACTORY_TEST_ENABLE 
	else
	{
		switch(keyType)
		{
			case _KEY_COLLECT_SHORT_PRESS:
				factory_mask |= 1;
				factory_test_play(_KEY_COLLECT_);
				break;
			case _KEY_VOICE_SHORT_PRESS:
				factory_mask |= 1 << 1;
				factory_test_play(_KEY_VOICE_);
				break;
			case _KEY_WECHAT_SHORT_PRESS:
				factory_test_play(_KEY_WECHAT_);
				factory_mask |= 1 << 2;
				break;
			case _KEY_NEXT_SHORT_PRESS:
				factory_test_play(_KEY_NEXT_);
				factory_mask |= 1 << 3;
				break;
			case _KEY_PREV_SHORT_PRESS:
				factory_test_play(_KEY_PREV_);
				factory_mask |= 1 << 4;
				break;
            case _KEY_WECHAT_LONG_PRESS:
                factory_test_play(_KEY_MIC_DAC_LOOP);
                factory_mask |= 1 << 5;
                break;
		}
		if(0x3f == factory_mask)
		{
			rt_thread_delay(1500);
			factory_test_play(TEST_FINISH);
		}
	}
#endif
}

static void key_next_short_press(void *param)
{
	rt_kprintf("next short\r\n");
	key_button_event_handler(_KEY_NEXT_SHORT_PRESS);
}

static void key_next_long_press(void *param)
{
rt_kprintf("next long\r\n");
	key_button_event_handler(_KEY_NEXT_LONG_PRESS);
}

static void key_prev_short_press(void *param)
{
rt_kprintf("prev short\r\n");
	key_button_event_handler(_KEY_PREV_SHORT_PRESS);
}

static void key_prev_long_press(void *param)
{
rt_kprintf("prev long\r\n");
	key_button_event_handler(_KEY_PREV_LONG_PRESS);
}

static void key_press_short_collect(void *param)
{
	rt_kprintf("collect short\r\n");
	key_button_event_handler(_KEY_COLLECT_SHORT_PRESS);
}
static void key_press_long_collect(void *param)
{
	rt_kprintf("collect long\r\n");
	key_button_event_handler(_KEY_COLLECT_LONG_PRESS);
}

static void key_cofig_net_press_long(void *param)
{
	rt_kprintf("config net press long\r\n");
	key_button_event_handler(_KEY_CONFIG_NET_LONG_PRESS);
}
static void key_voice_short_press(void *param)
{
	rt_kprintf("voice short\r\n");
	key_button_event_handler(_KEY_VOICE_SHORT_PRESS);
	//start_voice_config(my_voice_config_callback);
}
static void key_voice_long_press(void *param)
{
	rt_kprintf("voice long\r\n");
	key_button_event_handler(_KEY_VOICE_LONG_PRESS);
}
static void key_wechat_short_press(void *param)
{
	rt_kprintf("chat short\r\n");
	key_button_event_handler(_KEY_WECHAT_SHORT_PRESS);
}


static void key_wechat_long_press(void *param)
{
	rt_kprintf("chat long\r\n");
	key_button_event_handler(_KEY_WECHAT_LONG_PRESS);
}


typedef struct{
	KEYITEM key;
	void (*short_press)(void *arg);
	void (*double_press)(void *arg);
	void (*long_press)(void *arg);
	void (*hold_press)(void *arg);
}key_button_item_t;

const key_button_item_t key_button_table[]={
	{KEY_COMBO_S1S2_CONFIG_NET,NULL,NULL,key_cofig_net_press_long,NULL},
	{KEY_S1_PREV_PLAY,key_prev_short_press,NULL,key_prev_long_press,NULL},
	{KEY_S2_NEXT,key_next_short_press,NULL,key_next_long_press,NULL},
	{KEY_S3_COLLECT,key_press_short_collect,NULL,key_press_long_collect,NULL},
	{KEY_S4_WECHAT,key_wechat_short_press,NULL,key_wechat_long_press,NULL},
	{KEY_S5_VOICE,key_voice_short_press,NULL,key_voice_long_press,NULL},
};

void key_button_app_init(void)
{
	
#if CFG_ENABLE_BUTTON
	#if (0 == KEY_BUTTION_TABLE_CFG)
		//key_item_configure(MAP_KEY_PLAY_PAUSE_PIN, key_press_short_collect, NULL,key_press_long_collect,NULL );
		key_item_configure(MAP_KEY_VOLUME_ADD_NEXT, key_next_short_press,NULL,
								key_next_long_press,NULL);
		key_item_configure(MAP_KEY_VOLUME_REDUCE_PREV, key_prev_short_press,NULL,
								key_prev_long_press, NULL);
		key_item_configure(MAP_KEY_VOICE, key_voice_short_press, NULL, key_voice_long_press,NULL);
		key_item_configure(MAP_WECHAT, key_wechat_short_press, NULL, key_wechat_long_press,NULL);
	#elif KEY_BUTTION_TABLE_CFG
		int i;
		for(i=0;i<(sizeof(key_button_table)/sizeof(key_button_item_t));i++)
		{
			if((key_button_table[i].short_press)||(key_button_table[i].double_press)
				||(key_button_table[i].long_press) ||(key_button_table[i].hold_press))
			{
				matrix_key_item_configure(key_button_table[i].key, key_button_table[i].short_press, 
							key_button_table[i].double_press, key_button_table[i].long_press,
							key_button_table[i].hold_press);
			}
		}
	#endif
#endif
}


#endif

//////SD
static CHECK_STATUS_E sd_status = OFFLINE_STATUS;
static beken_timer_t sd_check_handle_timer = {0};
uint8_t get_sd_check_pin_status(void)
{
	return gpio_input(SD_STATUS_CHECK_PIN);
}

static CHECK_STATUS_E linein_status = ONLINE_STATUS;
uint8_t get_linein_check_pin_status(void)
{
	return gpio_input(LINEIN_STATUS_CHECK_PIN);
}

static void sd_check_handle_timer_callback( void * arg )  
{
	int level;
	static uint16 cnt_online = 0;
	static uint16 linein_online = 0;
    
    level = get_sd_check_pin_status();
    
	if(level == SD_INSTER_STATUS_CHECK_PIN_LEVEL)
	{
		if(OFFLINE_STATUS == sd_status)
		{
		    if (cnt_online < SD_DEBOUNCE_COUNT)
	        {
	            cnt_online ++;
	        }
			else
			{
				sd_status = ONLINE_STATUS;
				offline_player_sd_status_change_event(OFFLINE_PLAYER_SD_STATUS_INSTER);
			}
		}
	}
	else
	{
		if(ONLINE_STATUS == sd_status)
		{
			cnt_online = 0;
			sd_status = OFFLINE_STATUS;
			offline_player_sd_status_change_event(OFFLINE_PLAYER_SD_STATUS_REMOVE);
		}
	}

    level = get_linein_check_pin_status();
    
    if(level == LINEIN_INSTER_STATUS_CHECK_PIN_LEVEL)
	{
		if(OFFLINE_STATUS == linein_status)
		{
		    if (linein_online < SD_DEBOUNCE_COUNT)
	        {
	            linein_online ++;
	        }
			else
			{
				linein_status = ONLINE_STATUS;
                offline_player_sd_status_change_event(OFFLINE_PLAYER_LINEIN_STATUS_INSTER);
                rt_kprintf("ONLINE_STATUS\r\n");
			}
		}
	}
	else
	{
		if(ONLINE_STATUS == linein_status)
		{
			linein_online = 0;
			linein_status = OFFLINE_STATUS;
            offline_player_sd_status_change_event(OFFLINE_PLAYER_LINEIN_STATUS_REMOVE);
            rt_kprintf("OFFLINE_STATUS\r\n");
		}
	}
    
} 

void send_keymode_2_offline(void)
{
	offline_player_key_button_event_handler(OFFLINE_PLAYER_KEYHANDLE_MODE_CHANGE);
}

void send_keymode_2_turing(void)
{
	cchip_key_button_event_handler(CLOUD_KEYHANDLE_MODE_CHANGE);
}

int sd_is_online(void)
{
	return (sd_status == ONLINE_STATUS)?1:0;
}

void sd_check_init(void)
{
	OSStatus err;
	
	gpio_config(SD_STATUS_CHECK_PIN, GMODE_INPUT_PULLUP);
    gpio_config(LINEIN_STATUS_CHECK_PIN, GMODE_INPUT_PULLUP);

	err = rtos_init_timer(&sd_check_handle_timer, 
							SD_CHECK_SCAN_INTERVAL_MS, 
							sd_check_handle_timer_callback, 
							NULL);
	err = rtos_start_timer(&sd_check_handle_timer);
		ASSERT(kNoErr == err);	

    ASSERT(kNoErr == err);
}
