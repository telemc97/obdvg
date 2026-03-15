#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Common settings for Pico W
#define NO_SYS                      0
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            16
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0
#define LWIP_SOCKET                 0
#define SYS_LIGHTWEIGHT_PROT        1

#define LWIP_CHKSUM_ALGORITHM       3

#define LWIP_DHCP                   1
#define LWIP_DNS                    1
#define LWIP_TCP                    1
#define LWIP_UDP                    1

// Some extra ones for Pico W Bluetooth support if needed
#define LWIP_IGMP                   1
#define LWIP_IPV4                   1

#endif
