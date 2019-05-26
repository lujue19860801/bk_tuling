#include "include.h"
#include "wlan_ui_pub.h"
#include "rw_pub.h"
#include "vif_mgmt.h"
#include "sa_station.h"
#include "param_config.h"
#include "common/ieee802_11_defs.h"
#include "driver_beken.h"
#include "mac_ie.h"
#include "sa_ap.h"
#include "main_none.h"
#include "sm.h"
#include "mac.h"
#include "sm_task.h"
#include "scan_task.h"
#include "hal_machw.h"
#include "error.h"
#include "lwip_netif_address.h"
#include "lwip/inet.h"
#include <stdbool.h>
#include "rw_pub.h"
#include "power_save_pub.h"
#include "rwnx.h"
#include "net.h"
#include "mm_bcn.h"
#include "phy_trident.h"
#include "mcu_ps_pub.h"
#include "manual_ps_pub.h"
#include "gpio_pub.h"

#if CFG_ROLE_LAUNCH
#include "role_launch.h"
#endif

monitor_cb_t g_monitor_cb = 0;
int g_set_channel_postpone_num = 0; 
static msg_sta_states msg_sta_old_state = 0;

extern void mm_hw_ap_disable(void);
extern int hostapd_main_exit(void);
extern int supplicant_main_exit(void);
extern void net_wlan_add_netif(void *mac);
extern void wpa_hostapd_release_scan_rst(void);
extern int mm_bcn_get_tx_cfm(void);

static void rwnx_remove_added_interface(void)
{
#define TEST_MAC_ADDR "\xC8\x93\x48\x22\x22\x10"

    int ret;
    struct mm_add_if_cfm *cfm;
    u8 test_mac[6] = TEST_MAC_ADDR;

    cfm = (struct mm_add_if_cfm *)os_malloc(sizeof(struct mm_add_if_cfm));
    ret = rw_msg_send_add_if((const unsigned char *)&test_mac, 3, 0, cfm);

    if(ret || (cfm->status != CO_OK))
    {
        os_printf("[saap]MM_ADD_IF_REQ failed!\r\n");
        return;
    }

    rw_msg_send_apm_start_req(cfm->inst_nbr, 1, NULL);
    rw_msg_send_remove_if(cfm->inst_nbr);

    if(cfm)
    {
        os_free(cfm);
    }
}

void bk_wlan_connection_loss(void)
{
    struct vif_info_tag *p_vif_entry = vif_mgmt_first_used();

    while (p_vif_entry != NULL)
    {
        if(p_vif_entry->type == VIF_STA)
        {
            os_printf("bk_wlan_connection_loss vif:%d\r\n", p_vif_entry->index);
            sta_ip_down();
            rw_msg_send_connection_loss_ind(p_vif_entry->index);
        }
        p_vif_entry = vif_mgmt_next(p_vif_entry);
    }
}

uint32_t bk_sta_cipher_is_open(void)
{
    ASSERT(g_sta_param_ptr);
    return (SECURITY_TYPE_NONE == g_sta_param_ptr->cipher_suite);
}

uint32_t bk_sta_cipher_is_wep(void)
{
    ASSERT(g_sta_param_ptr);
    return (SECURITY_TYPE_WEP == g_sta_param_ptr->cipher_suite);
}

int bk_sta_cipher_type(void)
{
    if(!sta_ip_is_start()) 
    {
        return -1;
    }
    
    return g_sta_param_ptr->cipher_suite;
}


uint32_t bk_wlan_ap_get_frequency(void)
{
    uint8_t channel = bk_wlan_ap_get_channel_config();
    
    return rw_ieee80211_get_centre_frequency(channel);
}

uint8_t bk_wlan_ap_get_channel_config(void)
{
    return g_ap_param_ptr->chann;
}

void bk_wlan_ap_set_channel_config(uint8_t channel)
{
    g_ap_param_ptr->chann = channel;
}

uint8_t bk_wlan_has_role(uint8_t role)
{
    struct netif *lwip_if;
    VIF_INF_PTR vif_entry;
    uint32_t role_count = 0;

    vif_entry = (VIF_INF_PTR)rwm_mgmt_is_vif_first_used();
    while(vif_entry)
    {
        lwip_if = (struct netif *)vif_entry->priv;
        if(vif_entry->type == role)
        {
            role_count ++ ;
        }

        vif_entry = (VIF_INF_PTR)rwm_mgmt_next(vif_entry);
    }

    return role_count;
}

void bk_wlan_set_coexist_at_init_phase(uint8_t current_role)
{
    uint32_t coexit = 0;
    
    switch(current_role)
    {
        case CONFIG_ROLE_AP:
            if(bk_wlan_has_role(VIF_STA))
            {
                coexit = 1;
            }
            break;
            
        case CONFIG_ROLE_STA:
            if(bk_wlan_has_role(VIF_AP))
            {
                coexit = 1;
            }
            break;
            
        case CONFIG_ROLE_NULL:
            if(bk_wlan_has_role(VIF_STA)
                && bk_wlan_has_role(VIF_AP))
            {
                coexit = 1;
            }
            break;
            
        case CONFIG_ROLE_COEXIST:
            coexit = 1;
            ASSERT(CONFIG_ROLE_COEXIST == g_wlan_general_param->role);
            break;
            
        default:
            break;
    }

    if(coexit)
    {
        g_wlan_general_param->role = CONFIG_ROLE_COEXIST;
    }
}

