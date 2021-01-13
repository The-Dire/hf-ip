#ifndef TUNTAP_IF_H
#define TUNTAP_IF_H
void tun_init(); // tun设备初始化
int tun_read(char *buf, int len); // tun设备读数据
int tun_write(char *buf, int len);// tun设备写数据
void free_tun(); // 释放tun设备占用资源
#endif
