#ifndef __LIST_PLAYER_INCLUDE__
#define __LIST_PLAYER_INCLUDE__

#ifdef __cplusplus
extern "C"
{
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include "player_app.h"

#define LIST_PLAYER_DEBUG  rt_kprintf

typedef struct music_list *music_list_t;

/* 列表播放器事件回调函数 */
typedef void (*lister_event_handler)(int event);
/* 播放列表回调事件 */
typedef void (*list_event_handler)(music_list_t table, int event, void *arg);

struct music_item
{
    char *name;
    char *URL;
};
typedef struct music_item *music_item_t;

struct music_info
{
    rt_list_t node;
    struct music_item item;
};

struct music_list
{
    rt_list_t list;

    int item_num;                      /* 歌曲数目 */
    int item_position;                 /* 单曲的播放进度 */
    struct rt_mutex mutex;
    struct music_info *curr_info;      /* 当前正在播放的歌曲信息 */

    list_event_handler event_handler;  /* 播放列表回调事件 */
    void *user_data;
};

struct player_list
{
    struct player_app parent;

    music_list_t table;

    struct rt_work work;
    int status;     /* 播放器状态 */
    int event;
    int player_mode;

    struct rt_mutex lock;
    struct rt_mutex owner;
    lister_event_handler evt_handler;
};

/* 回调事件 */
#define LISTER_BEGIN_OF_ITEM    0x01    /* 单曲开始*/
#define LISTER_END_OF_ITEM      0x02    /* 单曲结束 */
#define LISTER_NEXT_ITEM        0x04    /* 下一曲 */
#define LISTER_PREV_ITEM        0x08    /* 上一曲 */
#define LISTER_BEGIN_OF_LIST    0x10    /* 播放列表开始 */
#define LISTER_END_OF_LIST      0x20    /* 播放列表结束 */

/* 播放器初始化 */
int list_player_init(void);

/* 播放歌单*/
int list_player_play(music_list_t table);
/* 清空歌单, 歌单会被自动回收 */
int list_player_empty(void);
/* 从播放器中脱离歌单，歌单不会回收 */
music_list_t list_player_detach_items(void);

/*
 * 设置事件回调函数
 * 会在：
 * 每首歌曲结束，
 * 按键，播放下一首
 * 按键，播放上一首
 *
 * 列表播放结束
 * 进行回调
 */
void list_player_set_handler(lister_event_handler handler);

/* 设置播放列表回调函数 */
void list_player_set_table_handler(music_list_t table, list_event_handler handler, void *arg);

/* 列表播放器模式设置 */
int list_player_mode_set(int mode);

/* 播放模式 */
#define LISTER_NONE_MODE        (0x00)  //模式未设置
#define LISTER_LIST_ONCE_MODE   (0x01)  //列表单次循环（顺序播放）
#define LISTER_SONG_ONCE_MODE   (0x02)  //歌曲单次循环（单曲播放）
#define LISTER_LIST_REPEAT_MODE (0x03)  //列表周期循环（循环播放）
#define LISTER_SONG_REPEAT_MODE (0x04)  //歌曲周期循环（单曲循环）

/* 列表播放器，停止/暂停/恢复 */
void list_player_stop(void);
void list_player_pause(void); // same as list_player_suspend().
void list_player_suspend(void);
void list_player_resume(void);

/* 列表播放器，上一曲/下一曲 */
void list_player_next(void);
void list_player_prev(void);

/* 列表播放器，播放指定曲目 */
int list_player_num(int num);
int list_player_item(music_item_t item);

/* 返回当前的music list */
music_list_t list_player_current_music_list(void);
/* 返回当前的歌曲信息 */
music_item_t list_player_current_item(void);
/* 返回当前播放位置 */
int list_player_current_index(void);
music_list_t list_player_current_table(void);
/* 返回指定位置的歌曲信息 */
music_item_t list_player_item_get(int index);
/* 返回指定歌曲的位置 */
int list_player_index_get(music_item_t item);

int list_player_is_exist(void);

/* 创建歌单 */
music_list_t list_player_items_create(void);
/* 删除歌单 */
void list_player_items_delete(music_list_t table);

int list_player_item_clear_curr_info(music_list_t table);
/* 添加歌曲 */
int list_player_item_add(music_list_t table, music_item_t item, int index);
/* 删除歌曲 */
int list_player_item_del(music_list_t table, struct music_item *item);
/* 根据index删除歌曲 */
int list_player_item_del_by_index(music_list_t table, int index);

/* dump list info */
int list_player_dump(music_list_t table);

void list_player_lock(void);
void list_player_unlock(void);
void list_player_take(void);    /* 保证list_player独占 */
void list_player_release(void);

#ifdef __cplusplus
}
#endif

#endif