uint16_t bk_wlan_sta_get_frequency(void)
{  
    uint16_t frequency = 0;
    uint32_t sta_flag = 0;
    VIF_INF_PTR vif_entry;
    
    vif_entry = (VIF_INF_PTR)rwm_mgmt_is_vif_first_used();
    while(vif_entry)
    {
        if(vif_entry->type == VIF_STA)
        {
            sta_flag = 1;
            break;
        }

        vif_entry = (VIF_INF_PTR)rwm_mgmt_next(vif_entry);
    }

    if(0 == sta_flag)
    {
        goto get_exit;
    }

    frequency = chan_get_vif_frequency(vif_entry);
    
get_exit:    
    return frequency;
}

uint8_t bk_wlan_sta_get_channel(void)
{  
    uint8_t channel = 0;
    uint16_t frequency;

    frequency = bk_wlan_sta_get_frequency();
    if(frequency)
    {
        channel = rw_ieee80211_get_chan_id(frequency);
    }
    
    return channel;
}

uint8_t bk_wlan_ap_get_default_channel(void)
{
    uint8_t channel;

    /* if ap and sta are coexist, ap channel shall match sta channel firstly*/
    channel = bk_wlan_sta_get_channel();
    if(0 == channel)
    {
        channel = DEFAULT_CHANNEL_AP;
    }

    return channel;
}

void bk_wlan_ap_csa_coexist_mode(void *arg, uint8_t dummy)
{
    int ret;
    uint16_t frequency;
    
    if(0 == bk_wlan_has_role(VIF_AP))
    {        
        return;
    }

    frequency = bk_wlan_sta_get_frequency();
    if(frequency)
    {
        os_printf("\r\nhostapd_channel_switch\r\n");
#if CFG_ROLE_LAUNCH
        if(!fl_get_pre_sta_cancel_status())
#endif
        {
        	ret = hostapd_channel_switch(frequency);
        }
        if(ret)
        {
            os_printf("csa_failed:%x\r\n", ret);
        }
    }
}

void bk_wlan_reg_csa_cb_coexist_mode(void)
{
    /* the callback routine will be invoked at the timepoint of associating at station mode*/
    mhdr_connect_user_cb(bk_wlan_ap_csa_coexist_mode, 0);
}

void bk_wlan_ap_init(network_InitTypeDef_st *inNetworkInitPara)
{
    os_printf("Soft_AP_start\r\n");

    if(!g_ap_param_ptr)
    {
        g_ap_param_ptr = (ap_param_t *)os_zalloc(sizeof(ap_param_t));
        ASSERT(g_ap_param_ptr);
    }

    os_memset(g_ap_param_ptr, 0x00, sizeof(*g_ap_param_ptr));

    if(MAC_ADDR_NULL((u8 *)&g_ap_param_ptr->bssid))
    {
        wifi_get_mac_address((u8 *)&g_ap_param_ptr->bssid, CONFIG_ROLE_AP);
    }

    bk_wlan_ap_set_channel_config(bk_wlan_ap_get_default_channel());

    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_zalloc(sizeof(general_param_t));
        ASSERT(g_wlan_general_param);
    }
    g_wlan_general_param->role = CONFIG_ROLE_AP;
    bk_wlan_set_coexist_at_init_phase(CONFIG_ROLE_AP);

    if(inNetworkInitPara)
    {
        g_ap_param_ptr->ssid.length = MIN(SSID_MAX_LEN, os_strlen(inNetworkInitPara->wifi_ssid));
        os_memcpy(g_ap_param_ptr->ssid.array, inNetworkInitPara->wifi_ssid, g_ap_param_ptr->ssid.length);
        g_ap_param_ptr->key_len = os_strlen(inNetworkInitPara->wifi_key);
        if(g_ap_param_ptr->key_len < 8)
        {
            g_ap_param_ptr->cipher_suite = SECURITY_TYPE_NONE;
        }
        else
        {
            g_ap_param_ptr->cipher_suite = SECURITY_TYPE_WPA2_AES;
            os_memcpy(g_ap_param_ptr->key, inNetworkInitPara->wifi_key, g_ap_param_ptr->key_len);
        }

        if(inNetworkInitPara->dhcpMode == DHCP_Server)
        {
            g_wlan_general_param->dhcp_enable = 1;
        }
        else
        {
            g_wlan_general_param->dhcp_enable = 0;
        }
        inet_aton(inNetworkInitPara->local_ip_addr, &(g_wlan_general_param->ip_addr));
        inet_aton(inNetworkInitPara->net_mask, &(g_wlan_general_param->ip_mask));
        inet_aton(inNetworkInitPara->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
			
#if CFG_ROLE_LAUNCH
		if(rl_pre_ap_set_status(RL_STATUS_AP_INITING))
		{
			return;
		}
#endif
    }
	
    sa_ap_init();
}

