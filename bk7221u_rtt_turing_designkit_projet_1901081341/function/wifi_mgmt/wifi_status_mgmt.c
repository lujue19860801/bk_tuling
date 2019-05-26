#include "rtthread.h"
#include "include.h"
#include <wlan_ui_pub.h>
#include "wifi_status_mgmt.h"
#include "cchip_play_tip.h"
#include "cchip_led_control.h"
#include "TuringDispatcher.h"
#include "key_app.h"
#include "TuringPlaylist.h"
#include "easyflash.h"
#include "rw_pub.h"
#include "mode_mgmt.h"
#include "flash_music_list.h"
#include "wlan_cmd.h"
#include "TuringConfig.h"
#include "TuringInterJson.h"

static uint32_t wifi_connect_flag = 0;
static uint32_t wifi_successive_failed_count = 0;
#if FACTORY_TEST_ENABLE
static uint32_t factory_ssid_connect = 0;
void set_factory_ssid(uint32_t flag)
{
	factory_ssid_connect = flag;
}
#endif
uint32_t wfm_get_wifi_connect_status(void)
{
    return wifi_connect_flag;
}

void wfm_sta_connect_start_func(void *ctx)
{
    WFM_PRINTF("wifi start connect\r\n");
}

void wfm_connection_lost_func(void *ctx)
{
	char *value;
	
    WFM_PRINTF("connection_lost\r\n");

    wifi_connect_flag = 0;

    list_player_stop();

    value = ef_get_env("airkissflag");
    if(value)
    {
        if(1 == atoi(value))
        {
            send_play_tip_event(TURING_SPEAKER_NOT_CONNECT);
        }
    }
    else
    {
        send_play_tip_event(TURING_SPEAKER_NOT_CONNECT);
    }
}

void wfm_auth_fail_func(void *ctx)
{
    uint16_t *param = (uint16_t *)ctx;
    WFM_PRINTF("--- auth fail:%x ----", *param);

    wifi_connect_flag = 0;
	wifi_successive_failed_count ++;
	
	mmgmt_attempted_connect();
	if(CONNECT_FAILED_MAX_COUNT_OF_INITIAL_MODE < wifi_successive_failed_count)
	{
		mmgmt_determine_initial_offline_mode();
	}
	
    send_play_tip_event(TURING_PLAYTIP_WIFI_CONNECT_FAIL);

	flash_player_enter();
}

int wfm_assoc_fail_too_many_times(void)
{
	return (CONNECT_FAILED_MAX_COUNT_OF_INITIAL_MODE < wifi_successive_failed_count)?1:0;
}
void wfm_assoc_fail_func(void *ctx)
{
    uint16_t *param = (uint16_t *)ctx;
    WFM_PRINTF("--- assoc fail:%x ----", *param);

    wifi_connect_flag = 0;
	wifi_successive_failed_count ++;

	mmgmt_attempted_connect();
	if(CONNECT_FAILED_MAX_COUNT_OF_INITIAL_MODE < wifi_successive_failed_count)
	{
		mmgmt_determine_initial_offline_mode();
	}

    send_play_tip_event(TURING_PLAYTIP_WIFI_CONNECT_FAIL);

	flash_player_enter();
}

void wfm_connected_func(void)
{
    static int connect_success_count = 0;
    char *ssidbuf = NULL, *pswbuf = NULL;

    WFM_PRINTF("---- connected ----");
    play_num = 0;
    wifi_connect_flag = 1;
	wifi_successive_failed_count = 0;
	change_led_state(LED_STATE_CONNECT_OK);
#if FACTORY_TEST_ENABLE
	if(1 == factory_ssid_connect)
	{
		mmgmt_switch2_factory_test();
		factory_test_play(TEST_START);
	}
	else
#endif
	{	
		mmgmt_initial_mode_confirm();
	    if(0 == connect_success_count)
	    {
	    	/*if(mmgmt_is_cloud_mode())
	    	{
	        	send_play_tip_event(TURING_PLAYTIP_WIFI_CONNECT_SUCCESS);
	    	}*/
	        connect_success_count++;
	    }

	    //TuringDispatcherQueueSend(TURING_DISPATCHER_REPORT_DEVICE_STATUS);
	}
	
    if (wifi_get_setting(&ssidbuf, &pswbuf) == 0)
    {
        if ((ssidbuf) && (pswbuf))
        {
            wifi_save_ap(ssidbuf, pswbuf);
        }
    }

}

void wfm_callback_register(void)
{
    user_connected_callback(wfm_connected_func);
    user_callback_func_register(wfm_sta_connect_start_func,
                                wfm_connection_lost_func,
                                wfm_auth_fail_func,
                                wfm_assoc_fail_func);
}
// eof

