#include "include.h"
#include "arm_arch.h"
#include "target_util_pub.h"
#include "mem_pub.h"
#include "uart_pub.h"
#include "sys_rtos.h"
#include "task.h"
#include "rtos_pub.h"
#include "error.h"
#include "fake_clock_pub.h"
#include "wlan_ui_pub.h"
//#include "socket.h"
#include "airkiss.h"

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <sys/socket.h>
// #include <lwip_netconf.h>

#define MAX_CHANNELS            14
#define AIRKISS_DEBUG           0
#define AIRKISS_SWITCH_TIMER    50     // ms
#define AIRKISS_DOING_TIMER     20000  // 20s
#define MIN_SEL_CHAN_TIMER      1000   // ms
#define MAX_SEL_CHAN_TIMER      5000   // ms
#define AIRKISS_CONNECT_TIMER   60000  // Ms

#define AIRKISS_RECV_BUFSIZE    24   // fctrl + duration + mac1+ mac2 + mac3 + seq
#define MIN_UDP_RANDOM_SEND     20
#define MIN_VALID_DATACNT_INCHAN    4
#define MIN_VALID_BCNCNT_INCHAN     1


#if AIRKISS_DEBUG
#define AIRKISS_PRT             os_printf
#define AIRKISS_WARN            warning_prf
#define AIRKISS_FATAL           fatal_prf
#else
#define AIRKISS_PRT             null_prf
#define AIRKISS_WARN            warning_prf
#define AIRKISS_FATAL           fatal_prf
#endif

typedef enum
{
    AIRKISS_SCAN_ALL_CHAN = 0,
    AIRKISS_SCAN_SELECTED_CHAN
} airkiss_mode;

typedef struct {
    u8 bcn_cnt;
    u8 data_cnt;
    u16 channel;
} chan_param_t;

typedef struct {
    chan_param_t chan[MAX_CHANNELS];
    u8 cur_chan_idx;
    u8 all_chan_nums;
    u8 selected_chan_nums;
    u8 mode;
} airkiss_channel_t;

static airkiss_context_t *ak_contex = NULL;
const airkiss_config_t ak_conf = {
    (airkiss_memset_fn)&os_memset,
    (airkiss_memcpy_fn)&os_memcpy,
    (airkiss_memcmp_fn)&os_memcmp,
    (airkiss_printf_fn)&AIRKISS_PRT
};

beken_timer_t ak_chan_timer;
beken_timer_t ak_doing_timer;
beken_thread_t ak_thread_handle = NULL;
beken_semaphore_t ak_semaphore = NULL;
beken_semaphore_t ak_connect_semaphore = NULL;
airkiss_channel_t g_chans;

volatile u8 airkiss_exit = 0;
int read_size = 0;
u8 *read_buf = NULL;

extern void demo_sta_app_init(char *oob_ssid,char *connect_key);
extern void sta_set_default_netif(void);
extern void reset_default_netif(void);
extern void net_set_sta_ipup_callback(void *fn);

static struct rt_wlan_device *g_wlan;

void airkiss_count_usefull_packet(const unsigned char *frame, int size)
{
    struct mac_hdr *fmac_hdr = (struct mac_hdr *)frame;
    chan_param_t *cur_chan = &g_chans.chan[g_chans.cur_chan_idx];

    if(!frame || !size)
        return;

    if((MAC_FCTRL_BEACON == (fmac_hdr->fctl & MAC_FCTRL_TYPESUBTYPE_MASK)) ||
       (MAC_FCTRL_PROBERSP == (fmac_hdr->fctl & MAC_FCTRL_TYPESUBTYPE_MASK))) {
        cur_chan->bcn_cnt++;
    } else if(MAC_FCTRL_DATA_T == (fmac_hdr->fctl & MAC_FCTRL_TYPE_MASK)){
        cur_chan->data_cnt++;
    }
}

void airkiss_set_scan_all_channel(void)
{
    int i;
    os_memset(&g_chans, 0, sizeof(airkiss_channel_t));

    g_chans.all_chan_nums = MAX_CHANNELS - 1;
    for(i=0; i<g_chans.all_chan_nums; i++)
    {
        g_chans.chan[i].channel = i + 1;
    }

    g_chans.mode = AIRKISS_SCAN_ALL_CHAN;
    g_chans.cur_chan_idx = 0;

    AIRKISS_WARN("change to all scan mode\r\n");
}

u32 airkiss_calc_time_for_selected_channel(u8 valid_cnt)
{
    u32 timer_cnt;

    timer_cnt = (valid_cnt / MIN_VALID_DATACNT_INCHAN) * MIN_SEL_CHAN_TIMER;

    if(timer_cnt < MIN_SEL_CHAN_TIMER)
        return MIN_SEL_CHAN_TIMER;
    else if(timer_cnt > MAX_SEL_CHAN_TIMER)
        return MAX_SEL_CHAN_TIMER;

    return timer_cnt;
}