OSStatus bk_wlan_start_ap(network_InitTypeDef_st *inNetworkInitParaAP)
{
    int ret, flag ,empty;
    GLOBAL_INT_DECLARATION();

	while( 1 )
	{
		GLOBAL_INT_DISABLE();
		flag = mm_bcn_get_tx_cfm();
        empty = is_apm_bss_config_empty();
		if ( flag == 0 && empty == 1)
		{
			GLOBAL_INT_RESTORE();
			break;
		}
		else
		{
			GLOBAL_INT_RESTORE();
			rtos_delay_milliseconds(100);
		}
	}
    
	bk_wlan_stop(Soft_AP);
    
    bk_wlan_ap_init(inNetworkInitParaAP);
    
    ret = hostapd_main_entry(2, 0);
    if(ret)
    {
        os_printf("bk_wlan_start softap failed!!\r\n");
        return -1;
    }

    net_wlan_add_netif(&g_ap_param_ptr->bssid);
    
    ip_address_set(Soft_AP, 
                   inNetworkInitParaAP->dhcpMode,
                   inNetworkInitParaAP->local_ip_addr,
                   inNetworkInitParaAP->net_mask,
                   inNetworkInitParaAP->gateway_ip_addr,
                   inNetworkInitParaAP->dnsServer_ip_addr);
    uap_ip_start();

    sm_build_broadcast_deauthenticate();

    return kNoErr;
}

void bk_wlan_sta_init(network_InitTypeDef_st *inNetworkInitPara)
{
    if(!g_sta_param_ptr)
    {
        g_sta_param_ptr = (sta_param_t *)os_zalloc(sizeof(sta_param_t));
        ASSERT(g_sta_param_ptr);
    }

    wifi_get_mac_address((u8 *)&g_sta_param_ptr->own_mac, CONFIG_ROLE_STA);
    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_zalloc(sizeof(general_param_t));
        ASSERT(g_wlan_general_param);
    }
    g_wlan_general_param->role = CONFIG_ROLE_STA;
    bk_wlan_set_coexist_at_init_phase(CONFIG_ROLE_STA);

    if(inNetworkInitPara)
    {
        g_sta_param_ptr->ssid.length = MIN(SSID_MAX_LEN, os_strlen(inNetworkInitPara->wifi_ssid));
        os_memcpy(g_sta_param_ptr->ssid.array,
                  inNetworkInitPara->wifi_ssid,
                  g_sta_param_ptr->ssid.length);


        g_sta_param_ptr->key_len = os_strlen(inNetworkInitPara->wifi_key);
        os_memcpy(g_sta_param_ptr->key, inNetworkInitPara->wifi_key, g_sta_param_ptr->key_len);

        if(inNetworkInitPara->dhcpMode == DHCP_Client)
        {
            g_wlan_general_param->dhcp_enable = 1;
        }
        else
        {
            g_wlan_general_param->dhcp_enable = 0;
            inet_aton(inNetworkInitPara->local_ip_addr, &(g_wlan_general_param->ip_addr));
            inet_aton(inNetworkInitPara->net_mask, &(g_wlan_general_param->ip_mask));
            inet_aton(inNetworkInitPara->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
        }
		
#if CFG_ROLE_LAUNCH
	    if(rl_pre_sta_set_status(RL_STATUS_STA_INITING))
	    {
	        return;
	    }
#endif
    }
#if CFG_USE_STA_PS
    if(msg_sta_old_state != MSG_GOT_IP)
#endif  	
    mhdr_set_station_status(MSG_CONNECTING);

    bk_wlan_reg_csa_cb_coexist_mode();
    sa_station_init();
}

OSStatus bk_wlan_start_sta(network_InitTypeDef_st *inNetworkInitPara)
{
    bk_wlan_stop(Station);
    
    bk_wlan_sta_init(inNetworkInitPara);
    
    supplicant_main_entry(inNetworkInitPara->wifi_ssid);
    
    net_wlan_add_netif(&g_sta_param_ptr->own_mac);
    
    ip_address_set(inNetworkInitPara->wifi_mode,
                   inNetworkInitPara->dhcpMode,
                   inNetworkInitPara->local_ip_addr,
                   inNetworkInitPara->net_mask,
                   inNetworkInitPara->gateway_ip_addr,
                   inNetworkInitPara->dnsServer_ip_addr);
    
    return kNoErr;
}

OSStatus bk_wlan_start(network_InitTypeDef_st *inNetworkInitPara)
{
    int ret = 0;
#if CFG_ROLE_LAUNCH
    LAUNCH_REQ lreq;
#endif
    
    if(bk_wlan_is_monitor_mode()) 
    {
        os_printf("monitor (ie.airkiss) is not finish yet, stop it or waiting it finish!\r\n");
        return ret;
    }

    if(inNetworkInitPara->wifi_mode == Soft_AP)
    {
        #if CFG_ROLE_LAUNCH
        lreq.req_type = LAUNCH_REQ_AP;
        lreq.descr = *inNetworkInitPara;
        
        rl_ap_request_enter(&lreq, 0);
        #else
        bk_wlan_start_ap(inNetworkInitPara);
        #endif
    }
    else if(inNetworkInitPara->wifi_mode == Station)
    {
        #if CFG_ROLE_LAUNCH
        lreq.req_type = LAUNCH_REQ_STA;
        lreq.descr = *inNetworkInitPara;
        
        rl_sta_request_enter(&lreq, 0);
        #else
        bk_wlan_start_sta(inNetworkInitPara);
        #endif
    }

    return 0;
}

