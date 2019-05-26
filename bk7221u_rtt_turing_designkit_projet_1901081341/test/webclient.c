#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>

#include "webclient.h"

#define WEBCLIENT_READ_BUFFER (1024 * 4)

int webclient_test(int argc, char **argv)
{
    struct webclient_session* session = RT_NULL;
    unsigned char *buffer = RT_NULL;
    int ret = 0;

    if(argc != 2)
    {
        rt_kprintf("input argc is err!\n");
        ret =  -RT_ERROR;
        goto _exit;
    }

    buffer = rt_malloc(WEBCLIENT_READ_BUFFER);
    if(!buffer)
    {
        rt_kprintf("have no memory for buffer\n");
        ret =  -RT_ERROR;
        goto _exit;
    }

    session = webclient_open(argv[1]);
    if(!session)
    {
        rt_kprintf("open failed, url:%s\n", argv[1]);
        ret =  -RT_ERROR;
        goto _exit;
    }
    rt_kprintf("response : %d, content_length : %d\n",session->response, session->content_length);

    int rc = 0;
    int nw = WEBCLIENT_READ_BUFFER;
    
    int content_pos = 0;
    int len = 0, i = 0;
    int content_length = session->content_length;
    do
    {   
        int page_pos = 0;
        do
        {
            memset(buffer + page_pos, 0x00, nw - page_pos);
            rc = webclient_read(session, buffer + page_pos, nw - page_pos);
            if(rc < 0)
            {
                rt_kprintf("webclient read err ret : %d\n", rc);
                break;
            }

            if(rc == 0)
                break;

            page_pos += rc;
            
        }while(page_pos < nw);
        
        len = page_pos;
		for(i = 0; i<len; i++)
			 rt_kprintf("%c", buffer[i]);
        
        content_pos += page_pos;   
    }while(content_pos < content_length);
    rt_kprintf("content pos : %d, content_length : %d\n", content_pos, session->content_length); 

    if(session)
        webclient_close(session);
_exit:    

    if(buffer)
        rt_free(buffer);

    return ret;
}
MSH_CMD_EXPORT(webclient_test,webclient open URI test);
