#ifndef __TURING_UTILS_H__
#define __TURING_UTILS_H__

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#define UUID_MAX_LEN                      37
#define SESSION_UPLOAD_SIZE               16*1024
#define VOLUME_NAMESPACE                  "volume"

enum
{
    SC_TYPE_NONE = 0,
    SC_TYPE_AIRKISS = 1,
    SC_TYPE_SINVOICE = 2,
};


void GetUuidString(char *pDate);
long GetCurrentTime(void) ;
long GetCurrentTimeSec(void);
void PrintCurrentTime(void);
void PrintTime(time_t rawtime);
char *CompatStrdup(const char *src);
void *SafeRealloc(void *ptr, size_t size);
void GetRandStr(char s[], int number);
int GetDeviceID(char *id);
void ParseUrl(const char *URL, const char *protocl, char* *host, unsigned int *port, char **abs_path)  ;
int vad(short samples[], int len);
int UriHasScheme(char *uri);
char *UriGetSuffix(char *uri, char *data, int len);
int CreatePath(const char *path);
char *GetUrlSongName(char *url);
int GetFileLineNum(char *file);
time_t StrToTime(char *time, char *format);
void SetWifiConnectType(int type);
int GetWifiConnectType();
#endif
// eof

