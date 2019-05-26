#include "intc_pub.h"
#include "pwm_pub.h"
#include "rw_pub.h"
#include "rtos_pub.h"
#include "arm_arch.h"
#include "sys_ctrl_pub.h"
#include "mcu_ps.h"
#include "mcu_ps_pub.h"
#include "power_save_pub.h"
#include "ps_debug_pub.h"
#include "target_util_pub.h"
#include "fake_clock_pub.h"
#include "icu_pub.h"
#include "timer_pub.h"


static MCU_PS_INFO mcu_ps_info =
{
    .mcu_ps_on = 0,
    .peri_busy_count = 0,
    .mcu_prevent = 0,

};

void peri_busy_count_add(void )
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    mcu_ps_info.peri_busy_count ++;
    GLOBAL_INT_RESTORE();
}

void peri_busy_count_dec(void )
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    mcu_ps_info.peri_busy_count --;
    GLOBAL_INT_RESTORE();
}

UINT32 peri_busy_count_get(void )
{
    return mcu_ps_info.peri_busy_count;
}

void mcu_prevent_set(UINT32 prevent )
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    mcu_ps_info.mcu_prevent |= prevent;
    GLOBAL_INT_RESTORE();
}

void mcu_prevent_clear(UINT32 prevent )
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    mcu_ps_info.mcu_prevent &= ~ prevent;
    GLOBAL_INT_RESTORE();
}

UINT32 mcu_prevent_get(void )
{
    return mcu_ps_info.mcu_prevent;
}

void ps_pwm0_enable(void)
{
    UINT32 reg = 0;
    reg = REG_READ(PWM_CTL);
    reg &= (~0xf);
    reg |= 0x7;
    REG_WRITE(PWM_CTL, reg);
}

void ps_pwm0_disable(void )
{
    UINT32 reg;
    reg = REG_READ(PWM_CTL);
    REG_WRITE(PWM_CTL, reg & (~0xf));
}

void ps_pwm0_set_period(UINT32 period, UINT8 clk_mux)
{
    UINT32 reg = 0;
    reg = REG_READ(ICU_PWM_CLK_MUX);
    reg &= ~(CO_BIT(PWM0_BIT));
    reg |= (clk_mux << PWM0_BIT);
    REG_WRITE(ICU_PWM_CLK_MUX, reg);
#if (CFG_SOC_NAME == SOC_BK7231)
    reg = 0;
    reg |= period << PWM0_END_POSI | 0x0 << PWM0_DC_POSI;
    REG_WRITE(PWM0_COUNTER , reg);
#else
	reg = period;
    REG_WRITE(PWM0_COUNTER , reg);
    reg = 0;
    REG_WRITE(PWM0_DUTY_CYCLE , reg);   
#endif
}

void ps_pwm0_reconfig(UINT32 period, UINT8 clk_mux)
{
    UINT32 param;
    //disable
#if (CFG_SOC_NAME == SOC_BK7231)	
    ps_pwm0_disable();
    delay(5);
    //new
    ps_pwm0_set_period(period, clk_mux);
    delay(1);
    //reenable
    ps_pwm0_enable();
#else
    ps_pwm0_disable();
    //new
    ps_pwm0_set_period(period, clk_mux);
    //reenable
    ps_pwm0_enable();
    delay(5);

    REG_WRITE(PWM_INTERRUPT_STATUS,0x3f);
#endif
	
}

