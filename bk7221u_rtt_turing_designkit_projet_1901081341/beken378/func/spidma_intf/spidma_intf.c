#include "include.h"
#include "arm_arch.h"

#if CFG_USE_SPIDMA
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_pub.h"
#include "error.h"
#include "fake_clock_pub.h"
#include "co_list.h"

#include "spidma_pub.h"
#include "spidma_intf.h"
#include "spidma_intf_pub.h"

#include "drv_model_pub.h"
#include "mem_pub.h"

#include "app_lwip_tcp.h"
#include "app_lwip_udp.h"

#if CFG_GENERAL_DMA
#include "general_dma_pub.h"
#endif

#define SPIDMA_INTF_DEBUG        1
#include "uart_pub.h"
#if SPIDMA_INTF_DEBUG
#define SPIDMA_INTF_PRT           os_printf
#define SPIDMA_INTF_WPRT          warning_prf
#define SPIDMA_INTF_FATAL         fatal_prf
#else
#define SPIDMA_INTF_PRT           null_prf
#define SPIDMA_INTF_WPRT          null_prf
#define SPIDMA_INTF_FATAL         null_prf
#endif

#define SPIDMA_DROP_DATA_NONODE     1

#define RX_TIMEOUT_30US             30
#define RX_TIMEOUT_500US            500
#define FPGA_MAIN_CLK               120
#define SPIDMA_RXDATA_TIMEOUT       (FPGA_MAIN_CLK * RX_TIMEOUT_500US)

#define SPIDMA_RXNODE_SIZE_UDP      1472
#define SPIDMA_RXNODE_SIZE_TCP      1460
#ifndef SPIDMA_RXNODE_SIZE
#define SPIDMA_RXNODE_SIZE          SPIDMA_RXNODE_SIZE_UDP
#endif

#define SPIDMA_DROP_DATA_FLAG       0x01

#ifndef SPIDMA_RXDATA_TIMEOUT
#define SPIDMA_RXDATA_TIMEOUT       SPIDMA_DEF_RXDATA_TIMEOUT_VAL
#endif

#if SPIDMA_DROP_DATA_NONODE
#define SPIDATA_POOL_LEN            (SPIDMA_RXNODE_SIZE * 38)  // 54KB
#else
#define SPIDATA_POOL_LEN            (SPIDMA_RXNODE_SIZE * 5)  // 7KB
#endif

#define SPIDMA_RXBUF_LEN            (SPIDMA_DEF_RXDATA_THRE_INT * 2)

#if ((SPIDMA_RXBUF_LEN != 1024)  && (SPIDMA_RXBUF_LEN != 2048)  &&  \
     (SPIDMA_RXBUF_LEN != 4096)  && (SPIDMA_RXBUF_LEN != 8192)  &&  \
     (SPIDMA_RXBUF_LEN != 16384) && (SPIDMA_RXBUF_LEN != 32768) &&  \
     (SPIDMA_RXBUF_LEN != 65536) )
#error "SPIDMA_RXBUF_LEN should be 1024/2048/4096/8192/16384/32768/65536."
#endif

typedef struct temp_message 
{
	u32 temp_msg;
}SPIDMA_MSG_T;

enum
{
	SPIDMA_INT_POLL          = 0, 
	SPIDMA_EXIT,
};

#define SPIDMA_QITEM_COUNT      (30)
#define SPIDMA_TIMER_INTVAL     (1)

SPIDMA_DESC_ST spidma_intf;
volatile DD_HANDLE spidma_hdl = DD_HANDLE_UNVALID;
xTaskHandle  spidma_thread_hdl = NULL;
beken_queue_t spidma_msg_que = NULL;

typedef struct spidma_elem_st
{
    struct co_list_hdr hdr;
    void *buf_start;
    UINT32 buf_len;
} SPIDMA_ELEM_ST, *SPIDMA_ELEM_PTR;

typedef struct spidma_pool_st
{
    UINT8* rx_buf;
    UINT8* pool;
    SPIDMA_ELEM_ST elem[SPIDATA_POOL_LEN / SPIDMA_RXNODE_SIZE];
    struct co_list free;
    struct co_list ready; 
    #if SPIDMA_DROP_DATA_NONODE
    struct co_list receiving;
    volatile UINT32 drop_pkt_flag;    
    #endif
    int (*send_func)(UINT8 *data, UINT32 len);
} SPIDMA_POOL_ST, *SPIDMA_POOL_PTR;