void bk_wlan_start_scan(void)
{
    SCAN_PARAM_T scan_param = {0};
	
#if CFG_USE_STA_PS    
    msg_sta_old_state = mhdr_get_station_status();
    bk_wlan_dtim_rf_ps_mode_disable();
#endif

    if(bk_wlan_is_monitor_mode()) 
    {
        os_printf("monitor (ie.airkiss) is not finish yet, stop it or waiting it finish!\r\n");
        return;
    }

    bk_wlan_sta_init(0);

    os_memset(&scan_param.bssid, 0xff, ETH_ALEN);

    rw_msg_send_scanu_req(&scan_param);
}

void bk_wlan_scan_ap_reg_cb(FUNC_2PARAM_PTR ind_cb)
{
    mhdr_scanu_reg_cb(ind_cb,0);
}

unsigned char bk_wlan_get_scan_ap_result_numbers(void)
{
    struct scanu_rst_upload *scan_rst;
    unsigned char scanu_num = 0;

    scan_rst = sr_get_scan_results();
    if(scan_rst)
    {
        scanu_num = scan_rst->scanu_num;
    }

    return scanu_num;
}

OSStatus bk_wlan_ap_is_up(void)
{
	#if CFG_ROLE_LAUNCH
		if(RL_STATUS_AP_INITING < rl_pre_ap_get_status())
		{
			return 1;
		}
	#endif
	
	return 0;
}

OSStatus bk_wlan_sta_is_connected(void)
{
	#if CFG_ROLE_LAUNCH
		if(RL_STATUS_STA_LAUNCHED <= rl_pre_sta_get_status())
		{
			return 1;
		}
	#endif
	
	return 0;
}

void bk_wlan_get_scan_ap_result(SCAN_RST_ITEM_PTR scan_rst_table,unsigned char get_scanu_num)
{
    struct scanu_rst_upload *scan_rst;
    unsigned char scanu_num = 0,i;
    
    scan_rst = sr_get_scan_results();
    if(scan_rst)
    {
        scanu_num = (scan_rst->scanu_num) > get_scanu_num ? (get_scanu_num):(scan_rst->scanu_num);
        
        for(i = 0;i<scanu_num;i++)
        {
            scan_rst_table[i] = *(scan_rst->res[i]);
        }
    }   

    sr_release_scan_results(scan_rst);
}


void bk_wlan_start_assign_scan(UINT8 **ssid_ary, UINT8 ssid_num)
{
    SCAN_PARAM_T scan_param = {0};

#if CFG_USE_STA_PS    
    msg_sta_old_state = mhdr_get_station_status();

#endif

    bk_wlan_sta_init(0);

    os_memset(&scan_param.bssid, 0xff, ETH_ALEN);
    scan_param.num_ssids = ssid_num;
    for (int i = 0 ; i < ssid_num ; i++ )
    {
        scan_param.ssids[i].length = MIN(SSID_MAX_LEN, os_strlen(ssid_ary[i]));
        os_memcpy(scan_param.ssids[i].array, ssid_ary[i], scan_param.ssids[i].length);
    }
    rw_msg_send_scanu_req(&scan_param);
}

void bk_wlan_sta_init_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv)
{
    if(!g_sta_param_ptr)
    {
        g_sta_param_ptr = (sta_param_t *)os_malloc(sizeof(sta_param_t));
        ASSERT(g_sta_param_ptr);
    }

    if(MAC_ADDR_NULL((u8 *)&g_sta_param_ptr->own_mac))
    {
        wifi_get_mac_address((char *)&g_sta_param_ptr->own_mac, CONFIG_ROLE_STA);
    }

    g_sta_param_ptr->ssid.length = MIN(SSID_MAX_LEN, os_strlen(inNetworkInitParaAdv->ap_info.ssid));
    os_memcpy(g_sta_param_ptr->ssid.array, inNetworkInitParaAdv->ap_info.ssid, g_sta_param_ptr->ssid.length);

	g_sta_param_ptr->cipher_suite = inNetworkInitParaAdv->ap_info.security;
    g_sta_param_ptr->fast_connect_set = 1;
    g_sta_param_ptr->fast_connect.chann = inNetworkInitParaAdv->ap_info.channel;
    os_memcpy(g_sta_param_ptr->fast_connect.bssid, inNetworkInitParaAdv->ap_info.bssid, ETH_ALEN);
    g_sta_param_ptr->key_len = inNetworkInitParaAdv->key_len;
    os_memcpy((uint8_t *)g_sta_param_ptr->key, inNetworkInitParaAdv->key, inNetworkInitParaAdv->key_len);

    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_malloc(sizeof(general_param_t));
    }
    g_wlan_general_param->role = CONFIG_ROLE_STA;
    bk_wlan_set_coexist_at_init_phase(CONFIG_ROLE_STA);

    if(inNetworkInitParaAdv->dhcpMode == DHCP_Client)
    {
        g_wlan_general_param->dhcp_enable = 1;
    }
    else
    {
        g_wlan_general_param->dhcp_enable = 0;
        inet_aton(inNetworkInitParaAdv->local_ip_addr, &(g_wlan_general_param->ip_addr));
        inet_aton(inNetworkInitParaAdv->net_mask, &(g_wlan_general_param->ip_mask));
        inet_aton(inNetworkInitParaAdv->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
    }
    
    bk_wlan_reg_csa_cb_coexist_mode();
    sa_station_init();
}

