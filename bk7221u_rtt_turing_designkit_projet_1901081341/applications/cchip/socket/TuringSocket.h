#ifndef __TURING_SOCKET_H__
#define __TURING_SOCKET_H__

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include <stdio.h>


int CreateSocketFd( char *host, int timeout);
int GetResponse(int socket_fd, char **text);
int   GetResponseAndWriteFile(int socketFd, FILE *fp);

long    GetContentLength(int socketFd);

#endif



