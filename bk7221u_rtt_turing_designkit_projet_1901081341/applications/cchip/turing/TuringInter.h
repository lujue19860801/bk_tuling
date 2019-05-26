#ifndef __TURING_INTER_H__
#define __TURING_INTER_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <rtdef.h>
#include "MediaService.h"

#define INTER_DEBUG_PRTF                           1

#if INTER_DEBUG_PRTF
#define INTER_PRINTF                               rt_kprintf
#else
#define INTER_PRINTF(...)                            
#endif //INTER_DEBUG_PRTF

#define TI_RET_SUCCESS                            (0)
#define TI_RET_FAILURE                            (-1)

#define TURINT_INTER_TASK_STACK_SIZE              (4096)
#define TURINT_INTER_TASK_PRIORITY                (9)

#define CFG_TIMEOUT_PARAM
#define TI_RECV_TO_DURATION                       (3)
#define TI_SEND_TO_DURATION                       (5)

enum{
	MSG_UPLOAD_START = 0,
	MSG_UPLOAD_DONE = -1,
	MSG_UPLOAD_ABOLISH = -2
};

enum{
	STATE_UPLOADING = 0,
	STATE_UPLOAD_SUSPEND,
	STATE_UPLOAD_OVER
};

struct resp_header
{
    int status_code;         //HTTP/1.1 '200' OK
    char content_type[128];  //Content-Type: application/gzip
    long content_length;     //Content-Length: 11683079
};

typedef struct _inter_control_
{
    int sockfd;
    uint32_t is_canceling;
    int state;
    rt_mutex_t mux;
} InterControl;

typedef enum
{
    INTER_STATE_NONE = 0,
    INTER_STATE_ONGING,
    INTER_STATE_DONE,
} InterState;

typedef enum
{
    TURING_EVENT_START,
} TuringEventType;

typedef struct TuringNotification
{
    void *receiver;
    TuringEventType type;
    void *data;
    int len;
} TuringNotification;

typedef struct TuringInterService   //extern from TreeUtility
{
    /*relation*/
    MediaService Based;
    /*private*/
    int wifiConnected;
    int toneEnable;
    int sdMounted;
} TuringInterService;

void ti_set_cancel_state(void);
void ti_clear_cancel_state(void);
void ti_sockfd_close(void);
void SetInterSockfd(int fd);
void TuringInterInit(void *pv);
void StartTuringInter(void);
void TuringInterDeinit(void);
void updateTokenValue(const char *token);
void spendTime(long time);
void SetEndTime(long time);
void SetStartTime(long time);
void StartTuringUpload(void);
int ti_is_inter_canceling(void);
TuringInterService *TuringInterServiceCreate(void);
void ti_force_finished(void);
int getResponse(int socket_fd, char **text);
int ti_send_upload_msg(int msg);
void ti_waiting_for_upload_done(void);
#endif
// eof

