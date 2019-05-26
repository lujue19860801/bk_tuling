#include "offline_player.h"
#include "rtthread.h"
#include "include.h"
#include <finsh.h>
#include <dfs_posix.h>
#include <player_app.h>
#include <player.h>
#include <list_player.h>

#include <player_system.h>

#include "key_app.h"

#include "cchip_play_tip.h"
#include "cchip_led_control.h"
#include "mode_mgmt.h"

enum{
	OFFLINE_PLAYER_PLAY = (0x1 << 0),
	OFFLINE_PLAYER_PAUSE = (0x1 << 1),
	
	OFFLINE_PLAYER_SONG_NEXT = (0x1 << 2),
	OFFLINE_PLAYER_SONG_PREV = (0x1 << 3),
	OFFLINE_PLAYER_VOLUME_ADD = (0x1 << 4),
	OFFLINE_PLAYER_VOLUME_REDUCE = (0x1 << 5),

	OFFLINE_PLAYER_SD_INSTER_EVENT = (0x1 << 6),
	OFFLINE_PLAYER_SD_REMOVE_EVENT = (0x1 << 7),
	OFFLINE_MODE_CHANGE = (0x01 << 8),
	
	OFFLINE_PLAYER_LINEIN_INSTER_EVENT = (0x1 << 9),
	OFFLINE_PLAYER_LINEIN_REMOVE_EVENT = (0x1 << 10),

	OFFLINE_PLAYER_CHANGE_DIR = (0x1 << 11),
    
	OFFLINE_PLAYER_ALL_EVENT = 0x0FFF,
};

#define	MAX_SONG_DIR	    64       // 128
#define	MAX_DIR_LEVEL	    8
#define MAX_LFN_LEN	        256

typedef enum {
	GET_PREV,
	GET_NEXT
}PLAY_DIRECTION;

typedef struct SONG_DIR_
{
	char *path;
	unsigned short dir_music_num;
}SONG_DIR;

typedef struct PLAY_INFO_
{
	SONG_DIR dir_info[MAX_SONG_DIR];
	char cur_file_path[MAX_LFN_LEN];
	
	unsigned short music_num;
	unsigned short music_index;
	unsigned short dir_num;
	unsigned short dir_index;
	
	unsigned char init_done : 1;
	unsigned char player_status : 1;
	unsigned char player_pause_flag : 1;
	unsigned char vol_step : 5;
	unsigned char vol;
}PLAY_INFO;

PLAY_INFO _offline_player = {0};
static 	struct rt_event offplay_env;
static music_list_t offline_music_list = NULL;

void offline_list_items_delete(void);

/* 创建播放列表 */
void offline_list_items_create(void)
{
    if (offline_music_list)
    {
        offline_list_items_delete();
    }

    offline_music_list = list_player_items_create();
}

/* 删除播放列表 */
void offline_list_items_delete(void)
{
	music_list_t cur_table;
	
	if(offline_music_list == NULL) 
		return;
	cur_table = list_player_current_music_list();
	if(cur_table == offline_music_list)
	{
    	list_player_detach_items();
	}
	
    list_player_items_delete(offline_music_list);
    offline_music_list = NULL;
	_offline_player.player_status = 0;
	_offline_player.player_pause_flag = 0;
    OLP_PRINTF(" <%s:%d> list_player_items_delete\r\n", __func__, __LINE__);
}

/* 挂起播放 */
int offline_list_suspend(void)
{
	music_list_t cur_table = list_player_current_music_list();

	if((offline_music_list) && (cur_table == offline_music_list))
	{
		cur_table = list_player_detach_items();
	}
   	
    return RT_EOK;
}

int offline_list_resume(void)
{
	music_list_t cur_table = list_player_current_music_list();

	if((offline_music_list) &&  (cur_table != offline_music_list))
	{
		 int current_volume = player_app_get_volume(NULL);
		 player_app_set_volume(NULL, 0);
		 list_player_detach_items();
		 list_player_play(offline_music_list);
		 OLP_PRINTF("list_player_num before\r\n");
		 player_app_set_volume(NULL, current_volume);
		 return RT_EOK;
	}
	else
    {
        OLP_PRINTF("L%d offline_music_player not implement  \n", __LINE__);
    }
	
	return -RT_ERROR;
}

