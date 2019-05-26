#ifndef _VAD_H_
#define _VAD_H_

#include "wb_vad_c.h"

#define VAD_DEBUG_PRTF                           0
#if VAD_DEBUG_PRTF
#define VAD_PRTF                                 rt_kprintf
#else
#define VAD_PRTF(...)                            
#endif //VAD_DEBUG_PRTF

#define VAD_RET_SUCCESS                          (0)
#define VAD_RET_FAILURE                          (-1)

#define VAD_OCCUR_NULL                           (0)
#define VAD_OCCUR_DONE                           (1)

#define HEAD_SPEECH_COUNT                        (8)
#define TAIL_NOISE_COUNT                         (16)

#define WB_FRAME_LEN                             (FRAME_LEN)

#define WB_FRAME_TYPE_NULL                      (2)
#define WB_FRAME_TYPE_SPEECH                    (1)
#define WB_FRAME_TYPE_NOISE                     (0)

extern void vad_pre_start(void);
extern int vad_entry(short samples[], int len);
extern int wb_vad_enter(void);
extern int wb_vad_entry(char *buffer, int len);
extern void wb_vad_deinit(void);
extern int wb_vad_get_frame_len(void);
extern void wb_estimate_init(void);

#endif // _VAD_H_
// eof

