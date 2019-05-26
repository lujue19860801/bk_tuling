#include <rthw.h>
#include <rtthread.h>

#include <stdint.h>
#include <stdlib.h>

extern int beken_pm_level;

extern int bk_wlan_dtim_rf_ps_mode_enable(void );
extern int bk_wlan_dtim_rf_ps_mode_need_disable(void);

extern int bk_wlan_mcu_ps_mode_enable(void);
extern int bk_wlan_mcu_ps_mode_disable(void);

extern int bk_wlan_dtim_with_normal_open(void);
extern int bk_wlan_dtim_with_normal_close(void);

extern void power_save_set_linger_time(uint32_t data_wakeup_time);

static int pm_level(int argc, char **argv)
{
	uint32_t level;

    if(argc != 2)
    {
        rt_kprintf("input argc is err!\n");
		return -1;
    }

	level = atoi(argv[1]);
	if(level > 99)
	{
		level = 99;
	}

	rt_kprintf("power_save_set_linger_time: %d => %d\n", level);
	power_save_set_linger_time(level);
	//beken_pm_level = level;

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

MSH_CMD_EXPORT(pm_level, pm_level 1);

FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_rf_ps_mode_enable, __cmd_rf_ps, bk_wlan_dtim_rf_ps_mode_enable);
//FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_rf_ps_mode_need_disable, __cmd_rf_ps_disable, bk_wlan_dtim_rf_ps_mode_need_disable);

FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_mcu_ps_mode_enable, __cmd_mcu_ps, bk_wlan_mcu_ps_mode_enable);
FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_mcu_ps_mode_disable, __cmd_mcu_ps_disable, bk_wlan_mcu_ps_mode_disable);

//FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_with_normal_open, __cmd_dtim_normal_open, bk_wlan_dtim_with_normal_open);
//FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_with_normal_close, __cmd_dtim_normal_close, bk_wlan_dtim_with_normal_close);

#endif /* RT_USING_FINSH */

