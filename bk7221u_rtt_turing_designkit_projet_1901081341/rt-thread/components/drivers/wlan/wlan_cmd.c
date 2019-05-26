/*
 * File      : wlan_cmd.c
 *             Wi-Fi common commands
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016-03-12     Bernard      first version
 */

#include <rtthread.h>
#include <wlan_dev.h>

#include <finsh.h>

#include <lwip/dhcp.h>
#include "wlan_cmd.h"

#ifdef LWIP_USING_DHCPD
#include <dhcp_server.h>
#endif
#include "TuringConfig.h"
#include "mode_mgmt.h"
#include "wifi_status_mgmt.h"

struct rt_wlan_info info;
static char wifi_ssid[32]    = {0};
static char wifi_key[32]     = {0};
static int network_mode      = WIFI_STATION;

#define WLAN_DEBUG   1
#if WLAN_DEBUG
#define WLAN_DBG(...)     rt_kprintf("[WLAN]"),rt_kprintf(__VA_ARGS__)
#else
#define WLAN_DBG(...)
#endif

#ifndef WIFI_SETTING_FN
#define WIFI_SETTING_FN     "/appfs/setting.json"
#endif

#ifndef WIFI_DEVICE_STA_NAME
#define WIFI_DEVICE_STA_NAME    "w0"
#endif
#ifndef WIFI_DEVICE_AP_NAME
#define WIFI_DEVICE_AP_NAME    "ap"
#endif

#ifdef RT_USING_DFS
#include <dfs_posix.h>
#ifdef PKG_USING_CJSON
#include <cJSON_util.h>
#endif


#include "cchip_led_control.h"
int wifi_get_mode(void)
{
    return network_mode;
}

int wifi_set_mode(int mode)
{
    network_mode = mode;

    return network_mode;
}

int wifi_set_setting(const char *ssid, const char *pwd)
{
    if (!ssid) return -1;

    strncpy(wifi_ssid, ssid, sizeof(wifi_ssid));
    wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';

    if (pwd)
    {
        strncpy(wifi_key, pwd, sizeof(wifi_key));
        wifi_key[sizeof(wifi_key) - 1] = '\0';
    }
    else wifi_key[0] = '\0';

    return 0;
}

int wifi_get_setting(const char **ssid, const char **pwd)
{
    if ((ssid == NULL) || (ssid == NULL)) 
        return -1;

    *ssid = wifi_ssid;
    *pwd = wifi_key;

    return 0;
}

#ifdef PKG_USING_CJSON
int wifi_read_cfg(const char *filename)
{
    int fd;
    cJSON *json = RT_NULL;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
    {
        /* no setting file */
        return -1;
    }

    if (fd >= 0)
    {
        int length;

        length = lseek(fd, 0, SEEK_END);
        if (length)
        {
            char *json_str = (char *) rt_malloc(length);
            if (json_str)
            {
                lseek(fd, 0, SEEK_SET);
                read(fd, json_str, length);

                json = cJSON_Parse(json_str);
                rt_free(json_str);
            }
        }
        close(fd);
    }

    if (json)
    {
        cJSON *wifi = cJSON_GetObjectItem(json, "wifi");
        cJSON *ssid = cJSON_GetObjectItem(wifi, "SSID");
        cJSON *key  = cJSON_GetObjectItem(wifi, "Key");
        cJSON *mode = cJSON_GetObjectItem(wifi, "Mode");

        if (ssid)
        {
            memset(wifi_ssid, 0x0, sizeof(wifi_ssid));
            rt_strncpy(wifi_ssid, ssid->valuestring, sizeof(wifi_ssid) - 1);
        }

        if (key)
        {
            memset(wifi_key, 0x0, sizeof(wifi_key));
            rt_strncpy(wifi_key, key->valuestring, sizeof(wifi_key) - 1);
        }

        if (mode)
        {
            network_mode = mode->valueint;
        }

        cJSON_Delete(json);
    }

    return 0;
}