void add_to_offline_list(char *name)
{
    struct music_item items;
	items.name = strrchr(name,'/');
	items.URL = name;
    list_player_item_add(offline_music_list, &items, -1);
}


/*
idx: song index in current dir ,start from 0
path: current dir full path name
fullname:the searched song's full path name(path name + filename)
*/
static int  read_file_in_dir(unsigned int idx,char *path,char * fullname)
{
	struct dirent  *ent  = NULL;
	DIR *pDir = NULL;
	unsigned short cur_idx = 0;
	int ret = -1;
	
	pDir = opendir(path);
	if(NULL != pDir)
	{
		while(1)
		{
			ent = readdir(pDir);
			if(NULL == ent)
				break;
			if(ent->d_type & DT_REG)
			{
				if( (0 == strncasecmp(strchr(ent->d_name,'.'),".mp3",4)) ||
					(0 == strncasecmp(strchr(ent->d_name,'.'),".wav",4)) )
				{
					if( cur_idx == idx )
					{
						ret = 0;
						snprintf(fullname, MAX_LFN_LEN,"%s%s%s",path,"/",ent->d_name);						
						break;
					}
					cur_idx ++;
				}
			}
		}
		closedir(pDir);
	}
	return ret;
}
	
static int get_song(PLAY_DIRECTION prev_next)
{
	int i = 0;
	int ret = -1;
	unsigned short total = 0;
	if(_offline_player.music_num == 0)
		return ret;
	
	if(GET_PREV == prev_next)
	{//prev
		if(0 == _offline_player.music_index)
			_offline_player.music_index = _offline_player.music_num - 1;
		else
			_offline_player.music_index --;
	}
	else
	{//next
		if(_offline_player.music_index >= _offline_player.music_num - 1)
			_offline_player.music_index = 0;
		else
			_offline_player.music_index ++;
	}

	if(_offline_player.music_index != 0)
	{
		for(i=0;i<sizeof(_offline_player.dir_info)/sizeof(SONG_DIR);i++)
		{
			total += _offline_player.dir_info[i].dir_music_num;
			if(_offline_player.music_index <= total-1)
				break;		
		}
		total -= _offline_player.dir_info[i].dir_music_num;
	}
	memset(_offline_player.cur_file_path,0,sizeof(_offline_player.cur_file_path));
	ret = read_file_in_dir(_offline_player.music_index-total,_offline_player.dir_info[i].path,_offline_player.cur_file_path);
	OLP_PRINTF("@@@@@ full name:%s @@@@@\r\n",_offline_player.cur_file_path);
	
	return ret;
}

static int get_dir(PLAY_DIRECTION prev_next)
{
	int i = 0;
	int ret = -1;
	
	if(_offline_player.dir_num == 0)
		return ret;

	if(GET_PREV == prev_next)
	{
		if(0 == _offline_player.dir_index)
			_offline_player.dir_index = _offline_player.dir_num - 1;
		else
			_offline_player.dir_index --;
	}
	else
	{
		if(_offline_player.dir_index >= _offline_player.dir_num -1)
			_offline_player.dir_index = 0;
		else
			_offline_player.dir_index ++;
	}
	_offline_player.music_index = 0;
	
	for(i=0;i<_offline_player.dir_index;i++)
		_offline_player.music_index +=  _offline_player.dir_info[_offline_player.dir_index].dir_music_num;
	if(_offline_player.music_index != 0)
		_offline_player.music_index -= 1;
	
	memset(_offline_player.cur_file_path,0,sizeof(_offline_player.cur_file_path));
	ret = read_file_in_dir(0,_offline_player.dir_info[_offline_player.dir_index].path,_offline_player.cur_file_path);
	return ret;
}

