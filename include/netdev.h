#ifndef NETDEV_H
#define NETDEV_H
#include "syshead.h"
#include "ethernet.h"
#include "skbuff.h"
#include "utils.h"

#define BUFLEN 1600
#define MAX_ADDR_LEN 32

#define netdev_dbg(fmt, args...)                \
    do {                                        \
        print_debug("NETDEV: "fmt, ##args);     \
    } while (0)

struct eth_hdr;
// 网卡的结构体
struct netdev {
    uint32_t addr;
    uint8_t addr_len;
    uint8_t hwaddr[6];
    uint32_t mtu;
};

void netdev_init(); // 初始化网卡
int netdev_transmit(struct sk_buff *skb, uint8_t *dst, uint16_t ethertype); // 网络流量传输函数
void *netdev_rx_loop(); // 网卡循环接收网络流量
void free_netdev(); // 释放网卡资源
struct netdev *netdev_get(uint32_t sip); // 获取网卡设备
#endif
