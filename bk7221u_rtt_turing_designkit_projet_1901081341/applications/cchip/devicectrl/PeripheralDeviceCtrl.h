
#ifndef __PERIPHERAL_DEVICE_CTRL_H__
#define __PERIPHERAL_DEVICE_CTRL_H__



#define TIME_FORMAT 						"%02d:%02d:%02d"
#define DATE_FORMAT 						"%08s %02d %04d"

#define ESP32_AUDIO_VER						"0.0.1"

//#define DEF_FACTORY_OTA_VER				"esp32-audio-app-1803162020.bin"



typedef struct {
	char *pNameSpace;
	unsigned int nDataSpace;
	unsigned int nDataType;
	unsigned char *pDataKey;
	unsigned char *pDataVal;
	unsigned int nDataLen;
	
}NvsFlashStorage;




int NVS_Flash_WriteData(const char *namespace, const char *data);

char * NVS_Flash_ReadData(const char *namespace);

esp_err_t NVS_Flash_DeleteData(const char *namespace);

//void Factory_Info_Init(void);

void PowerOnPlayTone(void);

char *CurrentDateConversion(void);

unsigned int Get_Current_Time(void);

char *CurrentTimeConversion(void);

void FirmWare_Info_Init(void);
















#endif
