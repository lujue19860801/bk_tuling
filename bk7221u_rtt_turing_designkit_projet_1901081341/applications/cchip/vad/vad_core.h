#ifndef _VAD_CORE_H_
#define _VAD_CORE_H_

#include "TuringConfig.h"

#define FRAME_TYPE_SPEECH                  (1)
#define FRAME_TYPE_NOISE                   (0)

#define VAD_BIG_ENDIAN                     0
#define VAD_UNDERSAMPLED                   0

/*threshold value of power*/
#ifdef k1
#define POWER_THRESHOLD_VAL 89925  //26510
#endif

#if defined(dalianmao)|| defined(DALIANMAO_TX_01)
#define POWER_THRESHOLD_VAL 89925  //26510
#endif

#ifdef wannengpei
#define POWER_THRESHOLD_VAL 89925  //26510
#endif

#ifdef HUBA_30EYE
#define POWER_THRESHOLD_VAL 89925  //26510
#endif
#ifdef HUBA
#define POWER_THRESHOLD_VAL 89925  //26510
#endif


#if defined(XIAOWANDOU)  || defined(xiaoxing) || defined(AR002)

#define POWER_THRESHOLD_VAL 89925  //26510
#endif

extern int vad(short samples[], int len);

#endif // _VAD_CORE_H_
// eof
