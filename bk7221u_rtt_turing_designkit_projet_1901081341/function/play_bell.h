#ifndef __PLAY_BELL_H__
#define __PLAY_BELL_H__

rt_err_t play_bell(const char *bell);
rt_err_t play_bell_asynchronous(const char *bell);
rt_err_t play_bell_recover(const char *bell);
char *get_bell_path(void);
#endif