void airkiss_add_channel(u8 channel, u8 bcn_cnt, u8 data_cnt)
{
    int i;

    if(data_cnt < MIN_VALID_DATACNT_INCHAN)
        return;
    if(bcn_cnt < MIN_VALID_BCNCNT_INCHAN)
        return;

    for(i=0; i<g_chans.selected_chan_nums; i++) {
        u8 cur_cnt = g_chans.chan[i].data_cnt;
        u8 cur_chan = g_chans.chan[i].channel;
        if(cur_cnt < data_cnt) {
            if(cur_chan != channel) {
                u8 move_cnt = g_chans.selected_chan_nums - i;
                os_memmove(&g_chans.chan[i+1], &g_chans.chan[i], move_cnt*sizeof(chan_param_t));
                g_chans.selected_chan_nums += 1;
            }

            g_chans.chan[i].channel = channel;
            g_chans.chan[i].data_cnt = data_cnt;
            return;
        }
    }

    if(i == g_chans.selected_chan_nums) {
        g_chans.selected_chan_nums ++;
        g_chans.chan[i].channel = channel;
        g_chans.chan[i].data_cnt = data_cnt;
    }

}

void airkiss_switch_channel_callback(void *data)
{
    int ret;
    u8 bcn_cnt = 0,  data_cnt = 0;
    u32 timer_cnt = 0;
    u8 channel = 0;
    chan_param_t *cur_chan = &g_chans.chan[g_chans.cur_chan_idx];

    bcn_cnt = cur_chan->bcn_cnt;
    data_cnt = cur_chan->data_cnt;
    channel = cur_chan->channel;
    AIRKISS_PRT("finish scan ch:%02d, bcn:%03d, data:%03d\r\n",
        channel, bcn_cnt, data_cnt);
    AIRKISS_PRT("\r\n");


    switch(g_chans.mode)
    {
        case AIRKISS_SCAN_ALL_CHAN:
            airkiss_add_channel(channel, bcn_cnt, data_cnt);
            //airkiss_clear_channel_valid_data_count();

            g_chans.cur_chan_idx++;
            timer_cnt = AIRKISS_SWITCH_TIMER;

            if(g_chans.cur_chan_idx >= g_chans.all_chan_nums) {
                g_chans.cur_chan_idx = 0;
                if(g_chans.selected_chan_nums) {
                    g_chans.mode = AIRKISS_SCAN_SELECTED_CHAN;
                    data_cnt = g_chans.chan[g_chans.cur_chan_idx].data_cnt;
                    timer_cnt = airkiss_calc_time_for_selected_channel(data_cnt);
                    AIRKISS_WARN("change to selected scan mode\r\n");
                }else{
                    airkiss_set_scan_all_channel();
                }
            }
            break;

        case AIRKISS_SCAN_SELECTED_CHAN:
            g_chans.cur_chan_idx++;
            data_cnt = g_chans.chan[g_chans.cur_chan_idx].data_cnt;
            timer_cnt = airkiss_calc_time_for_selected_channel(data_cnt);

            if(g_chans.cur_chan_idx >= g_chans.selected_chan_nums) {
                g_chans.cur_chan_idx = 0;
                airkiss_set_scan_all_channel();
                g_chans.mode = AIRKISS_SCAN_ALL_CHAN;
                timer_cnt = AIRKISS_SWITCH_TIMER;
            }
            break;

        default:
            AIRKISS_WARN("unknow state:%d\r\n", g_chans.mode);
            g_chans.mode = AIRKISS_SCAN_ALL_CHAN;
            return;
            break;
    }

    channel = g_chans.chan[g_chans.cur_chan_idx].channel;

    AIRKISS_PRT("start scan ch:%02d/%02d, time_intval:%d\r\n", g_chans.cur_chan_idx, channel, timer_cnt);
    rt_wlan_set_channel(g_wlan, channel);
    airkiss_change_channel(ak_contex);

    ret = rtos_change_period(&ak_chan_timer, timer_cnt);
    ASSERT(kNoErr == ret);

}

void airkiss_doing_timeout_callback(void *data)
{
    int ret;
    AIRKISS_WARN("airkiss_doing_timeout, restart channel switch timer\r\n");

    // stop doing timer
    ret = rtos_stop_timer(&ak_doing_timer);
    ASSERT(kNoErr == ret);

    airkiss_change_channel(ak_contex);

    // restart scan process
    airkiss_set_scan_all_channel();
    g_chans.cur_chan_idx = 0;  // set channel 1
    rt_wlan_set_channel(g_wlan, g_chans.chan[g_chans.cur_chan_idx].channel);
    ret = rtos_change_period(&ak_chan_timer, AIRKISS_SWITCH_TIMER);
    ASSERT(kNoErr == ret);
}

