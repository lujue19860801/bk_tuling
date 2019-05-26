#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "airkiss.h"
#include <stdio.h>
#include <sys/socket.h>
#include "cchip_play_tip.h"
#include "wlan_cmd.h"
#include "flash_music_list.h"

#define AIRKISS_PRINTF          rt_kprintf

#define AIRKISS_SWITCH_TIMER    rt_tick_from_millisecond(50)    // ms
#define AIRKISS_DOING_TIMER     rt_tick_from_millisecond(20000)  // 20s
#define MAX_CHANNEL_NUM         13

uint32_t airkiss_start_flag = 0;
const airkiss_config_t airkiss_cfg_ptr =
{
    (airkiss_memset_fn) &rt_memset,
    (airkiss_memcpy_fn) &rt_memcpy,
    (airkiss_memcmp_fn) &rt_memcmp,
    (airkiss_printf_fn) &rt_kprintf,
};
static airkiss_context_t *airkiss_context_ptr = RT_NULL;

static rt_timer_t airkiss_switch_channel_timer;
static rt_timer_t g_airkiss_lock_channel_timer;
static struct rt_wlan_device *g_wlan_device = RT_NULL;
static volatile uint8_t g_current_channel;
static int airkiss_recv_ret;
static rt_sem_t airkiss_decode_over_sema;

void airkiss_set_start_flag(void)
{
    airkiss_start_flag = 1;
}

void airkiss_clear_start_flag(void)
{
    airkiss_start_flag = 0;
}

uint32_t airkiss_is_start(void)
{
    return airkiss_start_flag;
}

static void airkiss_switch_channel_timer_handler(void *parameter)
{
    g_current_channel ++;
    if (g_current_channel > MAX_CHANNEL_NUM)
    {
        g_current_channel = 1;
    }

    AIRKISS_PRINTF("switch_channel_to_index:%d \n", g_current_channel);
    rt_wlan_set_channel(g_wlan_device, g_current_channel);
    airkiss_change_channel(airkiss_context_ptr);
}

static void airkiss_lock_timeout_handler(void *parameter)
{
    int ret;

    AIRKISS_PRINTF("\r\nairkiss_lock_timeout_handler\r\n");

    ret = airkiss_init(airkiss_context_ptr, &airkiss_cfg_ptr);
    if (ret != RT_EOK)
    {
        rt_kprintf("airkiss_init_failed\r\n");
    }

    rt_timer_start(airkiss_switch_channel_timer);
}

static void airkiss_monitor_callback(uint8_t *data, int len, void *user_data)
{
    airkiss_recv_ret = airkiss_recv(airkiss_context_ptr, data, len);
    if (airkiss_recv_ret == AIRKISS_STATUS_CHANNEL_LOCKED)
    {
        rt_timer_stop(airkiss_switch_channel_timer);
        AIRKISS_PRINTF("lock_channel in %d \n", g_current_channel);

        rt_timer_start(g_airkiss_lock_channel_timer);
    }
    else if (airkiss_recv_ret == AIRKISS_STATUS_COMPLETE)
    {
        rt_timer_stop(g_airkiss_lock_channel_timer);

        rt_sem_release(airkiss_decode_over_sema);
        AIRKISS_PRINTF("AIRKISS_STATUS_COMPLETE \n");
    }
}

static int airkiss_get_wifi_status(struct netif *netif)
{
    ip_addr_t ip_addr;
    int result = 0;

    ip_addr_set_zero(&ip_addr);
    if (ip_addr_cmp(&(netif->ip_addr), &ip_addr))
        result = 0;
    else
    {
        result = 1;
        rt_kprintf("Got IP address : %s\n", ipaddr_ntoa(&(netif->ip_addr)));
    }

    return result;
}

static void airkiss_wifi_connect(airkiss_result_t *airkiss_result)
{
    rt_kprintf("airkiss_wifi_connect %s, %s\n", airkiss_result->ssid, airkiss_result->pwd);
    rt_kprintf("airkiss_wifi_connect %d, %d\n", airkiss_result->ssid_length, airkiss_result->pwd_length);

    wifi_set_mode(WIFI_STATION);
    wifi_set_setting(airkiss_result->ssid, airkiss_result->pwd);
    wifi_default();
}

