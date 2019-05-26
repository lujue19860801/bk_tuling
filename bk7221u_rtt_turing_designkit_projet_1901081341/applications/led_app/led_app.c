#include "led_app.h"
#include "BkDriverPwm.h"
#include "gpio_pub.h"

#define	LED_ARRAY_ROW		(3)
#define	LED_ARRAY_COL		(5)

UINT32 pins_row[LED_ARRAY_ROW] = {GPIO19,GPIO17,GPIO14};
UINT32 pins_col[LED_ARRAY_COL] = {GPIO16,GPIO15,GPIO18,GPIO24,GPIO26};


void led_app_init(void)
{
	int i;
	//bk_pwm_initialize(BK_PWM_0,50,50.0);
	//bk_pwm_start(BK_PWM_0);
	for(i=0;i<LED_ARRAY_ROW;i++){
		gpio_config(pins_row[i],GMODE_OUTPUT);
	}
	for(i=0;i<LED_ARRAY_COL;i++){
		gpio_config(pins_col[i],GMODE_OUTPUT);
	}
	for(i=0;i<LED_ARRAY_ROW;i++){
		gpio_output(pins_row[i],1);
	}
	for(i=0;i<LED_ARRAY_COL;i++){
		gpio_output(pins_col[i],0);
	}
}

void led_app_deinit(void)
{
	//bk_pwm_stop(BK_PWM_0);
}