void bk_wlan_ap_init_adv(network_InitTypeDef_ap_st *inNetworkInitParaAP)
{
    if(!g_ap_param_ptr)
    {
        g_ap_param_ptr = (ap_param_t *)os_zalloc(sizeof(ap_param_t));
        ASSERT(g_ap_param_ptr);
    }

    if(MAC_ADDR_NULL((u8 *)&g_ap_param_ptr->bssid))
    {
        wifi_get_mac_address((u8 *)&g_ap_param_ptr->bssid, CONFIG_ROLE_AP);
    }

    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_zalloc(sizeof(general_param_t));
        ASSERT(g_wlan_general_param);
    }
    g_wlan_general_param->role = CONFIG_ROLE_AP;
    bk_wlan_set_coexist_at_init_phase(CONFIG_ROLE_AP);

    if(inNetworkInitParaAP)
    {
        if(0 == inNetworkInitParaAP->channel)
        {
            g_ap_param_ptr->chann = bk_wlan_ap_get_default_channel();
        }
        else
        {
            g_ap_param_ptr->chann = inNetworkInitParaAP->channel;
        }
        
        g_ap_param_ptr->ssid.length = MIN(SSID_MAX_LEN, os_strlen(inNetworkInitParaAP->wifi_ssid));
        os_memcpy(g_ap_param_ptr->ssid.array, inNetworkInitParaAP->wifi_ssid, g_ap_param_ptr->ssid.length);
        g_ap_param_ptr->key_len = os_strlen(inNetworkInitParaAP->wifi_key);
        os_memcpy(g_ap_param_ptr->key, inNetworkInitParaAP->wifi_key, g_ap_param_ptr->key_len);

        g_ap_param_ptr->cipher_suite = inNetworkInitParaAP->security;        

        if(inNetworkInitParaAP->dhcpMode == DHCP_Server)
        {
            g_wlan_general_param->dhcp_enable = 1;
        }
        else
        {
            g_wlan_general_param->dhcp_enable = 0;
        }
        inet_aton(inNetworkInitParaAP->local_ip_addr, &(g_wlan_general_param->ip_addr));
        inet_aton(inNetworkInitParaAP->net_mask, &(g_wlan_general_param->ip_mask));
        inet_aton(inNetworkInitParaAP->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
    }

    sa_ap_init();
}

OSStatus bk_wlan_start_sta_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv)
{
    if(bk_wlan_is_monitor_mode()) 
    {
        os_printf("airkiss is not finish yet, stop airkiss or waiting it finish!\r\n");
        return 0;
    }

    bk_wlan_stop(Station);

#if CFG_ROLE_LAUNCH
    if(rl_pre_sta_set_status(RL_STATUS_STA_INITING))
    {
        return 0;
    }
#endif
    
    bk_wlan_sta_init_adv(inNetworkInitParaAdv);

    supplicant_main_entry(inNetworkInitParaAdv->ap_info.ssid);

    net_wlan_add_netif(&g_sta_param_ptr->own_mac);
    ip_address_set(Station, inNetworkInitParaAdv->dhcpMode,
                   inNetworkInitParaAdv->local_ip_addr,
                   inNetworkInitParaAdv->net_mask,
                   inNetworkInitParaAdv->gateway_ip_addr,
                   inNetworkInitParaAdv->dnsServer_ip_addr);

    return 0;
}

OSStatus bk_wlan_start_ap_adv(network_InitTypeDef_ap_st *inNetworkInitParaAP)
{
    int ret = 0;

    if(bk_wlan_is_monitor_mode()) 
    {
        os_printf("monitor (ie.airkiss) is not finish yet, stop it or waiting it finish!\r\n");
        return ret;
    }

    bk_wlan_stop(Soft_AP);

#if CFG_ROLE_LAUNCH
    if(rl_pre_ap_set_status(RL_STATUS_AP_INITING))
    {
        return 0;
    }
#endif
    
    bk_wlan_ap_init_adv(inNetworkInitParaAP);

    ret = hostapd_main_entry(2, 0);
    if(ret)
    {
        os_printf("bk_wlan_start softap failed!!\r\n");
        return -1;
    }

    net_wlan_add_netif(&g_ap_param_ptr->bssid);

    ip_address_set(Soft_AP, inNetworkInitParaAP->dhcpMode,
                   inNetworkInitParaAP->local_ip_addr,
                   inNetworkInitParaAP->net_mask,
                   inNetworkInitParaAP->gateway_ip_addr,
                   inNetworkInitParaAP->dnsServer_ip_addr);
    uap_ip_start();

    sm_build_broadcast_deauthenticate();

    return kNoErr;
}

int bk_wlan_stop(char mode)
{
    int ret = kNoErr;
#if CFG_USE_STA_PS
    bk_wlan_dtim_rf_ps_mode_disable();
#endif

    switch(mode)
    {
    case Soft_AP:
        mm_hw_ap_disable();

        uap_ip_down();
        net_wlan_remove_netif(&g_ap_param_ptr->bssid);
        hostapd_main_exit();
        if(bk_wlan_has_role(VIF_STA))
        {
            g_wlan_general_param->role = CONFIG_ROLE_STA;
        }    
		
#if CFG_ROLE_LAUNCH
        rl_pre_ap_set_status(RL_STATUS_AP_LAUNCHED);
#endif
        break;

    case Station:
        sta_ip_down();
        net_wlan_remove_netif(&g_sta_param_ptr->own_mac);
        supplicant_main_exit();
        wpa_hostapd_release_scan_rst();
        if(bk_wlan_has_role(VIF_AP))
        {
            g_wlan_general_param->role = CONFIG_ROLE_AP;
        }   
		
#if CFG_ROLE_LAUNCH
        rl_pre_sta_set_status(RL_STATUS_STA_LAUNCHED);
#endif
        
        break;

    default:
        ret = kGeneralErr;
        break;
    }

    return ret;
}