//SPIDMA_POOL_ST spidma_pool;
SPIDMA_POOL_ST *spidma_pool = NULL;

static void spidma_intfer_pool_deinit(void)
{
    if(!spidma_pool)
        return;
    
    if(spidma_pool->rx_buf){
        os_free(spidma_pool->rx_buf);
        spidma_pool->rx_buf = NULL;
    }
    
    if(spidma_pool->pool){
        os_free(spidma_pool->pool);
        spidma_pool->pool = NULL;
    }
        
    os_free(spidma_pool);
    spidma_pool = NULL;
}

static UINT32 spidma_intfer_pool_init(void* data)
{
    UINT32 i = 0, ret = 0;
    SPIDMA_SND_TYPE snd_type = (SPIDMA_SND_TYPE)((int)data);

    spidma_pool = os_malloc(sizeof(SPIDMA_POOL_ST));
    if(!spidma_pool) 
    {
        SPIDMA_INTF_FATAL("malloc spidma_pool failed\r\n");
        ret = 1;
        goto pool_exit;
    }

    spidma_pool->rx_buf = os_malloc(SPIDMA_RXBUF_LEN * sizeof(UINT8));
    if(!spidma_pool->rx_buf) {
        SPIDMA_INTF_FATAL("malloc spidma rxbuf failed\r\n");
        ret = 2;
        goto pool_exit;
    }

    spidma_pool->pool = os_malloc(SPIDATA_POOL_LEN * sizeof(UINT8));
    if(!spidma_pool->pool) {
        SPIDMA_INTF_FATAL("malloc spidma pool failed\r\n");
        ret = 3;
        goto pool_exit;
    }
    
    os_memset(spidma_pool->pool, 0, sizeof(UINT8)*SPIDATA_POOL_LEN);
    co_list_init(&(spidma_pool->free));
    co_list_init(&(spidma_pool->ready));
    #if SPIDMA_DROP_DATA_NONODE
    co_list_init(&(spidma_pool->receiving));    
    spidma_pool->drop_pkt_flag = 0;
    #endif
    
    for(i = 0; i < (SPIDATA_POOL_LEN / SPIDMA_RXNODE_SIZE); i++)
    {
        spidma_pool->elem[i].buf_start =
            (void *)&(spidma_pool->pool[i * SPIDMA_RXNODE_SIZE]);
        spidma_pool->elem[i].buf_len = 0;

        co_list_push_back(&(spidma_pool->free),
                          (struct co_list_hdr *)&(spidma_pool->elem[i].hdr));
    }

    SPIDMA_INTF_PRT("spidma_intfer send type:%d\r\n", snd_type);

#if CFG_SUPPORT_TIANZHIHENG_DRONE
    if(snd_type == SPIDMA_SND_UDP)
        spidma_pool->send_func = app_lwip_udp_send_packet;
    else if(snd_type == SPIDMA_SND_TCP)
        spidma_pool->send_func = app_lwip_tcp_send_packet; 
    else
#endif
        spidma_pool->send_func = NULL; 

    return ret;
    
pool_exit:
    spidma_intfer_pool_deinit();
        
    return ret;
}

#if CFG_GENERAL_DMA
static void spidma_intfer_config_general_dma(void)
{
    GDMACFG_TPYES_ST cfg;

    cfg.dstdat_width = 32;
    cfg.srcdat_width = 32;
    cfg.dstptr_incr = 1;
    cfg.srcptr_incr = 1;
    cfg.src_start_addr = NULL;
    cfg.dst_start_addr = NULL;

    cfg.channel = GDMA_CHANNEL_1;
    cfg.prio = 0;
    cfg.u.type1.src_loop_start_addr = &(spidma_pool->rx_buf[0]);
    cfg.u.type1.src_loop_end_addr = &(spidma_pool->rx_buf[SPIDMA_RXBUF_LEN]);

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE1, &cfg);
}

void *spidma_memcpy(void *out, const void *in, UINT32 n)
{
    GDMA_DO_ST do_st;
    do_st.channel = GDMA_CHANNEL_1;
    do_st.src_addr = (void *)in;
    do_st.length = n;
    do_st.dst_addr = out;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_ENABLE, &do_st);
    return out;
}
#endif

