#include <rtthread.h>
#include "rthw.h"
#include "mode_mgmt.h"
#include "cchip_play_tip.h"
#include "list_player.h"
#include "key_app.h"

static uint32_t mmgmt_switch_count = 0;
static uint32_t mmgmt_phase = MODE_PHASE_UNCERTAIN;
static uint32_t mmgmt_flag = 0;
SYS_MODE_E g_sys_mode = MODE_CLOUD_PLAYER;

/* attention: system mode is confirmed, if wifi connected*/
uint32_t mmgmt_initial_mode_confirm(void)
{
	MMGMT_PRINTF("mmgmt_initial_mode_confirm\r\n");
	
	if(MODE_PHASE_UNCERTAIN == mmgmt_phase)
	{
		mmgmt_phase = MODE_PHASE_DECIDED;
	}
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_fuzzy_mode(void)
{
	MMGMT_PRINTF("mmgmt_fuzzy_mode\r\n");
	mmgmt_phase = MODE_PHASE_UNCERTAIN;
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_try_wifi_connected(void)
{
	MMGMT_PRINTF("mmgmt_try_wifi_connected\r\n");
	mmgmt_flag = MODE_FLAG_WIFI_TRY_CONNECT;
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_attempted_connect(void)
{
	MMGMT_PRINTF("mmgmt_attempted_connect\r\n");
	mmgmt_flag = MODE_FLAG_WIFI_ATTEMPTED_CONNECT;
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_no_wifi_connected(void)
{
	MMGMT_PRINTF("mmgmt_no_wifi_connected\r\n");
	mmgmt_flag = MODE_FLAG_NO_WIFI_CONNECT;
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_set_mode(SYS_MODE_E md)
{
	int level;
	
	if(MODE_MAX_COUNT <= md)
	{
		return MMGMT_FAILURE;
	}
	
	MMGMT_PRINTF("mmgmt_set_mode\r\n");
	level = rt_hw_interrupt_disable();
	g_sys_mode = md & 0xFF;
	rt_hw_interrupt_enable(level);
	
	return MMGMT_SUCCESS;
}

SYS_MODE_E mmgmt_get_mode(void)
{
	return g_sys_mode;
}

uint32_t mmgmt_is_offline_mode(void)
{
	return (MODE_OFFLINE_PLAYER == g_sys_mode);
}

uint32_t mmgmt_is_cloud_mode(void)
{
	return (MODE_CLOUD_PLAYER == g_sys_mode);
}
uint32_t mmgmt_is_flash_mode(void)
{
	return (MODE_FLASH_PLAYER == g_sys_mode);
}

uint32_t mmgmt_switch_offline2cloud(void)
{
	MMGMT_PRINTF("mmgmt_switch_offline2cloud\r\n");
	
	send_play_tip_event(TURING_PLAYTIP_WIFI_MODE);
	mmgmt_set_mode(MODE_CLOUD_PLAYER);
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_switch_cloud2offline(void)
{
	MMGMT_PRINTF("mmgmt_switch_cloud2offline\r\n");
	if (list_player_is_exist()) 
	{
        WaitForTipPlayFinshed();
		list_player_detach_items();
    }
	
	send_keymode_2_offline();
	
	return MMGMT_SUCCESS;
}
#if FACTORY_TEST_ENABLE
uint32_t mmgmt_is_factory_test_mode(void)
{
	return (MODE_FACTORY_TEST == g_sys_mode);
}

uint32_t mmgmt_switch2_factory_test(void)
{
	mmgmt_set_mode(MODE_FACTORY_TEST);
}
#endif
uint32_t mmgmt_switch(void)
{				
    SYS_MODE_E current_mode;
	extern int play_num;

	MMGMT_PRINTF("mmgmt_switch\r\n");

	mmgmt_switch_count ++;
	current_mode = mmgmt_get_mode();	
    if (MODE_CLOUD_PLAYER == current_mode) 
    {   
        if (!play_num)
        {
            MMGMT_PRINTF("mmgmt_switch NOT ready\r\n");
		    return MMGMT_FAILURE;
	    }

		mmgmt_switch_cloud2offline();
    }
	else if(MODE_OFFLINE_PLAYER == current_mode) 
    {
    	mmgmt_switch_offline2cloud();
    }
	
	return MMGMT_SUCCESS;
}

uint32_t mmgmt_determine_initial_offline_mode(void)
{
	MMGMT_PRINTF("mmgmt_switch:%d:%d\r\n", mmgmt_phase, mmgmt_flag);
	if(((MODE_PHASE_UNCERTAIN == mmgmt_phase)
		&& (MODE_FLAG_WIFI_ATTEMPTED_CONNECT == mmgmt_flag))
		|| (MODE_FLAG_NO_WIFI_CONNECT == mmgmt_flag)
		|| (mmgmt_is_flash_mode()))
	{
		mmgmt_phase = MODE_PHASE_DECIDED;
		
		mmgmt_switch_cloud2offline();
	}
}
extern int scan_failed_too_many_times(void);
int mmgmt_swith_to_flash_mode(void)
{
	rt_kprintf("====switch to flash mode:%x====\r\n",mmgmt_flag);
	if((MODE_FLAG_NO_WIFI_CONNECT == mmgmt_flag)||
		wfm_assoc_fail_too_many_times()||
		scan_failed_too_many_times())
	{
		flash_player_enter();
		return 1;
	}
	return 0;
}
// eof