OSStatus bk_wlan_set_ip_status(IPStatusTypedef *inNetpara, WiFi_Interface inInterface)
{
    OSStatus ret = kNoErr;

    switch ( inInterface )
    {
    case Soft_AP :
        if ( uap_ip_is_start() )
        {
            uap_ip_down();
        }
        else
        {
            ret = kGeneralErr;
        }
        break;

    case Station :
        if ( sta_ip_is_start() )
        {
            sta_ip_down();
        }
        else
        {
            ret = kGeneralErr;
        }
        break;

    default:
        ret = kGeneralErr;
        break;
    }

    if ( ret == kNoErr )
    {
        ip_address_set(inInterface, inNetpara->dhcp, inNetpara->ip,
                       inNetpara->mask, inNetpara->gate, inNetpara->dns);
        if ( inInterface == Soft_AP )
        {
            uap_ip_is_start();
        }
        else if ( inInterface == Station )
        {
            sta_ip_start();
        }
    }

    return ret;
}

OSStatus bk_wlan_get_ip_status(IPStatusTypedef *outNetpara, WiFi_Interface inInterface)
{
    OSStatus ret = kNoErr;
    struct wlan_ip_config addr;

    os_memset(&addr, 0, sizeof(struct wlan_ip_config));    
    
    switch ( inInterface )
    {
    case Soft_AP :
        net_get_if_addr(&addr, net_get_uap_handle());
        net_get_if_macaddr(outNetpara->mac, net_get_uap_handle());
        break;

    case Station :
        net_get_if_addr(&addr, net_get_sta_handle());
        net_get_if_macaddr(outNetpara->mac, net_get_sta_handle());
        break;

    default:
        ret = kGeneralErr;
        break;
    }

    if ( ret == kNoErr )
    {
        outNetpara->dhcp = addr.ipv4.addr_type;
        os_strcpy(outNetpara->ip, inet_ntoa(addr.ipv4.address));
        os_strcpy(outNetpara->mask, inet_ntoa(addr.ipv4.netmask));
        os_strcpy(outNetpara->gate, inet_ntoa(addr.ipv4.gw));
        os_strcpy(outNetpara->dns, inet_ntoa(addr.ipv4.dns1));
    }

    return ret;
}

OSStatus bk_wlan_get_link_status(LinkStatusTypeDef *outStatus)
{
    struct sm_get_bss_info_cfm *cfm;
    int ret;
    u8 vif_idx = 0, ssid_len;

    if( !sta_ip_is_start() )
    {
        return kGeneralErr;
    }
    
    outStatus->conn_state = mhdr_get_station_status();

    cfm = os_malloc(sizeof(struct sm_get_bss_info_cfm));
    if(!cfm)
    {
        return kNoMemoryErr;
    }

    vif_idx = rwm_mgmt_vif_mac2idx((void *)&g_sta_param_ptr->own_mac);
    ret = rw_msg_get_bss_info(vif_idx, (void *)cfm);
    if(ret)
    {
        os_free(cfm);
        return kGeneralErr;
    }
    
    outStatus->wifi_strength = cfm->rssi;
    outStatus->channel = rw_ieee80211_get_chan_id(cfm->freq);
    os_memcpy(outStatus->bssid, cfm->bssid, 6);
    ssid_len = MIN(SSID_MAX_LEN, os_strlen(cfm->ssid));
    os_memcpy(outStatus->ssid, cfm->ssid, ssid_len);

    os_free(cfm);

    return kNoErr;
}

void bk_wlan_ap_para_info_get(network_InitTypeDef_ap_st *ap_info)
{
    IPStatusTypedef ap_ips;
    
    if((!ap_info)||(!uap_ip_is_start()))
    {
        return;
    }
    
    memcpy(ap_info->wifi_ssid,g_ap_param_ptr->ssid.array,g_ap_param_ptr->ssid.length);
    memcpy(ap_info->wifi_key,g_ap_param_ptr->key,g_ap_param_ptr->key_len);
    ap_info->channel = g_ap_param_ptr->chann;
    ap_info->security = g_ap_param_ptr->cipher_suite;
     
    bk_wlan_get_ip_status(&ap_ips,Soft_AP);
    memcpy(ap_info->local_ip_addr,ap_ips.ip,16);
    memcpy(ap_info->gateway_ip_addr,ap_ips.gate,16);
    memcpy(ap_info->net_mask,ap_ips.mask,16);
    memcpy(ap_info->dnsServer_ip_addr,ap_ips.dns,16);

     ap_info->dhcpMode = g_wlan_general_param->dhcp_enable;
}


uint32_t bk_wlan_get_INT_status(void)
{
    return *((volatile UINT32 *)(ICU_INT_STATUS));
}