/*
full disk scan: get mp3 total number and DIRs' pathname which have .mp3 files
*/
void scan_files(char *path,rt_uint8_t recu_level)
{
    struct dirent  *ent  = NULL;
	DIR *pDir = NULL;
	short filecnt = 0;
	SONG_DIR *dir_ptr;
	rt_uint8_t tmp;

	pDir = opendir(path);
	if(NULL == pDir)
	{
		return;
	}
	
	tmp = strlen(path);
	while(1)
	{
		ent = readdir(pDir);
		if(NULL == ent)
			break;

		if( (0 == strcmp(ent->d_name,".")) || (0 == strcmp(ent->d_name,"..")) )
            continue;

		if(ent->d_type & DT_DIR)
		{
			if(recu_level < MAX_DIR_LEVEL)
			{
				snprintf(&path[tmp],strlen(ent->d_name)+1+1,"/%s",ent->d_name);
				recu_level++;
				scan_files(path,recu_level);
				path[tmp] = 0;
			}
			else
				break;
		}
		else
		{
			if((0 == strncasecmp(strchr(ent->d_name,'.'),".mp3",4))||
			   (0 == strncasecmp(strchr(ent->d_name,'.'),".wav",4)))
			{
				filecnt ++;
			}
		}
	}
	
	if(filecnt > 0)
	{
		dir_ptr = &_offline_player.dir_info[_offline_player.dir_num];
		
		dir_ptr->path = rt_malloc(strlen(path) + 1);
		if(RT_NULL == dir_ptr->path)
		{			
			OLP_PRINTF("path_malloc_null\r\n");
			closedir(pDir);
			return;
		}
		
		memset(dir_ptr->path,0,strlen(path)+1);
		snprintf(dir_ptr->path,strlen(path)+1,"%s",path);		
		dir_ptr->dir_music_num = filecnt;
		
		_offline_player.music_num += filecnt;
		_offline_player.dir_num++;
	}
	
	closedir(pDir);
}


void play_prev_next_song(PLAY_DIRECTION PrevNext)
{
	music_list_t cur_table = list_player_current_music_list();
	
	if((offline_music_list)&& (cur_table == offline_music_list))
	{
		list_player_stop();
		get_song(PrevNext);
		list_player_item_del_by_index(offline_music_list,-1);
		add_to_offline_list(_offline_player.cur_file_path);
		list_player_play(offline_music_list);
	}
}

void play_prev_next_dir(PLAY_DIRECTION PrevNext)
{
	music_list_t cur_table = list_player_current_music_list();
	
	if((offline_music_list)&& (cur_table == offline_music_list))
	{
		list_player_stop();
		get_dir(PrevNext);
		list_player_item_del_by_index(offline_music_list,-1);
		add_to_offline_list(_offline_player.cur_file_path);
		list_player_play(offline_music_list);
	}
}

int scan_sd(void)
{
	int i;
	char *path = rt_calloc(1,256);

	if(NULL == path)
	{
		OLP_PRINTF("scan malloc error\r\n");
		return -1;
	}
	
	snprintf(path,strlen("/sd")+1,"%s","/sd");
	scan_files(path,0);
	for(i=0;i<_offline_player.dir_num;i++)
	{
		OLP_PRINTF("====path[%d]:%s,song num:%d===\r\n",i,_offline_player.dir_info[i].path,_offline_player.dir_info[i].dir_music_num);
	}
	
	rt_free(path);
	
	if(_offline_player.dir_num > 0)
		return 0;
	else
		return -1;
}

uint32_t mount_sd(void)
{	
	uint32_t ret;
	
	/* mount sd card fat partition 1 as root directory */
	if(dfs_mount("sd0", "/sd", "elm", 0, 0) == 0)
	{
		OLP_PRINTF("SD Card initialized!\n");
		ret = 0;
	}
	else
	{
		OLP_PRINTF("SD Card initialzation failed!\n");
		ret = 1;
	}

	return ret;
}

void unmount_sd(void)
{
	int ret;
	ret = dfs_unmount("/sd");
	OLP_PRINTF("unmount sd :ret =%d\r\n",ret);
}

static void sd_list_event_handle(music_list_t table, int event, void *arg)
{
	OLP_PRINTF("======sd play event:%x======\r\n",event);
	if(sd_is_online())
	{
		if(LISTER_END_OF_LIST == event)
		{
			offline_player_send_event(OFFLINE_PLAYER_SONG_NEXT);
		}
	}
}

