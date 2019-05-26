#ifndef __TURING_MEDIA_CONTROL_H__
#define  __TURING_MEDIA_CONTROL_H__

#define WECHAT_MUSIC_PLAYISLT 2



void SaveMediaState(void *pv);
void ResumeMediaState(void *pv);
void SetMediaState(void *pv, int state);
void SetMediaMode(void *pv, int mode);




#endif


