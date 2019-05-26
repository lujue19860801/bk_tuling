#include "rtthread.h"
#include <dfs.h>
#include <dfs_fs.h>
#include <easyflash.h>
#include <wlan_ui_pub.h>
#include "player.h"
#include "player_app.h"
#include "include.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "app.h"
#include "ate_app.h"
#include "shell.h"
#include "debug.h"
#include "ServicesList.h"
#include "cchip_play_tip.h"
#include "cchip_key_check.h"
#include "TuringDispatcher.h"
#include "TuringPlaylist.h"
#include "key_app.h"
#include "adc_app.h"
#include "cchip_led_control.h"
#include "cchip_adccheck.h"
#include "wifi_status_mgmt.h"
#include "saradc_intf.h"
#include "airkiss.h"
#include "mw_asr.h"

static int wlan_app_init(void);
extern rt_err_t rt_audio_codec_hw_init(void);
extern int rt_audio_adc_hw_init(void);
extern int player_system_init(void);
extern void user_connected_callback(FUNCPTR fn);
extern int airkiss(int argc, char *argv[]);
extern rt_err_t rt_thread_sleep(rt_tick_t tick);
extern void player_bell_change_to_flash0(void);

extern const struct romfs_dirent romfs_root;
#define DFS_ROMFS_ROOT          (&romfs_root) 

int main(int argc, char **argv)
{
    easyflash_init();
	
    /* mount ROMFS as root directory */
    if (dfs_mount(RT_NULL, "/", "rom", 0, (const void *)DFS_ROMFS_ROOT) == 0)
    {
        rt_kprintf("ROMFS File System initialized!\n");
    }
    else
    {
        rt_kprintf("ROMFS File System initialized Failed!\n");
    }

#if 1
	if(dfs_mount("flash_disk0","/flash0", "elm", 0, 0) == 0)
	{
		rt_kprintf("elm file system in flash0 initialized! \r\n");
        player_bell_change_to_flash0();
	}
	else
	{
		rt_kprintf("elm file system in flash0 init failed!! \r\n");
	}
#endif
    wlan_app_init();

    return 0;
}

static void control_debug(void)
{
    char *value = NULL;
    int volume =-1;
    value = ef_get_env("debug");
    if (value)
    {
        volume =atoi(value) ;
        if(volume == 0)
        {
            log_set_quiet(1);
        }
    }
    else
    {
        log_set_quiet(1);
    }
}

void send_tip_start_system(void)
{
	send_play_tip_event(TURING_PLAYTIP_PLAY_BOOT_MUSIC);
}

void asr_message_cb_demo(MwAsrStatus status,void* info)
{
	#define	ASSERT_NOT_NULL(s)	do{if(!s) return;}while(0)
	MwAsrSceneResult* pScene;
	MwAsrErrorResult* pError;
	int old_value;

	if(status==ASR_STATE_ERROR){
		ASSERT_NOT_NULL(info);
		pError = (MwAsrErrorResult*)info;
		mw_asr_client_end_session(0,0);
	}else if(status==ASR_STATE_OK){
		ASSERT_NOT_NULL(info);
		pScene = (MwAsrSceneResult*)info;
		switch(pScene->scene){
			case ASR_SCENE_MUSIC:
				mw_asr_client_end_session(0,1);
				switch(pScene->sceneData.scene_music.type)
				{
					case MUSIC_OP_PLAY:
						break;
					case MUSIC_OP_PAUSE:
						break;
					case MUSIC_OP_RESUME:
						break;
					case MUSIC_OP_NEXT:
						break;
					case MUSIC_OP_PREV:
						break;
				}
				break;
			case ASR_SCENE_VOLUME:
			{
				switch (pScene->sceneData.scene_volume.op)
				{
					case VOLUME_UP:
						break;
					case VOLUME_DOWN:
						break;
					case VOLUME_MINIMIZE:
						break;
					case VOLUME_MAXIMIZE:
						break;
					default:
						break;
				}
				break;
			}
				
			case ASR_SCENE_OTHER:
			default:
				if(pScene->sceneData.scene_common.url==NULL){
					//aplayer_add_notify_item(pScene->sceneData.scene_common.prompt,NULL);
				}else{
					//aplayer_add_sync_notify_item(pScene->sceneData.scene_common.prompt,pScene->sceneData.scene_common.url,asr_sync_status_cb);
				}
				break;
		}
	}
}

void my_audio_stream_callback(void* buffer,rt_uint32_t size)
{
	mw_asr_client_write_audio((unsigned char*)buffer, size);
}

void my_player_init()
{
	//offline_player_init();
}

int user_app_start(void)
{
#ifdef BEKEN_START_WDT
    rt_hw_wdg_start(0, NULL);
#endif

    log_set_level(LOG_INFO);
    control_debug();
    //cchip_key_init();
    //services_init();
#if (USING_DEMO_BOARD != 1)
	//led_init();
	led_app_init();
    //turing_adc_create();
    MwAsrClientCbs asr_cbs;
	asr_cbs.asrstate_cb = asr_message_cb_demo;
    mw_asr_client_create("",1,asr_cbs);
	mw_audio_stream_init(my_audio_stream_callback);
#endif

    usb_charging_detect_open();
    wfm_callback_register();
    //star_airkiss();
}

#ifdef BEKEN_USING_WLAN
extern void ate_app_init(void);
extern void ate_start(void);

static int wlan_app_init(void)
{
    /* init ate mode check. */
    ate_app_init();

    if (get_ate_mode_state())
    {
        rt_kprintf("\r\n\r\nEnter automatic test mode...\r\n\r\n");

        finsh_set_echo(0);
        finsh_set_prompt("#");

        ate_start();
    }
    else
    {
        rt_kprintf("Enter normal mode...\r\n\r\n");
        app_start();
		key_button_app_init();
		
        saradc_work_create();
		adc_button_app_init();
		rt_audio_codec_hw_init();
		rt_audio_adc_hw_init();

		my_player_init();
		//player_system_init();
		sd_check_init();
		rt_kprintf("Chalmers 2019-02-22\r\n\r\n");
        user_app_start();
    }

    return 0;
}
#endif
// eof
