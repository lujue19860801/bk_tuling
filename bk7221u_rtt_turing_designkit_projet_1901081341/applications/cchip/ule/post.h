#ifndef __POST_H__
#define __POST_H__


int RtosPost(int socketFd,char *url , char*body ,char *file, void *data,char *head);

int RtosGetResponse(int socket_fd, char **text);
int rtos_get_socket_fd(char *host);


#endif








