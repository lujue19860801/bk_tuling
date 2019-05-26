#ifndef __TURING_INTER_JSON_H__
#define __TURING_INTER_JSON_H__

#include "TuringCommon.h"

#define TIJ_DEBUG_PRTF                     0

#define TIJ_FATAL_PRINTF                   rt_kprintf
#define TIJ_WARNING_PRINTF                 rt_kprintf
#define TIJ_LOG_PRINTF(...)

#if TIJ_DEBUG_PRTF
#define TIJ_PRINTF                         rt_kprintf
#else
#define TIJ_PRINTF(...)
#endif //TIJ_DEBUG_PRTF


/* Error Code*/
#define EC_VALUE_ERROR	                         40001	 /*字段错误*/
#define EC_ILLEGAL_VALUE	                     40002	 /*非法字段*/
#define EC_VALUE_IS_NULL_OR_MISSING	             40003	 /*字段为空或错误*/
#define EC_ASR_FAILURE	                         40004	 /*语音解析失败*/
#define EC_TTS_FAILURE	                         40005	 /*文本转语音失败*/
#define EC_NLP_FAILURE	                         40006	 /*语义解析失败*/
#define EC_TOKEN_INVALID_VALUE	                 40007	 /*无效token*/
#define EC_IS_EXPIRED	                         40008	 /*过期*/
#define EC_ACTIVE_INTERACTION_CORPUS_NOT_EXIST	 40010	 /*主动交互语料不存在*/
#define EC_GREETING_CORPUS_NOT_EXIST	         40011	 /*打招呼语料不存在*/
#define EC_REQUEST_IS_FORBIDDEN	                 40012	 /*拒绝请求*/
#define EC_OUT_OF_DEVICE_COUNT_LIMIT	         40013	 /*请求超出限制*/
#define EC_ASR_IO_READ_ERROR	                 43000	 /*读取asr上传音频流失败*/
#define EC_ASR_SERVICE_OUTIME	                 43010	 /*ASR服务器端超时*/
#define EC_ASR_CLIENT_OUTIME	                 43020	 /*ASR客户端超时*/
#define EC_ASR_EXCEPTION	                     43030	 /*ASR识别抛异常*/
#define EC_ASR_JT_ERROR	                         43040	 /*ASR的捷通服务返回错误*/
#define EC_UNKNOWN_ERROR	                     49999	 /*未知错误*/
#define EC_IN_PROGRESS	                         40000	 /*正在进行流式识别*/

typedef enum {
	TURING_FUNC_CHAT = 20000,
	TURING_SLEEP_CONTROL,		  //休眠控制
	TURING_VOLUME_CONTROL,		  //音量控制
	TURING_WEATHER_INQUIRY,       //天气查询  		 
	TURING_DATE_INQUIRY = 20005,		  //日期时间查询
	TURING_COUNT_CALCLATE,		  //计算器
	TURING_MUSIC_PLAY,            //播音乐
	TURING_STORY_TELL,			  //讲故事
	TURING_POEMS_RECITE,		  //古诗词
	TURING_ANIMAL_SOUND = 20011,          //动物叫声
	TURING_KNOWLEDGE,			  //百科知识
	TURING_PHONE_CALL,            //打电话
	TURING_SOUND_GUESS,			  //猜叫声
	TURING_CHINESE_ENGLISH,       //中英互译
	TURING_DANCE,		 		  //跳舞
	TURING_ENGLISH_DIALOGUES = 20018,     //英文对话
	TURING_MUSICAL_INSTRUMENTS,   //乐器声音
	TURING_NATURE_SOUND,          //大自然的声音	
	TURING_SCREEN_BRIGHTNESSS,    //屏幕亮度
	TURING_ELECTRICITY_QUERY,     //电量查询
	TURING_MOTION_CONTROL,        //运动控制
	TURING_TAKE_PHOTO,     		  //拍照
	TURING_ALARM_CLOCK,           //闹钟
	TURING_OPEN_APP,     		  //打开应用
	TURING_KNOWLEDGE_BASE, 		   //知识库
	TURING_ACTYIVE_INTER = 29998, //主动交互
	TURING_MAEKED_WORDS,		  //提示语
}TuringWifiFunc;

typedef enum {
	INTERACT_LIST_IND = 0,
	LOCAL_LIST_IND = 1,
	MENU_LIST_IND = 2,
	WECHAT_LIST_IND = 3,	
}music_list_index;

enum
{
	PLAYING_STATUS_PAUSE = 0x0,
	PLAYING_STATUS_PLAYING,
	PLAYING_STATUS_INTERRUPT_PAUSE
};

typedef enum
{
	TEST_START = 0,
	_KEY_COLLECT_ = 1,
	_KEY_VOICE_ = 2,
	_KEY_WECHAT_ = 3,
	_KEY_PREV_ = 4,
	_KEY_NEXT_ = 5,
	_KEY_MIC_DAC_LOOP = 6,
	TEST_FINISH = 7,
}FACTORY_MUSIC_ITEM;

int ProcessTuringJson(char *pData, void *service);
music_list_index Getplaylistindex(void );
void Setplaylistindex(music_list_index item);
int lplayer_get_status(void);
void lplayer_set_status(int item);
void Turing_list_items_create(void);
void Turing_list_items_delete(void);
int Turing_list_items_add_url(const char *name,const char *url);
void Turing_list_items_play(void);
void Turing_list_player_set_evt_handler(void);
void factory_test_play(FACTORY_MUSIC_ITEM item);
#endif
//  eof