void sd_play_init(void)
{		
	OLP_PRINTF("sd_play_init\n");

	memset(&_offline_player.dir_info,0,sizeof(SONG_DIR));
	memset(&_offline_player.cur_file_path,0,MAX_LFN_LEN);
	_offline_player.music_num = 0;
	_offline_player.music_index = 0;
	_offline_player.dir_num = 0;
	_offline_player.dir_index = 0;
	
	if(0 == scan_sd())
	{
		list_player_detach_items();
		offline_list_items_create();
		if(NULL != offline_music_list)
		{
			list_player_set_table_handler(offline_music_list, sd_list_event_handle, RT_NULL);
		    list_player_mode_set(LISTER_LIST_ONCE_MODE);
			//get the first song
			_offline_player.music_index = _offline_player.music_num -1;
			get_song(GET_NEXT);
			add_to_offline_list(_offline_player.cur_file_path);
			change_led_state(LED_STATE_MUSIC);
			list_player_play(offline_music_list);
			offline_player_set_volume(offline_player_get_volume());

			_offline_player.player_status = 1;
			_offline_player.player_pause_flag = 1;
		}
		
	}
}

static void sd_play_uninit(void)
{
	int i;
	
	OLP_PRINTF("sd_play_uninit\n");
	offline_list_items_delete();
	
	for(i=0;i< sizeof(_offline_player.dir_info)/sizeof(SONG_DIR);i++)
	{
		if(_offline_player.dir_info[i].path != NULL)
		{			
			OLP_PRINTF("path-%d:0x%x\n", i, _offline_player.dir_info[i].path);
			rt_free(_offline_player.dir_info[i].path);
			_offline_player.dir_info[i].path = 0;
		}
	}
	_offline_player.player_status = 0;
	_offline_player.player_pause_flag = 0;
}

int offline_player_items_dump(void)
{
    music_item_t item;
	music_list_t cur_table;

    if (!offline_music_list)
    {
        OLP_PRINTF("offline_music_list not initialize \n");
        return -RT_ERROR;
    }
	cur_table = list_player_current_music_list();

    OLP_PRINTF("[offline_music] music table dump:\n");
	if(cur_table == offline_music_list)
    	OLP_PRINTF("play status: playing\n");
	else
		OLP_PRINTF("play status: stop\n");
    OLP_PRINTF("item_num: %d\n", offline_music_list->item_num);
    OLP_PRINTF("item_position: %d\n", offline_music_list->item_position);

	///dump music list
	if(cur_table == offline_music_list)
	{
    	OLP_PRINTF("current_index: %d\n", list_player_current_index());
		item = list_player_current_item();
	    if (item)
	    {
	        OLP_PRINTF("current_name: %s\n", item->name);
	        OLP_PRINTF("current_url: %s\n", item->URL);
	    }
	    OLP_PRINTF("\n");
	}
	OLP_PRINTF("/*---------------------------------\n");
	list_player_dump(offline_music_list);
	OLP_PRINTF("---------------------------------*/\n");
    return RT_EOK;
}


int offline_player_set_volume(int volume)
{
	if(volume > 100)
		volume = 100;
	else if(volume < 0)
		volume = 0;
    player_app_set_volume(player_app_get_activated(), volume);
	return RT_EOK;
}

int offline_player_get_volume(void)
{
	int volume = player_app_get_volume(player_app_get_activated());
	
	return volume;
}

///循环....
int offline_player_mode(int mode)
{
    switch(mode)
	{
	case LISTER_LIST_ONCE_MODE	:
		#if OFFLINE_PLAYER_DEBUG
	    OLP_PRINTF("LISTER_LIST_ONCE_MODE\n");
		#endif
		break;
	case LISTER_SONG_ONCE_MODE  :
		#if OFFLINE_PLAYER_DEBUG
	    OLP_PRINTF("LISTER_SONG_ONCE_MODE\n");
		#endif
		break;
	case LISTER_LIST_REPEAT_MODE:
		#if OFFLINE_PLAYER_DEBUG
	    OLP_PRINTF("LISTER_LIST_REPEAT_MODE\n");
		#endif
		break;
	case LISTER_SONG_REPEAT_MODE:
		#if OFFLINE_PLAYER_DEBUG
	    OLP_PRINTF("LISTER_SONG_REPEAT_MODE\n");
		#endif
	   	break;
	default:
		#if OFFLINE_PLAYER_DEBUG
	    OLP_PRINTF("Undefine offline player mode\n");
		#endif
		return -RT_ERROR;
		///break;
	}
	
	list_player_mode_set(mode);
    return RT_EOK;
}

