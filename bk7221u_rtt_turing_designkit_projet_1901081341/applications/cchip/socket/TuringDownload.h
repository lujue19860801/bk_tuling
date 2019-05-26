#ifndef __TURING_DOWNLOAD_H__
#define __TURING_DOWNLOAD_H__

#include "TuringCommon.h"

int HttpDownLoadFile(char *url,char *file);

long HttpGetDownloadLength(char *url);
char *HttpGet(char *host, char *path);

#endif