void airkiss_monitor_callback(uint8_t*data, int len, rt_wlan_link_info_t *info)
{
    u16 fctl;
    GLOBAL_INT_DECLARATION();

    if(len < AIRKISS_RECV_BUFSIZE)
        return;

    GLOBAL_INT_DISABLE();
    read_size = len;
    read_buf = data;
    GLOBAL_INT_RESTORE();

    if(ak_semaphore)
        rtos_set_semaphore(&ak_semaphore);
}

int process_airkiss(const unsigned char *packet, int size)
{
    int ret, result;
    ret = airkiss_recv(ak_contex, (void *)packet, size);
    if(ret == AIRKISS_STATUS_CONTINUE)
    {
    }
    else if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)
    {
        result = rtos_stop_timer(&ak_chan_timer);
        ASSERT(kNoErr == result);
        AIRKISS_WARN("Lock channel in %d\r\n", g_chans.chan[g_chans.cur_chan_idx].channel);

        AIRKISS_WARN("start airkiss doing timer\r\n");
        result = rtos_start_timer(&ak_doing_timer);
        ASSERT(kNoErr == result);
    }
    else if(ret == AIRKISS_STATUS_COMPLETE)
    {
        result = rtos_stop_timer(&ak_doing_timer);
        ASSERT(kNoErr == result);
    }

    return ret;
}

void airkiss_connected_to_bssid(void)
{
    if(ak_connect_semaphore)
        rtos_set_semaphore(&ak_connect_semaphore);
}

static void airkiss_start_udp_boardcast(uint8_t random_data)
{
    int sock = -1;
    int udpbufsize = 2;
    uint8_t random = random_data;
    struct sockaddr_in g_stUDPBCAddr, g_stUDPBCServerAddr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        AIRKISS_WARN("notify create socket error!\n");
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
        AIRKISS_WARN("notify socket setsockopt error\n");
        goto _exit;
    }

    if (bind(sock, (struct sockaddr *)&g_stUDPBCServerAddr, sizeof(g_stUDPBCServerAddr)) != 0)
    {
        AIRKISS_WARN("notify socket bind error\n");
        goto _exit;
    }

    for (int i = 0; i <= 20; i++)
    {
        int ret = sendto(sock, (char*)&random, 1, 0 ,(struct sockaddr*)&g_stUDPBCAddr, sizeof(g_stUDPBCAddr));
        rt_thread_delay(100);
    }

    AIRKISS_WARN("airkiss notification thread exit!\n");

_exit:
    if(sock >= 0)
    {
        lwip_close(sock);
    }
}