int offline_player_send_event(int event)
{
	OLP_PRINTF("----offline player send event :%x----\r\n",_offline_player.init_done);
	if(_offline_player.init_done)
	{
		rt_event_send(&offplay_env,event);
	}
	return RT_EOK;
}

void offline_player_key_button_event_handler(OfflinePlayerKeyHandleEvent event)
{
	switch(event)
	{
		case OFFLINE_PLAYER_KEYHANDLE_PLAY_PAUSE:  ///playe/pause
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
		   	    OLP_PRINTF("%s:%d count=%d\n", __FUNCTION__, __LINE__,_offline_player.player_pause_flag);
			#endif
		    if (_offline_player.player_pause_flag & 0x1)
		    {
		        #if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
		        	OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_PAUSE\n");
				#endif
				offline_player_send_event(OFFLINE_PLAYER_PAUSE);
				_offline_player.player_pause_flag = 0;
		    }
		    else
		    {
		        #if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
		       		OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_PLAY\n");
			    #endif
				offline_player_send_event(OFFLINE_PLAYER_PLAY);
				_offline_player.player_pause_flag = 1;
		    }
			break;
		case OFFLINE_PLAYER_KEYHANDLE_MODE_CHANGE:
			OLP_PRINTF("---key mode change--d\r\n");
			offline_player_send_event(OFFLINE_MODE_CHANGE);
			break;
		case OFFLINE_PLAYER_KEYHANDLE_SONG_NEXT:   ///next song
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_SONG_NEXT\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_SONG_NEXT);
			break;
		case OFFLINE_PLAYER_KEYHANDLE_SONG_PREV:  ////prev song
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_SONG_PREV\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_SONG_PREV);
			break;
		case OFFLINE_PLAYER_KEYHANDLE_CHANGE_DIR:
			offline_player_send_event(OFFLINE_PLAYER_CHANGE_DIR);
			break;
		case OFFLINE_PLAYER_KEYHANDLE_VOLUME_ADD:  ///volume add
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_VOLUME_ADD\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_VOLUME_ADD);
			break;
		case OFFLINE_PLAYER_KEYHANDLE_VOLUME_REDUCE: ///volume re
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_VOLUME_REDUCE\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_VOLUME_REDUCE);
			break;
		default:
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[BT_EVT]OFFLINE_PLAYER_KEYHANDLE_UNDEFINE\n");
			#endif
			break;
	}
}

