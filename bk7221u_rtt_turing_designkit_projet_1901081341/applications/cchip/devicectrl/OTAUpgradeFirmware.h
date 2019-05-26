
#ifndef __OTA_UPGRADE_FIRMWARE_H__
#define __OTA_UPGRADE_FIRMWARE_H__


#define OTA_NAMESPACE			"ota_ver"

#define EXAMPLE_WIFI_SSID	 	"D-Link123"
#define EXAMPLE_WIFI_PASS	 	"88888888"

#define ESP32_FW_NAME  			"esp32-audio-app-"

#define UPGRADE_SERVER_IP	 	"120.77.233.10"

#define UPGRADE_SERVER_PORT		"80"

#if 1
#define UPGRADE_FILENAME		"/ESP32_YIYA_FW/"
#else
#define UPGRADE_FILENAME		"/ESP32_TEST_FW/"
#endif

#define BUFFSIZE				 (1024)

#define TEXT_BUFFSIZE 			 (1024)

#define UPGRADE_BUFSIZE			 2048


void CCHIP_OTA_Upgrade_FW_Init(void);

//esp_err_t ota_example_task_test(void *pvParameter);

char *Ota_Download_Http_RequestData(void);

int Ota_Check_Firmware_Version(void);

void unlock_otaupdating_cond(void);




#endif