int wifi_save_cfg(const char *filename)
{
    int fd;
    cJSON *json = RT_NULL;

    fd = open(filename, O_RDONLY, 0);
    if (fd >= 0)
    {
        int length;

        length = lseek(fd, 0, SEEK_END);
        if (length)
        {
            char *json_str = (char *) rt_malloc(length);
            if (json_str)
            {
                lseek(fd, 0, SEEK_SET);
                read(fd, json_str, length);

                json = cJSON_Parse(json_str);
                rt_free(json_str);
            }
        }
        close(fd);
    }
    else
    {
        /* create a new setting.json */
        fd = open(filename, O_WRONLY | O_TRUNC, 0);
        if (fd >= 0)
        {
            json = cJSON_CreateObject();
            if (json)
            {
                cJSON *wifi = cJSON_CreateObject();

                if (wifi)
                {
                    char *json_str;

                    cJSON_AddItemToObject(json, "wifi", wifi);
                    cJSON_AddStringToObject(wifi, "SSID", wifi_ssid);
                    cJSON_AddStringToObject(wifi, "Key", wifi_key);
                    cJSON_AddNumberToObject(wifi, "Mode", network_mode);

                    json_str = cJSON_Print(json);
                    if (json_str)
                    {
                        write(fd, json_str, rt_strlen(json_str));
                        cJSON_free(json_str);
                    }
                }
            }
        }
        close(fd);

        return 0;
    }

    if (json)
    {
        cJSON *wifi = cJSON_GetObjectItem(json, "wifi");
        if (!wifi)
        {
            wifi = cJSON_CreateObject();
            cJSON_AddItemToObject(json, "wifi", wifi);
        }

        if (cJSON_GetObjectItem(wifi, "SSID"))cJSON_ReplaceItemInObject(wifi, "SSID", cJSON_CreateString(wifi_ssid));
        else cJSON_AddStringToObject(wifi, "SSID", wifi_ssid);
        if (cJSON_GetObjectItem(wifi, "Key")) cJSON_ReplaceItemInObject(wifi, "Key", cJSON_CreateString(wifi_key));
        else cJSON_AddStringToObject(wifi, "Key", wifi_key);
        if (cJSON_GetObjectItem(wifi, "Mode")) cJSON_ReplaceItemInObject(wifi, "Mode", cJSON_CreateNumber(network_mode));
        else cJSON_AddNumberToObject(wifi, "Mode", network_mode);

        fd = open(filename, O_WRONLY | O_TRUNC, 0);
        if (fd >= 0)
        {
            char *json_str = cJSON_Print(json);
            if (json_str)
            {
                write(fd, json_str, rt_strlen(json_str));
                cJSON_free(json_str);
            }
            close(fd);
        }
        cJSON_Delete(json);
    }

    return 0;
}
#endif

int wifi_save_setting(void)
{
#ifdef PKG_USING_CJSON
    wifi_save_cfg(WIFI_SETTING_FN);
#endif

    return 0;
}
#endif


#include "ef_cfg.h"
#ifdef EF_USING_ENV



/*
	index: the ap list index
	ssid_buf: to store the ssid
	passcode_buf: to store password
	rw_flag:0--read,1-- write
*/
static int rw_ap_info(char index,char rw_flag,char *ssid_buf,char *passcode_buf)
{
	char *tmpbuf1 = NULL;
	char *tmpbuf2 = NULL;
	char ssid_key[]=SSIDKEYPREFIX;
	char passwd_key[]=PASSWDKEYPREFIX;

    rt_kprintf("rw_ap_info %p , %p\r\n",ssid_buf,passcode_buf);
    
	if((ssid_buf == NULL)||(passcode_buf == NULL) ||(index >= MAX_AP_LIST))
		return -2;

	snprintf(&ssid_key[sizeof(ssid_key)-2],2,"%d",index);
	snprintf(&passwd_key[sizeof(passwd_key)-2],2,"%d",index);

	if(rw_flag)
	{
		ef_set_env(ssid_key,ssid_buf);
		ef_set_env(passwd_key,passcode_buf);
		ef_save_env();
	}
	else
	{
		tmpbuf1 = ef_get_env(ssid_key);
		tmpbuf2 = ef_get_env(passwd_key);
		if((tmpbuf1 == NULL)&&(tmpbuf2 == NULL))
		{
			return -1;
		}

		strcpy(ssid_buf,tmpbuf1);
		strcpy(passcode_buf,tmpbuf2);
	}
	return 0;
}


