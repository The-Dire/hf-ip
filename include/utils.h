#ifndef UTILS_H
#define UTILS_H

#define CMDBUFLEN 100

#define print_debug(str, ...)                       \
    printf(str" - %s:%u\n", ##__VA_ARGS__, __FILE__, __LINE__);

#define print_err(str, ...)                     \
    fprintf(stderr, str, ##__VA_ARGS__);

int run_cmd(char *cmd, ...); // 执行系统命令
uint32_t sum_every_16bits(void *addr, int count); // 16位求和
uint16_t checksum(void *addr, int count, int start_sum);// 计算校验和
int get_address(char *host, char *port, struct sockaddr *addr);// 获取解析主机名
uint32_t parse_ipv4_string(char *addr);// 解析字符串转换成sock可接收的ipv4地址,sock是在tcp.c中声明的数据结构
uint32_t min(uint32_t x, uint32_t y);// 比较大小

#endif
