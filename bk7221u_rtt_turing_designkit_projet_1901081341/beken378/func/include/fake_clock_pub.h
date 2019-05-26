#ifndef _FAKE_CLOCK_PUB_H_
#define _FAKE_CLOCK_PUB_H_

#include "include.h"

#define FCLK_PWM_ID           PWM0
#define FCLK_DURATION_MS      1

#define FCLK_SECOND           (1000/FCLK_DURATION_MS)

extern UINT32 fclk_get_tick(void);
extern UINT32 fclk_get_second(void);
extern void fclk_reset_count(void);
extern void fclk_init(void);
extern UINT32 fclk_from_sec_to_tick(UINT32 sec);
extern UINT32 fclk_cal_endvalue(UINT32 mode);

#endif // _FAKE_CLOCK_PUB_H_
// eof