void wifi_save_ap(char *ssidbuf,char *passcode_buf)
{
	int i = 0;
	
	rt_kprintf("%s , %s\r\n",ssidbuf, passcode_buf);
    
	if((ssidbuf == NULL)||(passcode_buf == NULL))
		return;

	rw_ap_info(i,1,ssidbuf,passcode_buf);
}

int wifi_get_ap_info_from_ef(void)
{
	int ret = WCMD_FAILURE;
	int i = 0;
    char *tmp;
	char *ssid_buf;
	char *passcode_buf;

	tmp = (char *)rt_malloc(SSID_MAX_COUNT + PWS_MAX_COUNT);
	if(RT_NULL == tmp)
	{
		goto get_exit;
	}
	ssid_buf = tmp;
	passcode_buf = &tmp[SSID_MAX_COUNT];
		
    rt_kprintf("%p , %p\r\n", ssid_buf, passcode_buf);
	if(rw_ap_info(i,0,ssid_buf,passcode_buf)==0)
	{
		change_led_state(LED_STATE_CONNECTING);
		
		rt_kprintf("%s , %s\r\n",ssid_buf,passcode_buf);
        rt_kprintf("wifi_get_ap_info_from_ef %s, %s\n", ssid_buf, passcode_buf);
        rt_kprintf("wifi_get_ap_info_from_ef %d, %d\n", strlen(ssid_buf), strlen(passcode_buf));
        wifi_set_mode(WIFI_STATION);
        wifi_set_setting(ssid_buf, passcode_buf);
        wifi_default();

		ret = WCMD_SUCCESS;
	}
	else 
	{
        rt_kprintf("no_ap_info_found\r\n");
    }

	rt_free(tmp);

get_exit:	
	return ret;
}
#if FACTORY_TEST_ENABLE 
int try_get_factory_ssid(void)
{
	char *ssidbuf = NULL, *pswbuf = NULL;
    char *tmp = (char *)rt_calloc(1,SSID_MAX_COUNT + PWS_MAX_COUNT);
	int ret = -1;
	
	if(NULL == tmp)
	{
		rt_kprintf("====factory_ssid fail===\r\n");
		return -1;
	}
	ssidbuf = tmp;
	pswbuf = &tmp[SSID_MAX_COUNT];
	set_factory_ssid(0);
	ret = rw_ap_info(0,0,ssidbuf,pswbuf);
	if(((0 == ret)&&(0 == strcmp(ssidbuf,FACTORYTEST_SSID)))||(-1 == ret))
	{
		if(-1 == ret)
		{
			strcpy(ssidbuf,FACTORYTEST_SSID);
			strcpy(pswbuf,FACTORYTEST_PASSWORD);			
		}
		set_factory_ssid(1);
		wifi_set_mode(WIFI_STATION);
        wifi_set_setting(ssidbuf, pswbuf);
        wifi_default();
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	
	rt_free(tmp);
	
	return ret;
}
#endif

int test_env(int argc, char ** argv)
{
	if(argc == 1)
	{
		wifi_get_ap_info_from_ef();
		
		return 0;
	}

	if(argc == 3)
	{
		wifi_save_ap(argv[1],argv[2]);
		return 0;
	}
}

MSH_CMD_EXPORT(test_env,test_env command );
#endif

int wifi_softap_setup_netif(struct netif *netif)
{
    if (netif)
    {
#ifdef RT_LWIP_DHCP
        /* Stop DHCP Client */
        dhcp_stop(netif);
#endif

#ifdef LWIP_USING_DHCPD
        {
            char name[8];
            memset(name, 0, sizeof(name));
            strncpy(name, netif->name, sizeof(name) > sizeof(netif->name) ? sizeof(netif->name) : sizeof(name));
            dhcpd_start(name);
        }
#endif
    }

    return 0;
}

int wifi_default(void)
{
    int result = 0;
    struct rt_wlan_device *wlan;

#ifdef PKG_USING_CJSON
    /* read default setting for wifi */
    wifi_read_cfg(WIFI_SETTING_FN);
#endif

    if (network_mode == WIFI_STATION)
    {
        /* get wlan device */
        wlan = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_STA_NAME);
        if (!wlan)
        {
            rt_kprintf("no wlan:%s device\n", WIFI_DEVICE_STA_NAME);
            return -1;
        }

        /* wifi station */
        rt_wlan_info_init(&info, WIFI_STATION, SECURITY_WPA2_MIXED_PSK, wifi_ssid);
        result = rt_wlan_init(wlan, WIFI_STATION);
        if (result == RT_EOK)
        {
            result = rt_wlan_connect(wlan, &info, wifi_key);
        }
    }
    else
    {
        /* wifi AP */
        /* get wlan device */
        wlan = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_AP_NAME);
        if (!wlan)
        {
            rt_kprintf("no wlan:%s device\n", WIFI_DEVICE_AP_NAME);
            return -1;
        }

        rt_wlan_info_init(&info, WIFI_AP, SECURITY_WPA2_AES_PSK, wifi_ssid);
        info.channel = 11;

        /* wifi soft-AP */
        result = rt_wlan_init(wlan, WIFI_AP);
        if (result == RT_EOK)
        {
            result = rt_wlan_softap(wlan, &info, wifi_key);
        }
    }

    return result;
}

