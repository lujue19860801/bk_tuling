#ifndef __TURING_KEY_EVENT_H__
#define __TURING_KEY_EVENT_H__

#define KEYE_DEBUG_PRTF             1

#if KEYE_DEBUG_PRTF
#define KEYE_PRINTF                 rt_kprintf
#else
#define KEYE_PRINTF(...)                            
#endif //KEYE_DEBUG_PRTF

void TuringStartColumn(void *pv);
void TuringStartCollect(void *pv);
void TuringStartSpeech(void *pv, int type);
void TuringStartWeChat(void *pv, int type);
void TuringPlayWeChat(void *pv);
void TuringStartLock(void *pv);
void TuringShutDown(void *pv);
void FinshLastSession(void *pv);
void TuringKeyNextEvent(void *pv);
#endif
// eof

