#ifndef __SPIDMA_INTF_PUB_H__
#define __SPIDMA_INTF_PUB_H__

typedef enum {
    SPIDMA_SND_NONE         = 0LU,
    SPIDMA_SND_UDP,
    SPIDMA_SND_TCP,
} SPIDMA_SND_TYPE;
#endif

void spidma_intfer_exit_thread(void);
UINT32 spidma_intfer_init(UINT32 tcp_udp_mode);

