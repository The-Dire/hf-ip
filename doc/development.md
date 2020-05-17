## 开发文档

hf-ip处于非常初始阶段，具有许多硬编码值，并且开发起来并不直观。

本文档旨在提供有关当前功能，路线图和总体开发例程的信息。

## 调试

使用`make debug`构建hf-ip。 它添加了调试符号，并且默认情况下启用了Google的Address Sanitizer。

#### 调试输出

当使用`make debug`构建`hf-ip`程序时会变为debug状态并输出调试语句。你可以使用标头中定义的宏来启用/禁用不同的组件调试输出。

例如,启用特定于套接字的输出：
```sh
make clean
CFLAGS+=-DDEBUG_SOCKET make debug 
```

#### 通过tcpdump调试网络

将tcpdump与您正在使用的IP地址一起使用，例如：

```sh
tcpdump -i any host 10.0.0.4 -n
#输出为：
IP 10.0.0.4.12000 > 10.0.0.5.8000: Flags [S], seq 1525252, win 512, length 0
IP 10.0.0.5.8000 > 10.0.0.4.12000: Flags [S.], seq 1332068674, ack 1525253, win 29200, options [mss 1460], length 0
IP 10.0.0.4.12000 > 10.0.0.5.8000: Flags [.], ack 1, win 512, length 0
```

与详细的`hf-ip`输出信息一起看，就可以对行为和点模式进行故障排除。

#### 跟踪程序代码

gdb即可。

请参阅https://sourceware.org/gdb/current/onlinedocs/gdb/Threads.html 进行线程调试。

#### 调试内存分配和使用

到目前为止，有用的调试工具之一是Google的Address Sanitizer。 它内置于较新的GCC版本中，并通过-fsanitize = address激活。 make debug默认启用此功能。
https://github.com/google/sanitizers/wiki/AddressSanitizer

#### 调试并发或协程

hf-ip使用具有共享数据结构的多个线程，因此很容易引入诸如竞争条件之类的编程错误。

Google的Thread Sanitizer也内置在较新的GCC中，这有助于在没有适当防护的情况下精确地并发访问变量。

https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual


以上。