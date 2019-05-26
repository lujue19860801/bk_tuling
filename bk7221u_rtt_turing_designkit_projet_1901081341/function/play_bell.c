#include <rtthread.h>
#include <rthw.h>

#include "player.h"
#include "play_bell.h"
#include "beken_util.h"
#include <player_app.h>
#include "list_player.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[bell]"
#define DBG_LEVEL         DBG_LOG
// #define DBG_LEVEL         DBG_INFO
// #define DBG_LEVEL         DBG_WARNING
// #define DBG_LEVEL         DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#define TIP_PATH_DEFAULT             "/"
#define TIP_PATH_BELL                "/flash0/"

static const char *bell_path = TIP_PATH_DEFAULT;

static music_list_t bell_list;

/* 超时时间30s */
#define BELL_TIMEOUT_CHECK rt_tick_from_millisecond(15 * 1000)

struct bell_player
{
    music_list_t table;
    rt_sem_t end_sem;
    struct rt_mutex lock;
};
static struct bell_player g_bell;

static rt_bool_t player_is_play(void)
{
    rt_bool_t result = (player_get_state() == PLAYER_STAT_PLAYING) ? RT_TRUE : RT_FALSE;

    return result;
}

static rt_bool_t player_is_playing(void)
{
    rt_bool_t result = player_is_play() ? RT_TRUE : RT_FALSE;

    return result;
}

static rt_bool_t player_is_stop(void)
{
    rt_bool_t result = player_get_state() == PLAYER_STAT_STOPPED ? RT_TRUE : RT_FALSE;

    return result;
}

static rt_bool_t player_is_pause(void)
{
    rt_bool_t result = player_get_state() == PLAYER_STAT_PAUSED ? RT_TRUE : RT_FALSE;

    return result;
}

static int player_stop_playing(void)
{
    player_app_stop();
    return 0;
}

/* 创建提示音播放列表 */
static int bell_list_items_create(void)
{
    list_player_detach_items();
    if (g_bell.table)
    {
        list_player_items_delete(g_bell.table);
        g_bell.table = NULL;
    }

    dbg_log(DBG_INFO, "tick = %d, create bell table \n", rt_tick_get());
    g_bell.table = list_player_items_create();
}

/* 删除提示音播放列表 */
static void bell_list_items_delete(void)
{
    music_list_t table = RT_NULL;
    table = list_player_detach_items();
    if (table && g_bell.table)
    {
        dbg_log(DBG_INFO, "tick = %d, delete bell table \n", rt_tick_get());
        list_player_items_delete(g_bell.table);
        g_bell.table = NULL;
    }
}

/* 回调处理函数 */
static void bell_list_event_handle(music_list_t table, int event, void *arg)
{
    /* 播放结束释放信号量 */
    dbg_log(DBG_LOG, "tick = %d, bell_list_event_handle, event = %d \n", rt_tick_get(), event);
    if (event == LISTER_END_OF_ITEM)
    {
        dbg_log(DBG_LOG, "tick = %d, release end sem \n", rt_tick_get());
        rt_sem_release(g_bell.end_sem);
    }
}

void player_bell_change_to_flash0(void)
{
    bell_path = TIP_PATH_BELL;
    rt_kprintf("bellpath change toflash0\r\n");
}
char *get_bell_path(void)
{
	return bell_path;
}
rt_err_t play_bell(const char *bell)
{
    int result = RT_EOK;
    struct music_item item;
    char *dynamic_bell_path = 0;

    if (!bell)
    {
        return -RT_ERROR;
    }

    /* 创建表 */
    bell_list_items_create();

    dynamic_bell_path = dynamic_string_append(0, "%s%s.mp3", bell_path, bell);
    item.URL = dynamic_bell_path;
    item.name = dynamic_bell_path;
    dbg_log(DBG_LOG, "tick = %d, add bell url = %s, name = %s \n", rt_tick_get(), item.URL, item.name);
    list_player_item_add(g_bell.table, &item, -1);
    list_player_set_table_handler(g_bell.table, bell_list_event_handle, RT_NULL);
    dbg_log(DBG_LOG, "tick = %d, start play bell url = %s, \n", rt_tick_get(), item.URL);
    list_player_play(g_bell.table);
    dynamic_string_free(dynamic_bell_path);

    /* 等待播放完成 */
    result = rt_sem_take(g_bell.end_sem, BELL_TIMEOUT_CHECK);
    if (result != RT_EOK)
    {
        dbg_log(DBG_ERROR, "tick = %d, wait sem error, ret = %d \n", rt_tick_get(), result);
    }
    dbg_log(DBG_LOG, "tick = %d, end bell play \n", rt_tick_get());

    /* 删除表 */
    bell_list_items_delete();
    
    // to this the caputure tip ready play end
    if(strcmp("wozai",bell) == 0 || strcmp("wechat",bell) ==0 )
		tc_start_prompt_tone_over();

    return result;
}

rt_err_t play_bell_asynchronous(const char *bell)
{
    return -RT_ENOMEM;
}

rt_err_t play_bell_recover(const char *bell)
{
    music_list_t table = RT_NULL;

    dbg_log(DBG_LOG, "tick = %d, try list_player_take, thread = %s \n", rt_tick_get(), rt_thread_self()->name);
    list_player_take();
	
    dbg_log(DBG_LOG, "tick = %d, try take bell.lock, thread = %s \n", rt_tick_get(), rt_thread_self()->name);
    rt_mutex_take(&(g_bell.lock), RT_WAITING_FOREVER);
    dbg_log(DBG_LOG, "tick = %d, lock, thread = %s \n", rt_tick_get(), rt_thread_self()->name);

    table = list_player_detach_items();
    play_bell(bell);
    if (table)
    {
        list_player_play(table);
    }
    rt_mutex_release(&(g_bell.lock));

    dbg_log(DBG_LOG, "tick = %d, release lock, thread = %s \n", rt_tick_get(), rt_thread_self()->name);
    list_player_release();

    return -RT_ENOMEM;
}

static int play_bell_init(void)
{
    g_bell.end_sem = rt_sem_create("bell_sem", 0, RT_IPC_FLAG_FIFO);
    rt_mutex_init(&(g_bell.lock), "bell_lock", RT_IPC_FLAG_FIFO);
    return 0;
}

INIT_ENV_EXPORT(play_bell_init);

static int play_bell_msh(int argc, char *argv[])
{
    char *url = rt_strdup(argv[1]);

    rt_kprintf("[play_bell]: L%d url = %s \n", __LINE__, url);
    play_bell(url);
    if (url)
        rt_free(url);
}
FINSH_FUNCTION_EXPORT_ALIAS(play_bell_msh, __cmd_play_bell, play bell);

static int play_bell_asynchronous_msh(int argc, char *argv[])
{
    char *url = rt_strdup(argv[1]);

    rt_kprintf("[play_bell]: L%d url = %s \n", __LINE__, url);
    play_bell_asynchronous(url);
    if (url)
        rt_free(url);
}
FINSH_FUNCTION_EXPORT_ALIAS(play_bell_asynchronous_msh, __cmd_play_bell_asynchronous, play bell asynchronous);

static int play_bell_recover_msh(int argc, char *argv[])
{
    char *url = rt_strdup(argv[1]);

    rt_kprintf("[play_bell]: L%d url = %s \n", __LINE__, url);
    play_bell_recover(url);
    if (url)
        rt_free(url);
}
FINSH_FUNCTION_EXPORT_ALIAS(play_bell_recover_msh, __cmd_play_bell_recover, play bell recover);
