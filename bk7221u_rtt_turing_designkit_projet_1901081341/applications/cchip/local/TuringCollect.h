#ifndef __TURING_COLLECT_H__
#define __TURING_COLLECT_H__


enum {
	COLLECT_NONE= 0,
	COLLECT_STARTED,
	COLLECT_ONGING,
	COLLECT_DONE,
	COLLECT_CLOSE,
	COLLECT_CANCLE,
};
int CollectSong(void * pv);
int TuringCollectSave(void * pv);

#endif

