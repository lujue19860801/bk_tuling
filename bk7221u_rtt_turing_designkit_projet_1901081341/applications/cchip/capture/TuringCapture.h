#ifndef __TUNRING_CAPTURE_H__
#define __TUNRING_CAPTURE_H__

#include <rtdef.h>

#include "MediaService.h"
#include "RingFiFo.h"

#define CAPT_DEBUG_PRTF                     1

#define CAPT_FATAL_PRINTF                   rt_kprintf
#define CAPT_WARNING_PRINTF                 rt_kprintf
#define CAPT_LOG_PRINTF(...)

#if CAPT_DEBUG_PRTF
#define CAPT_PRINTF                         rt_kprintf
#else
#define CAPT_PRINTF(...)
#endif //CAPT_DEBUG_PRTF

#define CFG_SUPPORT_VOICE_SEND_TO_HTTP_SERVER     0

#define CFG_SUPPORT_RINGBUFFER              1
#define CFG_TC_VOICE_CHANNEL_NUM            1
#define CFG_FOREPART_VOICE_DISCARD_COUNT    1

#define CFG_VOICE_DURATTION_SPEECH          (6)
#define CFG_VOICE_DURATTION_WECHAT          (15)

#define MIC_GET_DATA_CHUNK_SIZE             (160 * 2 * 8)
#define MIC_RECORD_MAX_TIME_SECOND          (CFG_VOICE_DURATTION_SPEECH)

#define TURING_CAPTURE_TAG                   "TURING_CAPTURE"
#define TURING_CAPTURE_TASK_PRIORITY        10
#define TURING_CAPTURE_TASK_STACK_SIZE      (1024 * 3)
#define TURING_FIFO_SIZE                   ((1024 * 32 * 4) - 4)
#define CAPTURE_TIMEOUT                     6
#define VAD_COUNTS                          32

#include "wb_vad.h"

typedef enum
{
    CAPTURE_STATE_NONE = 0,
	CAPUTRE_STATE_PREPARE,
	CAPUTRE_STATE_PLAYTIP,
    CAPTURE_STATE_ONGOING,
    CAPTURE_STATE_DONE,
} CaptureState;

enum
{
    VAD_STATUS_UNDETECTED = 0,
    VAD_STATUS_DETECTED
};

enum
{
    VAD_ALG_NO_USE = 0,
    VAD_ALG_USE
};

enum
{
    VOICE_OWNER_SPEECH = 0,
    VOICE_OWNER_WECHAT
};

typedef struct _capture_entity_
{
    int state;
    int vad_status;
    int is_using_vad;
    int voice_owner;
	int is_canceling;
	
    int obtain_voice_data_count;
    int is_thread_running;
    struct rt_thread tc_thread;
    char tc_stack[TURING_CAPTURE_TASK_STACK_SIZE];

    RingBuffer *fifo;
    rt_mutex_t mutex;
    rt_sem_t sema_start;
    rt_sem_t sema_start_tone_over;
} TC_ENTITY_T;

typedef struct TuringCaptureService
{
    MediaService Based;
    int wifiConnected;
    int toneEnable;
    int sdMounted;
} TuringCaptureService;

int tc_entity_is_done(void);
int tc_entity_is_finished(void);
void tc_start_prompt_tone_over(void);
void tc_waiting_for_capture_finished(void);
void tc_fifo_clear(void);
void tc_fifo_deinit(void);
void tc_fifo_init(void);
int tc_fifo_get_data_length(void);
int tc_fifo_get_space_length(void);
int tc_fifo_data_take_out(unsigned char *buf,  unsigned int len);
void tc_finish_capture(void);
void tc_entity_deinit(TC_ENTITY_T **control);
TC_ENTITY_T *tc_entity_init(void);
void tc_set_entity_phase(int state);
static int tc_entity_is_cancled(void);

#ifdef CFG_ENABLE_MOUNT_TC_SERVICE
static void tc_task_entry(void *pv);
void tc_startup(int type, int voice_owner);
void tc_service_active(MediaService *pv);
TuringCaptureService *tc_service_create(void);
#else
static void tc_thread_entry(void *pv);
void tc_launch(int type, int voice_owner);
void tc_create(void);
void tc_active(void);
#endif
void tc_service_deactive(MediaService *self);
int tc_entity_is_prepare(void);

#endif
// eof

