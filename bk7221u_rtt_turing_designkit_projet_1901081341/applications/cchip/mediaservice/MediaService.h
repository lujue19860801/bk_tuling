#ifndef __MEDIA_SERVICE__
#define __MEDIA_SERVICE__

#include "linklist.h"
#define MEDIA_SERVICE_NAME_MAX 8
typedef struct MediaService MediaService;

struct MediaService { 
	struct list_head list;
	char name[MEDIA_SERVICE_NAME_MAX];
	
    void (*serviceActive)(struct MediaService* serv);
    void (*serviceDeactive)(struct MediaService* serv);
};

void MediaServicesDeactive(char* service);
void addMediaService(MediaService *service);
void activeMediaServices(void);
void deactiveMediaServices(void);
#endif
// eof