static void airkiss_send_notification_thread(void *parameter)
{
    int sock = -1;
    int udpbufsize = 2;
    uint8_t random = (uint32_t)parameter;
    struct sockaddr_in g_stUDPBCAddr, g_stUDPBCServerAddr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        AIRKISS_PRINTF("notify create socket error!\n");
        goto _exit;
    }

    g_stUDPBCAddr.sin_family = AF_INET;
    g_stUDPBCAddr.sin_port = htons(10000);
    g_stUDPBCAddr.sin_addr.s_addr = htonl(0xffffffff);
    rt_memset(&(g_stUDPBCAddr.sin_zero), 0, sizeof(g_stUDPBCAddr.sin_zero));

    g_stUDPBCServerAddr.sin_family = AF_INET;
    g_stUDPBCServerAddr.sin_port = htons(10000);
    g_stUDPBCServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(g_stUDPBCServerAddr.sin_zero), 0, sizeof(g_stUDPBCServerAddr.sin_zero));

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &udpbufsize, sizeof(int)) != 0)
    {
        AIRKISS_PRINTF("notify socket setsockopt error\n");
        goto _exit;
    }

    if (bind(sock, (struct sockaddr *)&g_stUDPBCServerAddr, sizeof(g_stUDPBCServerAddr)) != 0)
    {
        AIRKISS_PRINTF("notify socket bind error\n");
        goto _exit;
    }

    for (int i = 0; i <= 20; i++)
    {
        int ret = sendto(sock, (char *)&random, 1, 0, (struct sockaddr *)&g_stUDPBCAddr, sizeof(g_stUDPBCAddr));
        rt_thread_delay(10);
    }

    AIRKISS_PRINTF("airkiss notification thread exit!\n");

_exit:
    if (sock >= 0)
    {
        close(sock);
    }
}

