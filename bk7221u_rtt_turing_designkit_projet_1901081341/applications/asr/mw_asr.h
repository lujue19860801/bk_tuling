#ifndef MW_ASR_H_
#define MW_ASR_H_

typedef enum{
	ASR_STATE_OK = 0,
	ASR_STATE_ERROR,
	ASR_STATE_RESTART,
	ASR_STATE_FINISH
}MwAsrStatus;

typedef enum{
	ASR_RESULT_BAD_REQUEST = 0,				//��Ч����
	ASR_RESULT_UNAUTHORIZED,				//��֤����
	ASR_RESULT_SERVER_ERROR,				//���������ʹ���
	ASR_RESULT_SERVICE_ERROR,				//�ƶ����������ش���
	ASR_RESULT_NETWORK_ERROR,				//�����쳣
	ASR_RESULT_NO_RESPONSE					//�����������Ӧ��һ�����ϴ��˲�������Ч�������Ƶ
}MwAsrError;


typedef enum{
	ASR_SCENE_CHAT = 1,						//����
	ASR_SCENE_MUSIC,						//����
	ASR_SCENE_CLOCK,						//���ӡ�����
	ASR_SCENE_CALL,							//��绰
	ASR_SCENE_VOLUME,						//��������
	ASR_SCENE_SLEEP,						//����
	ASR_SCENE_OTHER							//δ�������������
}MwAsrScene;

typedef struct{
	MwAsrError error;
}MwAsrErrorResult;

//���ֳ����µľ������
enum{
	MUSIC_OP_PLAY = 1,						//��Դ����
	MUSIC_OP_PAUSE,							//������ͣ
	MUSIC_OP_RESUME,						//���Żָ�
	MUSIC_OP_NEXT,							//�л�����һ��
	MUSIC_OP_PREV,							//�л�����һ��
	MUSIC_OP_STOP,							//ֹͣ����
	MUSIC_OP_REPEAT							//�ظ�����
};

//���ڸ�������ͬ������״̬
typedef enum{
	MUSIC_STATUS_IDLE,
	MUSIC_STATUS_BUFFERING,
	MUSIC_STATUS_PLAYING,
	MUSIC_STATUS_PAUSED,
	MUSIC_STATUS_FINISHED,
	MUSIC_STATUS_STOPPED
}LOCAL_MUSIC_PLAY_STATUS_E;

//����������ֳ����������
typedef struct {
	int type;								//��������:���š���ͣ���ָ����л���Ŀ��ֹͣ��
	int in_album;							//��ǰ��Ŀ�Ƿ������ƶ�ר�������ǣ���ɽ����������л��������л���Ч
	char* title;							//��Ŀ���ƣ���"����ˮ"
	char* singer;							//����
	char* prompt;							//���Ÿ���֮ǰ��������ܣ���"Ϊ���������»�����ˮ"
	char* url;								//����url
}MwAsrSceneMusic;

//����ʶ�����ӳ����������
enum{
	CLOCK_OP_SET = 1,						//�������ӻ�����
	CLOCK_OP_DEL,							//ɾ�����ӻ�����
	CLOCK_OP_STOP,							//ֹͣ��ǰ����
	CLOCK_OP_UPDATE,						//��������������Ϣ
	CLOCK_OP_DELAY,							//�Ӻ�����
	CLOCK_OP_GET,							//��ѯ����
	CLOCK_OP_MAX
};

enum MW_ALARM_OPERATION
{
	DATA_OPR_NONE,
	DATA_OPR_SET = 1,
	DATA_OPR_UPDATE,
	DATA_OPR_LIST,
	DATA_OPR_DELETE,
	DATA_OPR_STOP,
	DATA_OPR_DELAY,
	DATA_OPR_MAX
};

typedef struct{
	int year;
	int month;		
	int mday;
}mw_date_t;

typedef struct {	
	int year;
	int month;		//months [1,12]
	int mday;		//day of the month [1,31]
	
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
}mw_tm_t;


//������Ϣ
typedef struct
{
	int id;										//����Ψһ��ʶ

	mw_tm_t alarm_time;							//���Ӽ���ʱ��
	mw_date_t start_date;						//������Ч������ʼ
	mw_date_t end_date;							//������Ч���ڽ�ֹ
	char szalarm_time[32];

	int repeat_mode;							//�����Ƿ��ظ�
	int operation;								//���Ӳ�������
	char *content;								//��������
}mw_alarm_node_t;

//���ӳ����������
typedef struct {
	int op;										//���Ӳ���:���á�ɾ�������µ�
	int num;									//����������Ŀ
	char* prompt;								//������ʾ����
	mw_alarm_node_t *nodes;						//���������Ϣ
}MwAsrSceneClock;

//��绰�����������
typedef struct {
	char* contact;								//��ϵ��
	char* phoneNumber;							//�绰����
	char* prompt;								//��ʾ����
}MwAsrSceneCall;

//����������������
enum{
	VOLUME_UP = 1,								//��������
	VOLUME_DOWN,								//��С����
	VOLUME_MAXIMIZE,							//�������
	VOLUME_MINIMIZE								//������С��
};

//��������������������������
typedef struct {
	int op;										//������������:���󡢵�С��
	int param;									//��������(0-100)
	char* prompt;								//������ʾ����
}MwAsrSceneVolume;

//�����������Ĭ�Ͻ������
typedef struct {
	char* prompt;								//��ʾ����
	char* url;									//�����������Դ����Դurl
}MwAsrSceneCommon;

//��������������
typedef struct{
	MwAsrScene 				scene;				//�����������
	struct{
		int 					emotion;		//������ʶ�𵽵���������
		int 					session_seq;
	}sceneContext;
	union{
		MwAsrSceneMusic 			scene_music;
		MwAsrSceneClock 			scene_clock;
		MwAsrSceneCall     			scene_call;
		MwAsrSceneVolume     		scene_volume;
		MwAsrSceneCommon 		scene_common;
	}sceneData;
}MwAsrSceneResult;



typedef void (*asr_message_callback)(MwAsrStatus status,void* info);

typedef struct{
	asr_message_callback		asrstate_cb;
}MwAsrClientCbs;


//��ʼ���������ģ��
int mw_asr_client_create(const char* config_file,int multiturn,MwAsrClientCbs cbs);

//���ĳЩ�����ƣ���Ҫ�豸�˵��ø÷�����app�˽�����Ӧ��ϵ
int mw_asr_client_attach(const char* account);

//�������ѣ����������������������
void mw_asr_client_touch_wakeup();

//��pcm��Ƶ����д���������ģ�顣����:pData,16k 16bit pcm��������;length,pcm�������ݳ���
void mw_asr_client_write_audio(unsigned char* pData,unsigned int length);

void mw_asr_client_set_busy(int busy);

void mw_asr_client_end_session(int active_flag,int multiturn_stop);

//�������ģ������
void mw_asr_client_destroy();

#endif
