#ifndef ICMPV4_H
#define ICMPV4_H

#include "syshead.h"
#include "skbuff.h"

#define ICMP_V4_REPLY           0x00
#define ICMP_V4_DST_UNREACHABLE 0x03
#define ICMP_V4_SRC_QUENCH      0x04
#define ICMP_V4_REDIRECT        0x05
#define ICMP_V4_ECHO            0x08
#define ICMP_V4_ROUTER_ADV      0x09
#define ICMP_V4_ROUTER_SOL      0x0a
#define ICMP_V4_TIMEOUT         0x0b
#define ICMP_V4_MALFORMED       0x0c

struct icmp_v4 {
    uint8_t type;   // 消息类型(该消息的目的是什么)通常仅使用大约8个。实现中，使用类型0（回声应答），3（目标不可达）和8（回声请求)。
    uint8_t code;   // 进一步描述消息的含义
    uint16_t csum;  // 
    uint8_t data[];
} __attribute__((packed));
// icmp 查询消息结构体
struct icmp_v4_echo {
    uint16_t id;        // 字段id由发送主机设置，以确定echo reply（回复)打算用于哪个进程
    uint16_t seq;       // 序号,用于检测回显消息在传输过程中是否消失或是否重新排序。
    uint8_t data[];     // 可选的，通常包含诸如echo的时间戳之类的信息。然后可以将其用于估计主机之间的往返时间。
} __attribute__((packed));
// icmp常见的目的不可达错误信息结构体
struct icmp_v4_dst_unreachable {
    uint8_t unused;     // 首个8位是不使用的字段
    uint8_t len;        // 数据报长度
    uint16_t var;       // 
    uint8_t data[];
} __attribute__((packed));


void icmpv4_incoming(struct sk_buff *skb);
void icmpv4_reply(struct sk_buff *skb);

#endif
