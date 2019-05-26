#include <lwip/sockets.h>
#include <lwip/netdb.h>

#define THREAD_SIZE 1024*4
#define PKT_SIZE (1024 * 8)
#define PORT     5001

static struct sockaddr_in server_addr;
static rt_bool_t is_running = RT_FALSE;

void iperf_client_entry(void* parameter)
{
    int sock, optval;
    rt_uint8_t *pkt;

    sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        rt_kprintf("[iperf] create socket failed.\n");
        return;
    }

    // pkt = (rt_uint8_t*) rt_malloc (PKT_SIZE);
    pkt = (rt_uint8_t*) rt_malloc (PKT_SIZE);
    if (pkt == RT_NULL)
    {
        rt_kprintf("[iperf] out of memory");
        lwip_close(sock);
        return;
    }

    /* disable nagle */
    optval = 1;
    lwip_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

    if (lwip_connect(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) != 0)
    {
        rt_kprintf("[iperf] connect to server failed.\n");
        lwip_close(sock);
        rt_free(pkt);
        return;
    }

	optval = RT_TICK_PER_SECOND*5; /* LWIP SO_SNDTIMEO: (s32_t)*(int*)optval  */
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&optval, sizeof(optval)); /* set send timeout 1s. */

    while (is_running == RT_TRUE)
    {
        {
            int res;
            fd_set writeset;
            struct timeval timeout;

            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            FD_ZERO(&writeset);
            FD_SET(sock, &writeset);

            res = lwip_select(sock + 1, RT_NULL, &writeset, RT_NULL, &timeout);

            if(res <= 0)
            {
                rt_kprintf("[iperf] wait for write timeout!\n");
                break;
            }

            if (!FD_ISSET(sock, &writeset))
            {
                rt_kprintf("[iperf] sock !FD_ISSET\n");
                break;
            }
        }

        if (lwip_write(sock, pkt, PKT_SIZE) <= 0)
        {
            rt_kprintf("disconnected.\n");
            break;
        }
    }

    rt_kprintf("[iperf] exit.\n");
    lwip_close(sock);
    rt_free(pkt);
}

int iperf_client(const char* server, int number)
{
    int index;
    rt_thread_t tid;
    struct hostent *host;

    if (is_running == RT_TRUE)
    {
        is_running = RT_FALSE;
        rt_kprintf("stop all of iperf client.\n");
        return 0;
    }

    is_running = RT_TRUE;
    host = lwip_gethostbyname(server);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr*) host->h_addr);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    for (index = 0; index < number; index ++)
    {
        tid = rt_thread_create("iperf", iperf_client_entry, (void*)index,
            THREAD_SIZE, 26, 10);
        if (tid != RT_NULL) rt_thread_startup(tid);
    }

    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(iperf_client, iperf_client(server, num_client));

#ifdef FINSH_USING_MSH
static int iperf(int argc, char **argv)
{
    const char* server;
    int num_client;

    if(argc != 3)
    {
        rt_kprintf("Usage: iperf 192.168.1.123 1\n");
        return -1;
    }

    server = argv[1];
    num_client = atoi(argv[2]);

    return iperf_client(server, num_client);
}
MSH_CMD_EXPORT(iperf, iperf 192.168.1.123 1);
#endif /* FINSH_USING_MSH */

#endif /* RT_USING_FINSH */
