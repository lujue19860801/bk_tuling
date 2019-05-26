
#ifndef __TURING_PARAM_H__
#define __TURING_PARAM_H__

#include "TuringCommon.h"

#define SPEECH_HOST "beta.app.tuling123.com";
#define IOT_HOST "iot-ai.tuling123.com"

#define UPLOADFILE_PATH "/resources/file/upload?apiKey="//"/resources/file/upload?apiKey=?apiKey="
#define GET_TOPIC_PATH "/iot/mqtt/topic"
#define GET_DATA_PATH "/iot/data"
#define GET_STATUS_PATH "/iot/status"
#define REPORT_STATUS_PATH "/iot/status/notify"
#define SEND_MESSAGE_PATH "/iot/message/send"
#define REPORT_AUDIO_PATH "/v2/iot/audio"//"/iot/audio"
#define COLLECT_SONG_PATH  				"/v2/iot/audio/collect"
#define GET_AUDIO_PATH 			 		"/v2/iot/audio"
#define VENDOR_AUTHORIZE_PATH   	 	"/vendor/new/authorize"
#define QUERY_DEVICE_STATUS_PATH  		"/vendor/new/device/status"
#define BIND_PATH   "/iot/bind"
#define SINVOICE_BIND_PATH "/iot/bind_from_sinvoice"
#define GET_MQTT_INFO_PATH   			"/vendor/mqtt?apiKey="
#define GET_WECHAT_TOKEN   				"/vendor/wechat_token?apiKey="

typedef struct turing_user{
	char deviceId[17];
	char aesKey[17];
	char apiKey[33];
	char token[33];
	char userId[33];
}TuringUser;

typedef struct turing_mqtt {
	char *topic;
	char *clientId;
	char *mediaId;
	char *fromUser;
	char *mqttUser;
	char *mqttPassword;
}TuringMqtt;

int  TuringUserInit(const char * apiKey , const char * aesKey ,const char *deviceId,   const char * token);
void TuringUserDeinit();
void SetTuringDeviceId(const char * deviceId) ;
char *GetTuringDeviceId(void) ;
void SetTuringToken(const char * token) ;
char *GetTuringToken() ;
void SetTuringUserId(const char * userId) ;
char *GetTuringUserId() ;
void SetTuringApiKey(const char * apiKey) ;
char *GetTuringApiKey() ;
char *GetTuringAesKey() ;
void SetTuringAesKey(const char * aesKey) ;
int TuringMqttInit(void);
void TuringMqttDeInit(void);
void SetTuringMqttClientId(char * clinetId) ;
void SetTuringMqttTopic(char * topic) ;
void SetTuringMqttMediaId(char * mediaId) ;
void SetTuringMqttFromUser(char * fromUser) ;
char *GetTuringMqttFromUser(void) ;
char *GetTuringMqttTopic(void) ;
char *GetTuringMqttClientId(void) ;
char *GetTuringMqttMediaId(void) ;
void SetTuringMqttMqttUser(char * userName) ;
void SetTuringMqttMqtPassword(char *passWord);
#endif

