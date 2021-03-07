#ifndef LIBHFIP_H_
#define LIBHFIP_H_

#include <poll.h>
#include <dlfcn.h>
#include "list.h"
#include "utils.h"

#ifdef DEBUG_API
#define hf_dbg(msg, ...)                                               \
    do {                                                                \
        print_debug("hfip ttid %lu "msg, pthread_self(), ##__VA_ARGS__); \
    } while (0)
#define hf_sock_dbg(msg, sock, ...)                                        \
    do {                                                                \
        hf_dbg("hffd %d fd %d: "msg, sock->hffd, sock->fd, ##__VA_ARGS__); \
    } while (0)
#else
#define hf_sock_dbg(msg, sock, ...)
#define hf_dbg(msg, ...)
#endif

struct hfip_sock {
    struct list_head list;
    int hffd; /* For Level-IP IPC */
    int fd;
};

static inline struct hfip_sock *hfip_alloc() {
    struct hfip_sock *sock = malloc(sizeof(struct hfip_sock));
    memset(sock, 0, sizeof(struct hfip_sock));

    return sock;
};

static inline void hfip_free(struct hfip_sock *sock) {
    free(sock);
}

#endif