/** @brief  Add the packet type which monitor should receive
 *
 *  @detail This function can be called many times to receive different wifi packets.
 */
int bk_wlan_monitor_rx_type(int type)
{
    unsigned int filter = 0;
    switch(type)
    {
    case WLAN_RX_BEACON:
        nxmac_accept_beacon_setf(1);
        break;
    case WLAN_RX_PROBE_REQ:
        nxmac_accept_probe_req_setf(1);
        break;
    case WLAN_RX_PROBE_RES:
        nxmac_accept_probe_resp_setf(1);
        break;
    case WLAN_RX_ACTION:
        break;
    case WLAN_RX_MANAGEMENT:
        nxmac_accept_other_mgmt_frames_setf(1);
        break;
    case WLAN_RX_DATA:
        nxmac_accept_other_data_frames_setf(1);
        nxmac_accept_qo_s_null_setf(1);
        nxmac_accept_qcfwo_data_setf(1);
        nxmac_accept_q_data_setf(1);
        nxmac_accept_cfwo_data_setf(1);
        nxmac_accept_data_setf(1);
        break;
    case WLAN_RX_MCAST_DATA:
        nxmac_accept_multicast_setf(1);
        nxmac_accept_broadcast_setf(1);
        break;
    case WLAN_RX_ALL:
        nxmac_rx_cntrl_set((uint32_t)0x7FFFFFFC);
        break;
    }

    return 0;
}

void bk_wlan_enable_lsig(void)
{
    hal_machw_allow_rx_rts_cts();
    phy_enable_lsig_intr();
}

void bk_wlan_disable_lsig(void)
{
    hal_machw_disallow_rx_rts_cts();
    phy_disable_lsig_intr();
}

/** @brief  Start wifi monitor mode
 *
 *  @detail This function disconnect wifi station and softAP.
 *
 */
int bk_wlan_start_monitor(void)
{
    monitor_cb_t cb_bakup = g_monitor_cb;
    g_monitor_cb = NULL;

    bk_wlan_stop(Soft_AP);
    bk_wlan_stop(Station);

    g_monitor_cb = cb_bakup;

    bk_wlan_ap_init(0);
    rwnx_remove_added_interface();
    g_wlan_general_param->role = CONFIG_ROLE_NULL;

    return 0;
}

/** @brief  Stop wifi monitor mode
 *
 */
int bk_wlan_stop_monitor(void)
{
    if(g_monitor_cb)
    {
        g_monitor_cb = 0;
        hal_machw_exit_monitor_mode();
    }

    return 0;
}

int bk_wlan_get_channel(void)
{
    int channel, freq;

    freq = rw_msg_get_channel(NULL);
    channel = rw_ieee80211_get_chan_id(freq);

    return channel;
}

/** @brief  Set the monitor channel
 *
 *  @detail This function change the monitor channel (from 1~13).
 *       it can change the channel dynamically, don't need restart monitor mode.
 */
int bk_wlan_set_channel_sync(int channel)
{
    rwnxl_reset_evt(0);
    rw_msg_set_channel(channel, NULL);
    
    return 0;
}

/** @brief  Set channel at the asynchronous method
 *
 *  @detail This function change the monitor channel (from 1~13).
 *       it can change the channel dynamically, don't need restart monitor mode.
 */
int bk_wlan_set_channel(int channel)
{
	GLOBAL_INT_DECLARATION();
	
    if(0 == channel)
    {
        channel = 1;
    }
	
	GLOBAL_INT_DISABLE();
	g_set_channel_postpone_num = channel;
	GLOBAL_INT_RESTORE();    

    CHECK_OPERATE_RF_REG_IF_IN_SLEEP();
	ke_evt_set(KE_EVT_RESET_BIT);
    CHECK_OPERATE_RF_REG_IF_IN_SLEEP_END();
    
    return 0;
}

/** @brief  Register the monitor callback function
 *        Once received a 802.11 packet call the registered function to return the packet.
 */
void bk_wlan_register_monitor_cb(monitor_cb_t fn)
{
    g_monitor_cb = fn;
}

monitor_cb_t bk_wlan_get_monitor_cb(void)
{
    return g_monitor_cb;
}

int bk_wlan_is_monitor_mode(void)
{
    return (0 == g_monitor_cb) ? false : true;
}

extern void bmsg_ps_sender(uint8_t ioctl);

/** @brief  Request deep sleep,and wakeup by gpio.
 *
 *  @param  gpio_index_map:The gpio bitmap which set 1 enable wakeup deep sleep.
 *              gpio_index_map is hex and every bits is map to gpio0-gpio31.
 *          gpio_edge_map:The gpio edge bitmap for wakeup gpios,
 *              gpio_edge_map is hex and every bits is map to gpio0-gpio31.
 *              0:rising,1:falling.
 */
void bk_enter_deep_sleep(UINT32 gpio_index_map,UINT32 gpio_edge_map)
{
    UINT32 param;
    UINT32 i;
    
    for (i = 0; i < GPIONUM; i++)
    {
        if (gpio_index_map & (0x01UL << i))
        {
            if(gpio_index_map & gpio_edge_map & (0x01UL << i))
            {
            	param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLUP);
            	sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param); 
            }
            else
            {
                param = GPIO_CFG_PARAM(i, GMODE_INPUT_PULLDOWN);
                sddev_control(GPIO_DEV_NAME, CMD_GPIO_CFG, &param);  
            }
        }
    }

    deep_sleep_wakeup_with_gpio(gpio_index_map,gpio_edge_map);
}