static void wifi_usage(void)
{
    rt_kprintf("wifi help     - Help information\n");
    rt_kprintf("wifi cfg SSID PASSWORD - Setting your router AP ssid and pwd\n");
    rt_kprintf("wifi          - Do the default wifi action\n");
    rt_kprintf("wifi wlan_dev scan\n");
    rt_kprintf("wifi wlan_dev join SSID PASSWORD\n");
    rt_kprintf("wifi wlan_dev ap SSID [PASSWORD]\n");
    rt_kprintf("wifi wlan_dev up\n");
    rt_kprintf("wifi wlan_dev down\n");
    rt_kprintf("wifi wlan_dev rssi\n");
    rt_kprintf("wifi wlan_dev status\n");
}

int wifi(int argc, char **argv)
{
    struct rt_wlan_device *wlan;

    if (argc == 1)
    {
        wifi_default();
        return 0;
    }

    if (strcmp(argv[1], "help") == 0)
    {
        wifi_usage();
        return 0;
    }

    if (strcmp(argv[1], "cfg") == 0)
    {
        /* configure wifi setting */
        memset(wifi_ssid, 0x0, sizeof(wifi_ssid));
        rt_strncpy(wifi_ssid, argv[2], sizeof(wifi_ssid) - 1);

        memset(wifi_key, 0x0, sizeof(wifi_key));
        rt_strncpy(wifi_key, argv[3], sizeof(wifi_key) - 1);

        network_mode = WIFI_STATION;

#ifdef PKG_USING_CJSON
        wifi_save_cfg(WIFI_SETTING_FN);
#endif

        return 0;
    }

    /* get wlan device */
    wlan = (struct rt_wlan_device *)rt_device_find(argv[1]);
    if (!wlan)
    {
        rt_kprintf("no wlan:%s device\n", argv[1]);
        return 0;
    }

    if (argc < 3)
    {
        wifi_usage();
        return 0;
    }

    if (strcmp(argv[2], "join") == 0)
    {
        rt_wlan_init(wlan, WIFI_STATION);
        network_mode = WIFI_STATION;
        
        wifi_set_setting(argv[3], argv[4]);

        /* TODO: use easy-join to replace */
        rt_wlan_info_init(&info, WIFI_STATION, SECURITY_WPA2_MIXED_PSK, argv[3]);
        rt_wlan_connect(wlan, &info, argv[4]);
        rt_wlan_info_deinit(&info);
    }
    else if (strcmp(argv[2], "up") == 0)
    {
        /* the key was saved in wlan device */
        rt_wlan_connect(wlan, RT_NULL, wlan->key);
    }
    else if (strcmp(argv[2], "down") == 0)
    {
        rt_wlan_disconnect(wlan);
        rt_wlan_info_deinit(&info);
    }
    else if (strcmp(argv[2], "scan") == 0)
    {
        struct rt_wlan_scan_result *scan_result = RT_NULL;

        rt_wlan_scan(wlan, &scan_result);
        if (scan_result)
        {
            int index, num;

            num = scan_result->ap_num;
            rt_kprintf("             SSID                      MAC            rssi   chn    Mbps\n");
            rt_kprintf("------------------------------- -----------------     ----   ---    ----\n");
            for (index = 0; index < num; index ++)
            {
                rt_kprintf("%-32.32s", scan_result->ap_table[index].ssid);
                rt_kprintf("%02x:%02x:%02x:%02x:%02x:%02x     ", 
                    scan_result->ap_table[index].bssid[0],
                    scan_result->ap_table[index].bssid[1],
                    scan_result->ap_table[index].bssid[2],
                    scan_result->ap_table[index].bssid[3],
                    scan_result->ap_table[index].bssid[4],
                    scan_result->ap_table[index].bssid[5]
                );
                rt_kprintf("%4d    ", scan_result->ap_table[index].rssi);
                rt_kprintf("%2d    ", scan_result->ap_table[index].channel);
                rt_kprintf("%d\n", scan_result->ap_table[index].datarate / 1000000);
            }
        }
        rt_wlan_release_scan_result(&scan_result);
    }
    else if (strcmp(argv[2], "rssi") == 0)
    {
        int rssi;

        rssi = rt_wlan_get_rssi(wlan);
        rt_kprintf("rssi=%d\n", rssi);
    }
    else if (strcmp(argv[2], "ap") == 0)
    {
        rt_err_t result = RT_EOK;

        if (argc == 4)
        {
            // open soft-AP
            rt_wlan_info_init(&info, WIFI_AP, SECURITY_OPEN, argv[3]);
            info.channel = 11;

            result = rt_wlan_init(wlan, WIFI_AP);
            /* start soft ap */
            result = rt_wlan_softap(wlan, &info, NULL);
            if (result == RT_EOK)
            {
                network_mode = WIFI_AP;
            }
        }
        else if (argc == 5)
        {
            // WPA2 with password
            rt_wlan_info_init(&info, WIFI_AP, SECURITY_WPA2_AES_PSK, argv[3]);
            info.channel = 11;

            result = rt_wlan_init(wlan, WIFI_AP);
            /* start soft ap */
            result = rt_wlan_softap(wlan, &info, argv[4]);
            if (result == RT_EOK)
            {
                network_mode = WIFI_AP;
            }
        }
        else
        {
            wifi_usage();
        }

        if (result != RT_EOK)
        {
            rt_kprintf("wifi start failed! result=%d\n", result);
        }
    }
    else if (strcmp(argv[2], "status") == 0)
    {
        int rssi;

        if (netif_is_link_up(wlan->parent.netif))
        {
            rssi = rt_wlan_get_rssi(wlan);

            rt_kprintf("Wi-Fi AP: %-.32s\n", wlan->info->ssid);
            rt_kprintf("MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n", wlan->info->bssid[0],
                       wlan->info->bssid[1],
                       wlan->info->bssid[2],
                       wlan->info->bssid[3],
                       wlan->info->bssid[4],
                       wlan->info->bssid[5]);
            rt_kprintf(" Channel: %d\n", wlan->info->channel);
            rt_kprintf("DataRate: %dMbps\n", wlan->info->datarate / 1000000);
            rt_kprintf("    RSSI: %d\n", rssi);
        }
        else
        {
            rt_kprintf("wifi disconnected!\n");
        }

        return 0;
    }

    return 0;
}
MSH_CMD_EXPORT(wifi, wifi command);
