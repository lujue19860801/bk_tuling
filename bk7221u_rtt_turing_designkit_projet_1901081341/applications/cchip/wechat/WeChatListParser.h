#ifndef __WECHAT_LIST_PARSER_H__
#define  __WECHAT_LIST_PARSER_H__

#include "mylist.h"
#include <time.h>

typedef struct weChatItem
{
    time_t time;
	
    char *url;
    char *tip;
    char *mediaId;
    char *fromUser;
} WeChatItem;

void FreeWeChatItem(void *data);
WeChatItem *NewWeChatItem(time_t time, char *url, char *fromUser, char *tip);
int GetWeChatListFromJson(list_t *list, char *p);
int WeChatListAddItem(list_t *list, WeChatItem *item);
char *WeChatListToJson(list_t *list);
#endif
// eof

