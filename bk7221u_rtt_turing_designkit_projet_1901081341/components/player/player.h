#ifndef PLAYER_H__
#define PLAYER_H__

#include "player_config.h"
#include "player_system.h"

#define PLAYER_VERSION          1L
#define PLAYER_SUBVERSION       0L
#define PLAYER_REVISION         0L

#define PLAYER_PRIORITY                   2
#define PLAYER_STACKSZ                    (4 * 1024)
#define PLAYER_WORKQUEUE_STACKSZ          (2 * 1024)

enum PLAYER_STAT
{
    PLAYER_STAT_STOPPED,
    PLAYER_STAT_PLAYING,
    PLAYER_STAT_PAUSED,
};

enum PLAYER_RESULT_CODE
{
    PLAYER_OK,
    PLAYER_FAILED,
};

enum PLAYER_RESULT
{
    PLAYER_CONTINUE,
    PLAYER_BREAKOUT,
    PLAYER_DO_PLAY,
    PLAYER_DO_SEEK,
};

enum PLAYER_EVENT
{
	PLAYER_AUDIO_PLAYBACK,
	PLAYER_AUDIO_CLOSED,

    PLYAER_STATE_CHANGED,
    PLAYER_VOLUME_CHANGED,

    PLAYER_APP_SUSPENDED,
    PLAYER_APP_RESUMED,
};

struct rt_work;
struct webclient_session;

/* play/stop/pause function, return PLAYER_RESULT_CODE */
int player_play(void);
int player_play_position(int position);
int player_play_websession(struct webclient_session* session);

int player_stop(void);
int player_pause(void);

void player_do_work(struct rt_work* work);

/* do seek */
int player_do_seek(int position);
/* fetch seek position and reset seek status */
int player_fetch_seek(void);

int player_set_uri(const char *uri);
const char* player_get_uri(void);

int player_get_state(void);
const char* player_get_state_str(void);
int player_get_duration(void);
int player_get_position(void);

/* API for codec */
/* handle player event, returns PLAYER_RESULT */
int player_event_handle(int timeout);

/* set duration/position in the music */
int player_set_duration(int duration);
int player_set_position(int position);

int player_init(void);

#endif
