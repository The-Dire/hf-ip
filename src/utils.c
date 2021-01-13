#include "syshead.h"
#include "utils.h"

extern int debug;
// 该函数调用方式详见src/tuntap_if.c
int run_cmd(char *cmd, ...)
{
    va_list ap;//声明命名参数
    char buf[CMDBUFLEN];
    va_start(ap, cmd);//va_start宏允许访问命名参数parmN后面的变量参数。
    vsnprintf(buf, CMDBUFLEN, cmd, ap);// 将结果写入buf

    va_end(ap);

    if (debug) {
        printf("EXEC: %s\n", buf);
    }

    return system(buf);
}
// 每16位求和
uint32_t sum_every_16bits(void *addr, int count)
{
    register uint32_t sum = 0;
    uint16_t * ptr = addr;
    
    while( count > 1 )  {
        /*  按位求和 */
        sum += * ptr++;
        count -= 2;
    }

    /*  添加剩余字节 */
    if( count > 0 )
        sum += * (uint8_t *) ptr;

    return sum;
}
// 计算校验和
uint16_t checksum(void *addr, int count, int start_sum)
{
    /* 计算从位置“addr”开始的
     *         字节数的校验和
     * Taken from https://tools.ietf.org/html/rfc1071
     */
    uint32_t sum = start_sum;

    sum += sum_every_16bits(addr, count);
    
    /*  将32位和压缩到16位 */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}
// 获取解析主机名
int get_address(char *host, char *port, struct sockaddr *addr)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(host, port, &hints, &result);

    if (s != 0) {
        print_err("getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        *addr = *rp->ai_addr;
        freeaddrinfo(result);
        return 0;
    }
    
    return 1;
}
// 解析字符串转换成sock可接收的ipv4地址,sock是在tcp.c中声明的数据结构
uint32_t parse_ipv4_string(char* addr) {
    uint8_t addr_bytes[4];
    sscanf(addr, "%hhu.%hhu.%hhu.%hhu", &addr_bytes[3], &addr_bytes[2], &addr_bytes[1], &addr_bytes[0]);
    return addr_bytes[0] | addr_bytes[1] << 8 | addr_bytes[2] << 16 | addr_bytes[3] << 24;
}
// 比较大小
uint32_t min(uint32_t x, uint32_t y) {
    return x > y ? y : x;
}
