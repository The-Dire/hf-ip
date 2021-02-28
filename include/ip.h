#ifndef IPV4_H
#define IPV4_H
#include "syshead.h"
#include "ethernet.h"
#include "skbuff.h"
#include "sock.h"

#define IPV4 0x04
#define IP_TCP 0x06
#define ICMPV4 0x01

#define IP_HDR_LEN sizeof(struct iphdr)
#define ip_len(ip) (ip->len - (ip->ihl * 4))

#ifdef DEBUG_IP
#define ip_dbg(msg, hdr)                                                \
    do {                                                                \
        print_debug("ip "msg" (ihl: %hhu version: %hhu tos: %hhu "   \
                    "len %hu id: %hu frag_offset: %hu ttl: %hhu " \
                    "proto: %hhu csum: %hx " \
                    "saddr: %hhu.%hhu.%hhu.%hhu daddr: %hhu.%hhu.%hhu.%hhu)", \
                    hdr->ihl,                                           \
                    hdr->version, hdr->tos, hdr->len, hdr->id,          \
                    hdr->frag_offset, hdr->ttl, hdr->proto, hdr->csum,   \
                    hdr->saddr >> 24, hdr->saddr >> 16, hdr->saddr >> 8, hdr->saddr >> 0, \
                    hdr->daddr >> 24, hdr->daddr >> 16, hdr->daddr >> 8, hdr->daddr >> 0); \
    } while (0)
#else
#define ip_dbg(msg, hdr)
#endif

struct iphdr {
    // TODO: 支持大端主机 
    uint8_t ihl : 4; // 长度字段ihl同样是4位长度，它表示IP标头中32位字的数量。由于该字段的大小为4位，因此只能保留最大值15。因此，IP标头的最大长度为60个八位位组 
    uint8_t version : 4; // ip版本
    uint8_t tos;    // 
    uint16_t len;   // 字段len传达整个IP数据报的长度。由于它是一个16位字段，因此最大长度为65535字节。后序的协议比如TCP分片
    uint16_t id;    // id字段用于索引数据报，并最终用于重组分段的IP数据报
    uint16_t frag_offset; // frag_offset指示片段在数据报中的位置
    uint8_t ttl;    // ttl生存时间
    uint8_t proto;  // roto字段为数据报提供了在其有效载荷中携带其他协议的固有能力。该字段通常包含16（UDP）或6（TCP）之类的值
    uint16_t csum;  // 校验和字段csum用于验证IP标头的完整性
    uint32_t saddr; // 源地址
    uint32_t daddr; // 目的地址
    uint8_t data[];
} __attribute__((packed));
// 从skbuff中取出ip协议的数据
static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
    return (struct iphdr *)(skb->head + ETH_HDR_LEN);
}
// 解析ip数据包头
static inline uint32_t ip_parse(char *addr)
{
    uint32_t dst = 0;
    
    if (inet_pton(AF_INET, addr, &dst) != 1) {
        perror("ERR: Parsing inet address failed");
        exit(1);
    }

    return ntohl(dst);
}

int ip_rcv(struct sk_buff *skb);
int ip_output(struct sock *sk, struct sk_buff *skb);

#endif