/** @brief  Enable dtim power save,close rf,and wakeup by ieee dtim dynamical
 *
 */
int bk_wlan_dtim_rf_ps_mode_enable(void )
{
    bmsg_ps_sender(PS_BMSG_IOCTL_RF_ENABLE);
}


/** @brief  When in dtim rf off mode,user can manual wakeup before dtim wakeup time.
 *          this function can not be called in "core_thread" context
 */
int bk_wlan_dtim_rf_ps_mode_do_wakeup()
{
    if(power_save_if_ps_rf_dtim_enabled()
            && power_save_if_rf_sleep())
    {
        power_save_rf_ps_wkup_semlist_wait();
    }
}

int bk_wlan_dtim_rf_ps_disable_send_msg(void)
{
    if(power_save_if_ps_rf_dtim_enabled()
            && power_save_if_rf_sleep())

    {
        power_save_wkup_event_set(NEED_DISABLE_BIT | NEED_ME_DISABLE_BIT);
    }
    else
    {
        power_save_dtim_rf_ps_disable_send_msg();
    }
}

/** @brief  Request exit dtim dynamical ps mode 
 */
int bk_wlan_dtim_rf_ps_mode_disable(void)
{
    bk_wlan_dtim_rf_ps_disable_send_msg();
    
	while( 1 )
	{
		if ( power_save_ps_mode_get() == PS_NO_PS_MODE)
		{
			break;
		}
		else
		{
			rtos_delay_milliseconds(100);
		}
	}
    
}

int bk_wlan_dtim_rf_ps_set_linger_time(UINT32 time_ms)
{
    power_save_set_linger_time(time_ms);
}

int bk_wlan_mcu_suppress_and_sleep(UINT32 sleep_ticks )
{
    TickType_t missed_ticks = 0;    
    missed_ticks = mcu_power_save( sleep_ticks );    
    vTaskStepTick( missed_ticks );
}

/** @brief  Enable mcu power save,close mcu ,and wakeup by irq
 */
int bk_wlan_mcu_ps_mode_enable(void)
{
    bmsg_ps_sender(PS_BMSG_IOCTL_MCU_ENABLE);
}

/** @brief  Disable mcu power save mode
 */
int bk_wlan_mcu_ps_mode_disable(void)
{
    bmsg_ps_sender(PS_BMSG_IOCTL_MCU_DISANABLE);
}

#if PS_DTIM_WITH_NORMAL
/** @brief  Open dtim with normal flag
 */
int bk_wlan_dtim_with_normal_open()
{
    ps_dtim_normal_enable = 1;
}

/** @brief  Close dtim with normal flag
 */
int bk_wlan_dtim_with_normal_close()
{
    ps_dtim_normal_enable = 0;
}
#endif

BK_PS_LEVEL global_ps_level = 0;
int bk_wlan_power_save_set_level(BK_PS_LEVEL level)
{
    if(level & PS_DEEP_SLEEP_BIT)
    {
        if(global_ps_level & PS_RF_SLEEP_BIT)
        {
            bk_wlan_dtim_rf_ps_mode_disable();
        }

        if(global_ps_level & PS_MCU_SLEEP_BIT)
        {
            bk_wlan_mcu_ps_mode_disable();
        }

        rtos_delay_milliseconds(100);
        bk_enter_deep_sleep(0xc000,0x0);
    }

    if((global_ps_level & PS_RF_SLEEP_BIT) ^ (level & PS_RF_SLEEP_BIT))
    {
        if(global_ps_level & PS_RF_SLEEP_BIT)
        {
            bk_wlan_dtim_rf_ps_mode_disable();
        }
        else
        {
            bk_wlan_dtim_rf_ps_mode_enable();
        }
    }

    if((global_ps_level & PS_MCU_SLEEP_BIT) ^ (level & PS_MCU_SLEEP_BIT))
    {
        if(global_ps_level & PS_MCU_SLEEP_BIT)
        {
            bk_wlan_mcu_ps_mode_disable();
        }
        else
        {
            bk_wlan_mcu_ps_mode_enable();
        }
    }

    global_ps_level = level;
}

void test_sta_connect_start_func(void *ctx)
{
	os_printf("--- start connect ---\r\n");	
}

void test_connection_lost_func(void *ctx)
{
	os_printf("--- connection loss ----\r\n");
}
void test_auth_fail_func(void *ctx)
{
	uint16_t *param = (uint16_t*)ctx;
	os_printf("--- auth fail:%x ----\r\n",*param);
}
void test_assoc_fail_func(void *ctx)
{
	uint16_t *param = (uint16_t*)ctx;
	os_printf("--- assoc fail:%x ----\r\n",*param);
}
void test_connected_func(void)
{
	os_printf("---- connected ----\r\n");
}

/*
 for user callback fuction resiter:
 
  connected callback func;
  connect start callback func;
  connection lost callback func;
  authentication fail callback func;
  association fail callback func 
*/
void user_callback_register(void)
{
	user_connected_callback(test_connected_func);
	user_callback_func_register(test_sta_connect_start_func,
								test_connection_lost_func,
								test_auth_fail_func,
								test_assoc_fail_func
	);
}
// eof