void offline_player_sd_status_change_event(int event)
{
	switch(event)
	{
		case OFFLINE_PLAYER_SD_STATUS_INSTER:
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[SD_EVT]OFFLINE_PLAYER_SD_STATUS_INSTER\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_SD_INSTER_EVENT);
			break;
		case OFFLINE_PLAYER_SD_STATUS_REMOVE:
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				OLP_PRINTF("[SD_EVT]OFFLINE_PLAYER_SD_STATUS_REMOVE\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_SD_REMOVE_EVENT);
			break;

        case OFFLINE_PLAYER_LINEIN_STATUS_INSTER:
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				rt_kprintf("[LINEIN_EVT]OFFLINE_PLAYER_LINEIN_INSTER_EVENT\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_LINEIN_INSTER_EVENT);
            break;
        case OFFLINE_PLAYER_LINEIN_STATUS_REMOVE:
			#if OFFLINE_PLAYER_KEY_BUTTON_DEBUG
				rt_kprintf("[LINEIN_EVT]OFFLINE_PLAYER_LINEIN_STATUS_REMOVE\n");
			#endif
			offline_player_send_event(OFFLINE_PLAYER_LINEIN_REMOVE_EVENT);
            break;
            
	}
}

#if CONFIG_SOUND_MIXER
extern void mixer_pause(void);
extern void mixer_replay(void);
extern void sdly_set_micphone_vol(int vol);
#endif

int offline_msg_handler(int event)
{
	int ret;

	if(_offline_player.player_status)
	{
		if(event & OFFLINE_PLAYER_PAUSE)
		{
			list_player_pause();
		}
		if(event & OFFLINE_PLAYER_PLAY)
		{
			list_player_resume();
		}
		if(event &  OFFLINE_PLAYER_SONG_NEXT)
		{
			play_prev_next_song(GET_NEXT);
		}
		
		if(event & OFFLINE_PLAYER_SONG_PREV)
		{
			play_prev_next_song(GET_PREV);
		}
		
		if(event & OFFLINE_PLAYER_VOLUME_ADD)
		{
			offline_player_set_volume(offline_player_get_volume() + _offline_player.vol_step);
		}
		
		if(event & OFFLINE_PLAYER_VOLUME_REDUCE)
		{
			offline_player_set_volume(offline_player_get_volume() - _offline_player.vol_step);
		}

		if(event & OFFLINE_PLAYER_CHANGE_DIR)
		{
			play_prev_next_dir(GET_NEXT);
		}
	}

	if((event & OFFLINE_MODE_CHANGE) && sd_is_online())
	{
		if(mmgmt_is_cloud_mode()||mmgmt_is_flash_mode())
		{
			OLP_PRINTF("---enter offlineplay mode---\r\n");
			mmgmt_set_mode(MODE_OFFLINE_PLAYER);
			send_play_tip_event(TURING_PLAYTIP_TFCARD_MODE);
			sd_play_init();
		}
		else
		{
			OLP_PRINTF("---exit offlineplay mode---\r\n");
			list_player_stop();
			send_keymode_2_turing();//switch to turing mode
		}
	}

	if(event & OFFLINE_PLAYER_SD_INSTER_EVENT)
	{
		uint32_t ret;
		
		OLP_PRINTF("---sd mount---\r\n");
		ret = mount_sd();
		if(0 == ret)
		{
			mmgmt_determine_initial_offline_mode();
		}
	}

	if( event & OFFLINE_PLAYER_SD_REMOVE_EVENT )
	{
		OLP_PRINTF("---sd unmount---\r\n");
		if(mmgmt_is_offline_mode())
		{
			list_player_stop();
			sd_play_uninit();
			if(!mmgmt_swith_to_flash_mode())
				send_keymode_2_turing();//switch to turing mode
		}
		unmount_sd();
	}

    if(event & OFFLINE_PLAYER_LINEIN_INSTER_EVENT)
	{
		uint32_t ret;
		
		rt_kprintf("---linein inster---\r\n");
        
#if CONFIG_SOUND_MIXER
        //mixer_replay();
        sdly_set_micphone_vol(50);
#endif
	}
    
    if(event & OFFLINE_PLAYER_LINEIN_REMOVE_EVENT)
	{
		uint32_t ret;
		
		rt_kprintf("---linein remove---\r\n");
        
#if CONFIG_SOUND_MIXER
        sdly_set_micphone_vol(0);
#endif
	}
    
	return RT_EOK;
}

void offline_player_main_entry(void *parameter)
{
	uint32_t recv_evt;
	int ret;

	rt_thread_delay(500);

    while (1)
    {
    	ret = rt_event_recv(&offplay_env,OFFLINE_PLAYER_ALL_EVENT,
							RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
							RT_WAITING_FOREVER, &recv_evt);

		if(ret == RT_EOK)
		{
			OLP_PRINTF("recv_evt:%x\n",recv_evt);
			offline_msg_handler(recv_evt);
		}
		else
		{
			OLP_PRINTF("recv_evt timeout\n");
		}
    }
}

int offline_player_init(void)
{
    rt_thread_t tid = RT_NULL;
	
	memset(&_offline_player,0,sizeof(_offline_player));
	rt_event_init(&offplay_env, "env", RT_IPC_FLAG_FIFO);
	_offline_player.vol_step = OFFLINE_VOLUME_DEFAULT_STEP;

    tid = rt_thread_create("offline",
                           offline_player_main_entry,
                           RT_NULL,
                           1024 * 4,
                           14,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
	
	_offline_player.init_done = 1;
	return RT_EOK;
}
