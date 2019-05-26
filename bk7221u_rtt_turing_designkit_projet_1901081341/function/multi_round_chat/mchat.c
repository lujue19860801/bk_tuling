#include <rtthread.h>
#include "mchat.h"
#include "cchip_key_handle.h"

static uint32_t mchat_flag = MCHAT_STOP;

uint32_t mchat_start(void)
{
	mchat_flag = MCHAT_ENTER;
	MCHAT_PRINTF("mchat_start\r\n");
	
	send_key_handle_event(TURING_KEYHANDLE_CAPTURE_START);

	return MCHAT_SUCCESS;
}

uint32_t mchat_continue(void)
{
	if(MCHAT_STOP == mchat_flag)
	{
		MCHAT_PRINTF("mchat_continue_return\r\n");
		return MCHAT_FAILURE;
	}
	
	mchat_flag = MCHAT_KEEP_UP;
	MCHAT_PRINTF("mchat_continue\r\n");
	send_key_handle_event(TURING_KEYHANDLE_CAPTURE_START);
	
	return MCHAT_SUCCESS;
}

uint32_t mchat_stop(void)
{
	mchat_flag = MCHAT_STOP;
	MCHAT_PRINTF("mchat_stop\r\n");
	return MCHAT_SUCCESS;
}

uint32_t mchat_is_otg(void)
{
	return (MCHAT_KEEP_UP == mchat_flag);
}

uint32_t mchat_is_enter(void)
{
	return (MCHAT_ENTER == mchat_flag);
}

void mchat_enter(void)
{
    mchat_flag = MCHAT_ENTER;
}
// eof