uint32_t airkiss_resource_create(void)
{
    int result = AIRKISS_SUCCESS;

    AIRKISS_PRINTF("airkiss_resource_create\r\n");
    airkiss_switch_channel_timer = rt_timer_create("switch_channel",
                                   airkiss_switch_channel_timer_handler,
                                   RT_NULL,
                                   AIRKISS_SWITCH_TIMER,
                                   RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    if (!airkiss_switch_channel_timer)
    {
        rt_kprintf("Create airkiss swtich channel timer failed \n");
        result = AIRKISS_FAILURE;
        goto init_exit;
    }

    g_airkiss_lock_channel_timer = rt_timer_create("doing_timeout",
                                   airkiss_lock_timeout_handler,
                                   RT_NULL,
                                   AIRKISS_DOING_TIMER,
                                   RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
    if (!g_airkiss_lock_channel_timer)
    {
        rt_kprintf("Create airkiss doing timeout timer failed \n");
        result = AIRKISS_FAILURE;
        goto init_exit;
    }

    airkiss_decode_over_sema = rt_sem_create("tlink", 0, RT_IPC_FLAG_FIFO);
    if (!airkiss_decode_over_sema)
    {
        rt_kprintf("Create  airkiss config done sem failed! \n");
        result = AIRKISS_FAILURE;
        goto init_exit;
    }

    airkiss_context_ptr = (airkiss_context_t *)rt_malloc(sizeof(airkiss_context_t));
    if (!airkiss_context_ptr)
    {
        rt_kprintf("Malloc memory for airkiss context \n");
        result = AIRKISS_FAILURE;
        goto init_exit;
    }

    rt_memset(airkiss_context_ptr, 0, sizeof(airkiss_context_t));

    result = airkiss_init(airkiss_context_ptr, &airkiss_cfg_ptr);
    if (result != RT_EOK)
    {
        rt_kprintf("Airkiss init failed!!\r\n");
        result = AIRKISS_FAILURE;
        goto init_exit;
    }

    AIRKISS_PRINTF("Airkiss version: %s\r\n", airkiss_version());

    g_wlan_device = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_STA_NAME);
    if (g_wlan_device == RT_NULL)
    {
        rt_kprintf("Device not found\n");
        result = AIRKISS_FAILURE;
        goto init_exit;
    }

init_exit:
    return result;
}

uint32_t airkiss_resource_destroy(void)
{
    AIRKISS_PRINTF("airkiss_resource_destroy\r\n");

    if (airkiss_context_ptr != RT_NULL)
    {
        rt_free(airkiss_context_ptr);
        airkiss_context_ptr = RT_NULL;
    }

    if (airkiss_switch_channel_timer)
    {
        rt_timer_stop(airkiss_switch_channel_timer);
        rt_timer_delete(airkiss_switch_channel_timer);
    }
    if (g_airkiss_lock_channel_timer)
    {
        rt_timer_stop(g_airkiss_lock_channel_timer);
        rt_timer_delete(g_airkiss_lock_channel_timer);
    }

    if (airkiss_decode_over_sema)
    {
        rt_sem_delete(airkiss_decode_over_sema);
        airkiss_decode_over_sema = 0;
    }
}

void airkiss_notify_decode_over(airkiss_result_t *result)
{
    rt_thread_t tid;

    tid = rt_thread_create("air_echo",
                           airkiss_send_notification_thread,
                           (void *)result->random,
                           1536, RT_THREAD_PRIORITY_MAX - 3, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}


void airkiss_decode_complete_handle(void)
{
    int8_t err;
    int8_t tick = 0;
    airkiss_result_t airkiss_result;

    AIRKISS_PRINTF("airkiss_decode_complete_handle\r\n");
    err = airkiss_get_result(airkiss_context_ptr, &airkiss_result);
    if (err == 0)
    {
        AIRKISS_PRINTF("airkiss_get_result() ok!\n");
        AIRKISS_PRINTF(" ssid = %s \n pwd = %s \n, ssid_length = %d \n pwd_length = %d \n, random = 0x%02x\r\n",
                       airkiss_result.ssid,
                       airkiss_result.pwd,
                       airkiss_result.ssid_length,
                       airkiss_result.pwd_length,
                       airkiss_result.random);
    }

    rt_wlan_cfg_monitor(g_wlan_device, WIFI_MONITOR_STOP);
    rt_wlan_set_monitor_callback(g_wlan_device, RT_NULL);

    airkiss_wifi_connect(&airkiss_result);
    do
    {
        tick ++;
        rt_thread_delay(rt_tick_from_millisecond(1000));
        if (tick >= 30)
        {
            rt_kprintf("GET IP Time Out!!! \n");
            return;
        }
    }
    while (!airkiss_get_wifi_status(g_wlan_device->parent.netif));

    airkiss_notify_decode_over(&airkiss_result);
}

static void airkiss_thread_entry(void *parameter)
{
    uint32_t ret;

    /*step 0: init resource of airkiss*/
    ret = airkiss_resource_create();
    if(AIRKISS_FAILURE == ret)
    {
        goto _exit;
    }

    /*step 1: init the parameter of switching channel, and then monitor*/
    g_current_channel = 1;
    rt_wlan_set_channel(g_wlan_device, g_current_channel);
    rt_wlan_set_monitor_callback(g_wlan_device, airkiss_monitor_callback);
    rt_wlan_cfg_monitor(g_wlan_device, WIFI_MONITOR_START);

    rt_timer_start(airkiss_switch_channel_timer);

    /*step 2: waiting for airkiss decode completion, or timeout*/
    if (rt_sem_take(airkiss_decode_over_sema, rt_tick_from_millisecond(1000 * 90)) != RT_EOK)
    {
        AIRKISS_PRINTF("airkiss_thread_timeout\r\n");
		
		mmgmt_no_wifi_connected();
		mmgmt_determine_initial_offline_mode();
		flash_player_enter();
    }

    /*step 3: handle decode phase*/
    if (AIRKISS_STATUS_COMPLETE == airkiss_recv_ret)
    {
        airkiss_decode_complete_handle();
    }

_exit:
    airkiss_clear_start_flag();
    airkiss_resource_destroy();
}

int airkiss(int argc, char *argv[])
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("airkiss",
                           airkiss_thread_entry,
                           RT_NULL,
                           1024 * 4,
                           13,
                           10);

    if (tid != RT_NULL)
        rt_thread_startup(tid);
}

void star_airkiss(void)
{
	int ret;
    char *value;

	ret = -1;
    value = ef_get_env("airkissflag");
    rt_kprintf("airkissflag value = %p \n", value);
    if(value)
    {
        rt_kprintf("##########value = %s \n", value);

        if(1 == atoi(value))
        {
        	mmgmt_try_wifi_connected();
            airkiss_set_start_flag();

            send_play_tip_event(TURING_PLAYTIP_WIFI_CONNECT_TO_NETWORK);
            airkiss(0, NULL);
			
            char airkissflag[3] = "0";
            ef_set_env("airkissflag", airkissflag);
            ef_save_env();

            rt_thread_sleep(5000);
        }
        else
        {
            value = ef_get_env("update");
            rt_kprintf("update value = %p \r\n", value);
            if(value)
            {
                rt_kprintf("update value:%d \r\n", atoi(value));

                if(2 == atoi(value))
                {
                    send_play_tip_event(TURIN_PLAYTIP_UPGRADED);
                    ef_set_env("update", "0");
                    ef_save_env();
                }
                else
                {
                    send_tip_start_system();
                }
            }
            else
            {
                send_tip_start_system();
            }

            ret = wifi_get_ap_info_from_ef();
            send_play_tip_event(TURING_PLAYTIP_WIFI_CONNECT_TO_NETWORK);
        }
    }
    else
    {
		send_tip_start_system();
#if FACTORY_TEST_ENABLE 
    	ret = try_get_factory_ssid();
		if(0 != ret)
		{
	        ret = wifi_get_ap_info_from_ef();
	        rt_kprintf("########## please press network key\n");
		}
#endif

    }

	if(WCMD_FAILURE == ret)
	{
		mmgmt_no_wifi_connected();
		flash_player_enter();
	}
	else if(WCMD_SUCCESS == ret)
	{
		mmgmt_try_wifi_connected();
	}
}

#ifdef FINSH_USING_MSH
#include "finsh.h"

MSH_CMD_EXPORT(airkiss, start_ariksss);
#endif
// eof