void spidma_intfer_send_msg(UINT32 new_msg)
{
	OSStatus ret;
	SPIDMA_MSG_T msg;

    if(spidma_msg_que) 
    {
    	msg.temp_msg = new_msg;
    	
    	ret = rtos_push_to_queue(&spidma_msg_que, &msg, BEKEN_NO_WAIT);
    	if(kNoErr != ret)
    	{
    		os_printf("spidma_intfer_send_msg failed\r\n");
    	}
    }
}

void spidma_intfer_exit_thread(void)
{
    spidma_intfer_send_msg(SPIDMA_EXIT);
}

static void spidma_intfer_rx_handler(void *curptr, UINT32 newlen)
{
    SPIDMA_ELEM_PTR elem = NULL;
    do {
        //SPIDMA_INTF_PRT("rx:%d\r\n",newlen);
        if(!newlen)
            break;
        
        #if SPIDMA_DROP_DATA_NONODE
        // drop pkt has happened, so drop it, until spidma timeout handler.
        if(spidma_pool->drop_pkt_flag & SPIDMA_DROP_DATA_FLAG)
            break;
        #endif

        elem = (SPIDMA_ELEM_PTR)co_list_pick(&(spidma_pool->free));
        if(elem) {               
            if(newlen > SPIDMA_RXNODE_SIZE)
                newlen = SPIDMA_RXNODE_SIZE;
            
            #if CFG_GENERAL_DMA
            spidma_memcpy(elem->buf_start, curptr, newlen);
            if(spidma_intf.node_len > newlen){
                UINT32 left = spidma_intf.node_len - newlen;
                os_memset(((UINT8*)elem->buf_start + newlen), 0, left);
            }
            #else
            os_memcpy(elem->buf_start, curptr, newlen);
            #endif
            elem->buf_len = spidma_intf.node_len;
            co_list_pop_front(&(spidma_pool->free));
            #if SPIDMA_DROP_DATA_NONODE
            co_list_push_back(&(spidma_pool->receiving), (struct co_list_hdr *)&elem->hdr);
            #else
            co_list_push_back(&(spidma_pool->ready), (struct co_list_hdr *)&elem->hdr);            
            #endif
        } 
        else {
            #if SPIDMA_DROP_DATA_NONODE
            // not node for receive pkt, drop aready received, and also drop
            // the new come.
            UINT32 cnt_rdy = co_list_cnt(&(spidma_pool->receiving));
            GLOBAL_INT_DECLARATION();
            //SPIDMA_INTF_PRT("no node: rdy:%d\r\n", cnt_rdy);

            GLOBAL_INT_DISABLE(); 
            spidma_pool->drop_pkt_flag |= SPIDMA_DROP_DATA_FLAG;
            GLOBAL_INT_RESTORE();
            
            if(cnt_rdy)
                co_list_concat(&(spidma_pool->free), &(spidma_pool->receiving));
            #endif
        }
    } while(0);
    
    spidma_intfer_send_msg(SPIDMA_INT_POLL);  

}

static void spidma_intfer_rx_timeout_handler(void)
{
    #if SPIDMA_DROP_DATA_NONODE
    // reset drop flag, new pkt can receive
    spidma_pool->drop_pkt_flag &= (~SPIDMA_DROP_DATA_FLAG);
    if(!co_list_is_empty(&(spidma_pool->receiving))) {        
        co_list_concat(&(spidma_pool->ready), &(spidma_pool->receiving));  
    }
    #endif
    
    spidma_intfer_send_msg(SPIDMA_INT_POLL);  

}

static void spidma_intfer_config_desc(void* data)
{
    SPIDMA_SND_TYPE snd_type = (SPIDMA_SND_TYPE)((int)data);
    UINT32 node_len = SPIDMA_RXNODE_SIZE_TCP;

   if(snd_type == SPIDMA_SND_UDP)
        node_len = SPIDMA_RXNODE_SIZE_UDP;
    else if(snd_type == SPIDMA_SND_TCP)
        node_len = SPIDMA_RXNODE_SIZE_TCP; 
    else {
        SPIDMA_INTF_WPRT("Err snd tpye in spidma\r\n"); 
    }
    
    spidma_intf.rxbuf = spidma_pool->rx_buf;
    spidma_intf.txbuf = NULL;
    spidma_intf.rxbuf_len = SPIDMA_RXBUF_LEN;
    spidma_intf.txbuf_len = 0;

    spidma_intf.rx_handler = spidma_intfer_rx_handler;
    spidma_intf.rx_timeout = spidma_intfer_rx_timeout_handler;
    spidma_intf.tx_handler = NULL;

    spidma_intf.mode = ((SPIDMA_DEF_RXDATA_THRE_INT & SPIDMA_DESC_RX_THRED_MASK)
                        << SPIDMA_DESC_RX_THRED_POSI);
    spidma_intf.timeout_val = SPIDMA_RXDATA_TIMEOUT;
    spidma_intf.node_len = node_len;
}