UINT32 mcu_power_save(UINT32 sleep_tick)
{
    UINT32 sleep_ms, sleep_pwm_t, param, miss_ticks = 0;
    UINT32 real_sleep_ms, start_sleep, ret, front_ms, wkup_type,reg;
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

    if(mcu_ps_info.mcu_ps_on == 1
            && (peri_busy_count_get() == 0)
            && (mcu_prevent_get() == 0)
            && (txl_sleep_check()
                && (! power_save_use_timer0()))
      )
    {
        uart_wait_tx_over();
        sleep_ms = sleep_tick * MS_TO_TICK;
#if (CFG_SOC_NAME == SOC_BK7231)
        sleep_pwm_t = ((sleep_ms * 102400) / 3125) ;

#else
        sleep_pwm_t = (sleep_ms * 32);
#endif
        if(sleep_pwm_t > 65535)
            sleep_pwm_t = 65535;
        else if(sleep_pwm_t < 160)
            sleep_pwm_t = 160;

        start_sleep = hal_machw_time();
#if (CHIP_U_MCU_WKUP_USE_TIMER && (CFG_SOC_NAME != SOC_BK7231))
        ps_pwm0_disable();

        REG_WRITE(TIMER3_CNT,sleep_pwm_t);
        delay(5);
        reg = REG_READ(TIMER3_5_CTL);
        reg |= (TIMERCTL3_EN_BIT);
        REG_WRITE(TIMER3_5_CTL,reg);

        param = (0xfffff & (~PWD_TIMER_32K_CLK) & (~PWD_UART2_CLK)
#if (!PS_NO_USE_UART1_WAKE)
                 & (~PWD_UART1_CLK)
#endif
                );
#else
        ps_pwm0_reconfig(sleep_pwm_t, PWM_CLK_LPO);
        param = (0xfffff & (~PWD_PWM0_CLK) & (~PWD_UART2_CLK)
#if (!PS_NO_USE_UART1_WAKE)
                 & (~PWD_UART1_CLK)
#endif
                );
#endif

        sctrl_mcu_sleep(param);
        wkup_type = sctrl_mcu_wakeup();
        real_sleep_ms = (hal_machw_time() -  start_sleep) / 1000;

        if(wkup_type == 0)
        {
            miss_ticks = (real_sleep_ms / MS_TO_TICK);
            miss_ticks = ((miss_ticks > sleep_tick) ? sleep_tick : miss_ticks);
        }
        else
        {
            miss_ticks = sleep_tick;
        }
#if (CHIP_U_MCU_WKUP_USE_TIMER && (CFG_SOC_NAME != SOC_BK7231))
        reg = REG_READ(TIMER3_5_CTL);
        reg &= (~TIMERCTL3_EN_BIT);
        REG_WRITE(TIMER3_5_CTL,reg);
        ps_pwm0_enable();

#else
        ps_pwm0_reconfig(fclk_cal_endvalue(PWM_CLK_26M), PWM_CLK_PCLK);
#endif
    }
    else
    {
    }

    fclk_update_tick(miss_ticks);
    GLOBAL_INT_RESTORE();
    ASSERT(miss_ticks >= 0);
    return miss_ticks;
}

void mcu_ps_dump(void)
{
    os_printf("mcu:%x\r\n", mcu_ps_info.mcu_ps_on);
}

#if (CHIP_U_MCU_WKUP_USE_TIMER)
void timer3_isr(UINT8 param)
{
    os_printf("t3\r\n");
}

void mcu_init_timer3(void)
{
	timer_param_t param;
	param.channel = 3;
	param.div = 1;
	param.period = 50 * 32;
	param.t_Int_Handler= timer3_isr;

	sddev_control(TIMER_DEV_NAME, CMD_TIMER_INIT_PARAM, &param);
}
#endif

void mcu_ps_init(void)
{
    UINT32 reg;
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

#if (CHIP_U_MCU_WKUP_USE_TIMER && (CFG_SOC_NAME != SOC_BK7231))
    mcu_init_timer3();
#endif

    if(0 == mcu_ps_info.mcu_ps_on)
    {
        sctrl_mcu_init();
        mcu_ps_info.mcu_ps_on = 1;
        mcu_ps_info.peri_busy_count = 0;
        mcu_ps_info.mcu_prevent = 0;
        os_printf("%s\r\n", __FUNCTION__);
    }

    GLOBAL_INT_RESTORE();
}

void mcu_ps_exit(void)
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();

    if(1 == mcu_ps_info.mcu_ps_on)
    {
        mcu_ps_info.mcu_ps_on = 0;
        sctrl_mcu_exit();
        mcu_ps_info.peri_busy_count = 0;
        mcu_ps_info.mcu_prevent = 0;
        os_printf("%s\r\n", __FUNCTION__);
    }

    GLOBAL_INT_RESTORE();
}
