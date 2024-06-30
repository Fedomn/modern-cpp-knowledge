# reference

- [epoll原理详版](https://mp.weixin.qq.com/s/7w0p7B_nGvdGU2nef4uCLA)

- [epoll原理demo系列](https://rebootcat.com/2020/09/26/epoll_cookbook/)
- https://rebootcat.com/2020/09/26/epoll_examples/
- https://rebootcat.com/2020/09/26/thundering_herd/

- [Multithreaded Server in c++ using Epoll](https://unscriptedcoding.medium.com/multithreaded-server-in-c-using-epoll-baadad32224c)



# doc

epoll是Linux操作系统提供的一种事件驱动的I/O模型，用于高效地处理大量并发连接的网络编程。它相比于传统的select和poll方法，具有更高的性能和扩展性。使用epoll可以实现以下几个优势：

高效处理大量并发连接：epoll采用了事件驱动的方式，只有当有可读或可写事件发生时才会通知应用程序，避免了遍历所有文件描述符的开销。

内核与用户空间数据拷贝少：使用epoll时，内核将就绪的文件描述符直接填充到用户空间的事件数组中，减少了内核与用户空间之间数据拷贝次数。

支持边缘触发（Edge Triggered）模式：边缘触发模式下，仅在状态变化时才通知应用程序。这意味着每次通知只包含最新状态的文件描述符信息，可以有效避免低效循环检查。

支持水平触发（Level Triggered）模式：水平触发模式下，在就绪期间不断地进行通知，直到应用程序处理完该文件描述符。