void airkiss_main( void* arg )
{
    int result;
    u32 con_time;
    airkiss_result_t ak_result;
    int airkiss_read_size = 0;
    u8 *airkiss_read_buf = NULL;

    result = rtos_init_timer(&ak_chan_timer,
                            AIRKISS_SWITCH_TIMER,
                            airkiss_switch_channel_callback,
                            (void *)0);
    ASSERT(kNoErr == result);

    result = rtos_init_timer(&ak_doing_timer,
                            AIRKISS_DOING_TIMER,
                            airkiss_doing_timeout_callback,
                            (void *)0);
    ASSERT(kNoErr == result);

    ak_contex = (airkiss_context_t *)os_malloc(sizeof(airkiss_context_t));
    airkiss_read_buf = (u8*)os_malloc(sizeof(u8)*AIRKISS_RECV_BUFSIZE);
    if((!ak_contex) || (!airkiss_read_buf))
    {
        AIRKISS_FATAL("Airkiss no buffer\r\n");
        goto kiss_exit;
    }

    result = airkiss_init(ak_contex, &ak_conf);
    if(result != 0)
    {
        AIRKISS_FATAL("Airkiss init failed!!\r\n");
        goto kiss_exit;
    }

    AIRKISS_WARN("Airkiss version: %s\r\n", airkiss_version());

    // stop monitor mode
    rt_wlan_cfg_monitor(g_wlan, WIFI_MONITOR_STOP);
    rt_wlan_register_monitor(g_wlan, RT_NULL);

    // start monitor
    rt_wlan_register_monitor(g_wlan, airkiss_monitor_callback);
    rt_wlan_cfg_monitor(g_wlan, WIFI_MONITOR_START);

    // start from first channel
    airkiss_set_scan_all_channel();
    g_chans.cur_chan_idx = 0;  // set channel 1

    rt_wlan_set_channel(g_wlan, g_chans.chan[g_chans.cur_chan_idx].channel);

    result = rtos_start_timer(&ak_chan_timer);
    ASSERT(kNoErr == result);

    airkiss_exit = 0;
    ak_result.ssid = NULL;

    for(;;)
    {
        GLOBAL_INT_DECLARATION();

        // check exteral single to exit
        if(airkiss_exit)
            break;

        result = rtos_get_semaphore(&ak_semaphore, BEKEN_WAIT_FOREVER);
        //ASSERT(kNoErr == result);

        GLOBAL_INT_DISABLE();
        os_memcpy(airkiss_read_buf, read_buf, AIRKISS_RECV_BUFSIZE);
        airkiss_read_size = read_size;
        GLOBAL_INT_RESTORE();

        // count received packet
        airkiss_count_usefull_packet(airkiss_read_buf, airkiss_read_size);

        if(g_chans.mode == AIRKISS_SCAN_SELECTED_CHAN)
        {
            if(AIRKISS_STATUS_COMPLETE == process_airkiss(airkiss_read_buf, airkiss_read_size))
            {
                AIRKISS_WARN("Airkiss completed.\r\n");
                airkiss_get_result(ak_contex, &ak_result);

                AIRKISS_WARN("Result:\r\n");
                AIRKISS_WARN("ssid:[%s]\r\n", ak_result.ssid);
                AIRKISS_WARN("ssid_len:[%d]\r\n", ak_result.ssid_length);
                AIRKISS_WARN("ssid_crc:[%x]\r\n", ak_result.reserved);
                AIRKISS_WARN("key:[%s]\r\n", ak_result.pwd);
                AIRKISS_WARN("key_len:[%d]\r\n", ak_result.pwd_length);
                AIRKISS_WARN("random:[0x%02x]\r\n", ak_result.random);
                break;
            }
        }
    }

    // stop monitor mode
    rt_wlan_cfg_monitor(g_wlan, WIFI_MONITOR_STOP);
    rt_wlan_register_monitor(g_wlan, RT_NULL);
    

    if(ak_result.ssid)
    {
        if(ak_connect_semaphore == NULL) {
            result = rtos_init_semaphore(&ak_connect_semaphore, 1);
            ASSERT(kNoErr == result);
        }

        net_set_sta_ipup_callback((void*)airkiss_connected_to_bssid);

        // connect to this bssid
        demo_sta_app_init(ak_result.ssid, ak_result.pwd);

        // wait for connect to bssid
        con_time = AIRKISS_CONNECT_TIMER;
        result = rtos_get_semaphore(&ak_connect_semaphore, con_time);
        if(result == kNoErr) {
            // start udp boardcast
            airkiss_start_udp_boardcast(ak_result.random);
        }else {
            AIRKISS_FATAL("airkiss connect to bssid timeout\r\n");
        }

        net_set_sta_ipup_callback(NULL);

        rtos_deinit_semaphore(&ak_connect_semaphore);
        ak_connect_semaphore = NULL;
    }

kiss_exit:
    AIRKISS_WARN("Airkiss exit.\r\n");

    os_free(ak_contex);
    ak_contex = NULL;
    os_free(airkiss_read_buf);

    result = rtos_deinit_timer(&ak_chan_timer);
    ASSERT(kNoErr == result);
    result = rtos_deinit_timer(&ak_doing_timer);
    ASSERT(kNoErr == result);

    rtos_deinit_semaphore(&ak_semaphore);
    ak_semaphore = NULL;

    ak_thread_handle = NULL;
    rtos_delete_thread(NULL);
}

u32 airkiss_process(u8 start)
{
    int ret;
    GLOBAL_INT_DECLARATION();
    
    AIRKISS_FATAL("airkiss_process:%d\r\n", start);

    g_wlan = (struct rt_wlan_device*)rt_device_find(WIFI_DEVICE_STA_NAME);
    if(!g_wlan)
    {
        rt_kprintf("ERROR: wifi device %s not found \n", WIFI_DEVICE_STA_NAME);
    }

    if(start) 
    {   
        // start airkiss 
        if(ak_semaphore == NULL) {
            ret = rtos_init_semaphore(&ak_semaphore, 1);
            ASSERT(kNoErr == ret);
        }

        if(ak_thread_handle == NULL) {
            ret = rtos_create_thread(&ak_thread_handle,
                                      BEKEN_DEFAULT_WORKER_PRIORITY,
                                      "airkiss",
                                      (beken_thread_function_t)airkiss_main,
                                      2048,
                                      (beken_thread_arg_t)0);
            if (ret != kNoErr)
            {
                AIRKISS_FATAL("Error: airkiss_start_process: %d\r\n", ret);
                goto init_err;
            }
        }
    }
    else 
    {
        // stop airkiss     
        if(ak_thread_handle && ak_semaphore) 
        {
            GLOBAL_INT_DISABLE();
            airkiss_exit = 1;
            GLOBAL_INT_RESTORE();
        }

    }

    return kNoErr;

init_err:
    return kGeneralErr;

}
