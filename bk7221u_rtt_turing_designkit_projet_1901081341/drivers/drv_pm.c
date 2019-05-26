#include <rthw.h>
#include <rtthread.h>

#include <stdint.h>

extern void WFI(void);

/*
0: normal
1: cpu sleep. -4mA
2: RF sleep. 27mA
3: RF+CPU sleep. 7mA
4: standby to reset. 7uA
*/

static void idle_hook(void)
{
    WFI();
}

static int drv_pm_init(void)
{
    rt_kprintf("%s\n", __FUNCTION__);
    rt_thread_idle_sethook(idle_hook);

    return 0;
}
INIT_DEVICE_EXPORT(drv_pm_init);
