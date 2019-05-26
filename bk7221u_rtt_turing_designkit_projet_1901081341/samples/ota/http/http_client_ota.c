/*
 * File      : http_client_ota.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-22     Murphy       the first version
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <rtthread.h>
#include <finsh.h>

#include "webclient.h"
#include "webclient_internal.h"
#include <fal.h>

#include "drv_flash.h"

#include "http_client_ota.h"
#include "flash_pub.h"

#define HTTP_OTA_BUFF_LEN         4096
#define HTTP_OTA_DL_DELAY         (10 * RT_TICK_PER_SECOND)

#define HTTP_OTA_URL              "http://192.168.10.135:80/rtthread.rbl"

#define FLASH_OTA_TEST             1

static void print_progress(size_t cur_size, size_t total_size)
{
    static unsigned char progress_sign[100 + 1];
    uint8_t i, per = cur_size * 100 / total_size;

    if (per > 100)
    {
        per = 100;
    }

    for (i = 0; i < 100; i++)
    {
        if (i < per)
        {
            progress_sign[i] = '=';
        }
        else if (per == i)
        {
            progress_sign[i] = '>';
        }
        else
        {
            progress_sign[i] = ' ';
        }
    }

    progress_sign[sizeof(progress_sign) - 1] = '\0';

    log_i("\033[2A");
    log_i("Download: [%s] %d%%", progress_sign, per);
}

int http_ota_fw_download(const char* uri)
{
    int fd = -1;
    int length, total_length = 0;
    rt_uint8_t *buffer_read = RT_NULL;
    struct webclient_session* session = RT_NULL;
    const struct fal_partition * dl_part = RT_NULL;

    session = webclient_open(uri);
    if (!session)
    {
        log_e("open uri failed.");
        goto __exit;
    }

    if (session->response == 304)
    {
        log_e("Firmware download failed! Server http response : 304 not modified!");
        goto __exit;
    }

    if (session->response != 200)
    {
        log_e("Firmware download failed! Server http response : %d !", session->response);
        goto __exit;
    }

    if (session->content_length == 0)
    {
        log_i("Request file size is 0!");
        goto __exit;
    }

    /* Get download partition information and erase download partition data */
    if ((dl_part = fal_partition_find(RT_BK_DL_PART_NAME)) == RT_NULL)
    {
        log_e("Firmware download failed! Partition (%s) find error!", RT_BK_DL_PART_NAME);
        goto __exit;
    }

    log_i("Start erase flash (%s) partition!", dl_part->name);
#if (FLASH_OTA_TEST == 1)
//for test
	flash_ctrl(CMD_FLASH_SET_CLK, 6);//set clk to 30M
#endif
    if (fal_partition_erase_all(dl_part) < 0)
    {
        log_e("Firmware download failed! Partition (%s) erase error!", dl_part->name);
        goto __exit;
    }
    log_i("Erase flash (%s) partition success!", dl_part->name);

    buffer_read = web_malloc(HTTP_OTA_BUFF_LEN);
    if (buffer_read == RT_NULL)
    {
        log_e("No memory for http ota!");
        goto __exit;
    }
    memset(buffer_read, 0x00, HTTP_OTA_BUFF_LEN);

    log_i("OTA file size is (%d)", session->content_length);

    do
    {
        length = webclient_read(session, buffer_read, HTTP_OTA_BUFF_LEN);   
        if (length > 0)
        {
            /* Write the data to the corresponding partition address */
            if (fal_partition_write(dl_part, total_length, buffer_read, length) < 0)
            {
                log_e("Firmware download failed! Partition (%s) write data error!", dl_part->name);
                goto __exit;
            }
            total_length += length;
            print_progress(total_length, session->content_length);
        }
        else
        {
            log_e("Exit: server return err (%d)!", length);
            goto __exit;
        }

    } while(total_length != session->content_length);

    if (total_length == session->content_length)
    {
        rt_thread_delay(rt_tick_from_millisecond(5));

        /* Reset the device, Start new firmware */
        extern void rt_hw_cpu_reset(void);
        rt_hw_cpu_reset();
    }

__exit:
    if (fd >= 0)
        close(fd);
    if (session != RT_NULL)
        webclient_close(session);
    if (buffer_read != RT_NULL)
        web_free(buffer_read);

    return 0;
}

void http_ota(uint8_t argc, char **argv)
{
    int parts_num;
    parts_num = fal_init();

    if (parts_num <= 0)
    {
        log_e("Initialize failed! Don't found the partition table.");
        return;
    }    
    if (argc < 2)
    {
        rt_kprintf("using uri: " HTTP_OTA_URL "\r\n");
        http_ota_fw_download(HTTP_OTA_URL);
    }
	else
	{
        http_ota_fw_download(argv[1]);
    }
}
/**
 * msh />http_ota [url]
*/
MSH_CMD_EXPORT(http_ota, OTA by http client: http_ota [url]);