static void spidma_intfer_poll_handler(void)
{
    UINT32 send_len;
    SPIDMA_ELEM_PTR elem = NULL;

    do{
        elem = (SPIDMA_ELEM_PTR)co_list_pick(&(spidma_pool->ready));        
        if(elem) {
            if(spidma_pool->send_func) {
                send_len = spidma_pool->send_func(elem->buf_start, elem->buf_len);
                //SPIDMA_INTF_PRT("spi org:%d, send:%d\r\n", elem->buf_len, send_len);
                if(send_len != elem->buf_len) {
                    break;
                }
            }
            co_list_pop_front(&(spidma_pool->ready));
            co_list_push_back(&(spidma_pool->free), (struct co_list_hdr *)&elem->hdr);
        }
    }
    while(elem);
}

static void spidma_intfer_main( beken_thread_arg_t data )
{
    UINT32 status;
    OSStatus err;    
    GLOBAL_INT_DECLARATION();
    
    if(spidma_intfer_pool_init(data))
    {
        goto spidma_exit;
    }

    spidma_intfer_config_desc(data);
#if CFG_GENERAL_DMA
    spidma_intfer_config_general_dma();
#endif

    GLOBAL_INT_DISABLE(); 
    spidma_hdl = ddev_open(SPIDMA_DEV_NAME, &status, (UINT32)&spidma_intf);

    if(DD_HANDLE_UNVALID == spidma_hdl)
    {
        GLOBAL_INT_RESTORE();
        SPIDMA_INTF_FATAL("sdidma open failed, exit process\r\n");
        goto spidma_exit;
    }
    GLOBAL_INT_RESTORE();

    while(1)
    {
        SPIDMA_MSG_T msg;
        err = rtos_pop_from_queue(&spidma_msg_que, &msg, BEKEN_WAIT_FOREVER);
        if(kNoErr == err)
        {
        	switch(msg.temp_msg) 
            {
                case SPIDMA_INT_POLL:
                    spidma_intfer_poll_handler();
                    break;
                    
                case SPIDMA_EXIT:
                    goto spidma_exit;
                    break;
                    
                default:
                    break;
            }
        }
    }
    
spidma_exit:
    SPIDMA_INTF_FATAL("spidma_intfer_main exit\r\n");

    spidma_intfer_pool_deinit();
    
    if(spidma_hdl != DD_HANDLE_UNVALID) 
    {
        ddev_close(spidma_hdl);
        spidma_hdl = DD_HANDLE_UNVALID;
    }

    rtos_deinit_queue(spidma_msg_que);
    spidma_msg_que = NULL;

    spidma_thread_hdl = NULL;
    rtos_delete_thread(NULL);
}

UINT32 spidma_intfer_init(UINT32 tcp_udp_mode)
{
    int ret;

    SPIDMA_INTF_PRT("spidma_intfer_init %d\r\n", tcp_udp_mode);
    if((!spidma_thread_hdl) && (!spidma_msg_que))
    {

    	ret = rtos_init_queue(&spidma_msg_que, 
    							"temp_det_queue",
    							sizeof(SPIDMA_MSG_T),
    							SPIDMA_QITEM_COUNT);
    	if (kNoErr != ret) 
    	{
    		SPIDMA_INTF_PRT("spidma_intfer ceate queue failed\r\n");
            return kGeneralErr;
    	}
        
        ret = rtos_create_thread(&spidma_thread_hdl,
                                      4,
                                      "spidma_intf",
                                      (beken_thread_function_t)spidma_intfer_main,
                                      1024,
                                      (beken_thread_arg_t)tcp_udp_mode);
        if (ret != kNoErr)
        {
            rtos_deinit_queue(spidma_msg_que);
            spidma_msg_que = NULL;
            SPIDMA_INTF_PRT("Error: Failed to create spidma_intfer: %d\r\n", ret);
            return kGeneralErr;
        }
    }

    return kNoErr;
}

#endif  // CFG_USE_SPIDMA

