https://webkit.org/blog/6161/locking-in-webkit/

Spinlocks are widely available, and those are often optimized for using little memory and having great fast paths for uncontended lock acquisition and microcontention. But spinlocks will waste CPU time when the lock isn’t immediately available.

OS mutexes know how to suspend threads if the lock is not available, so they are more efficient under contention – but they usually have a slower uncontended fast path, they don’t necessarily have the behavior we want under microcontention, and they require more memory. C++ provides access to OS mutexes with std::mutex.

Prior to WTF::Lock, WebKit had a mix of spinlocks and OS mutexes, and we would try to pick which one to use based on guesses about whether they would benefit more from `uncontended speed` or `efficiency under contention`.
- If we needed both of those qualities, we would have no choice but to punt on one of them. For example, we had a spinlock in CodeBlock that should have been adaptive because it protected long critical sections,
- and we had an OS mutex in our parallel GC that accounted for 3% of our time in the Octane Splay benchmark because of shortcomings in fast path performance.

These issues are resolved thanks to WTF::Lock. Also, there was no way to have a `small lock` (1 byte or less) that was also `efficient under contention`, since OS mutexes tend to be large.


--- deepseek 总结
https://yuanbao.tencent.com/chat/naQivTmsDa/b424a94d-659f-4bb3-ba5b-2cc314fb05d7


```c++
#include <stdio.h>
#include <pthread.h>

int main() {
    // pthread_mutex_t 的大小是 32 字节或 64 字节
    printf("Size of pthread_mutex_t: %zu bytes\n", sizeof(pthread_mutex_t));
    return 0;
}
```

直接使用 pthread_mutex_t 会带来如下问题：

1. 空间开销巨大 - 锁比数据还大，可能数据小于 32 字节
  - 内存占用急剧上升，尤其是在内存受限的设备（如手机）上
  - 缓存利用率降低，因为宝贵的高速缓存行被锁占据，而不是实际数据。
  - 限制了引擎设计，开发者会犹豫是否应该为小对象添加必要的同步保护，因为代价太高
* ​​WTF::Lock 的解决：​​ 仅需 ​​1 字节​​

2. 拖慢高频路径
  - 拖慢了事件处理的速度，即使没有竞争
* WTF::Lock 的解决：WTF::Lock 的无竞争路径 (lock() / unlock()) 几乎就是一个简单的 CAS 操作，​​开销极小​​，接近自旋锁的速度。这使得高频访问的性能损失降到最低。

---

WTF::Lock 算法细节​​
​​原子状态（2比特）：​​
- isLockedBit: 锁是否被持有。
- hasParkedBit: 是否有线程在等待队列中。

​​关键技术：WTF::ParkingLot
- ​​核心思想：将锁的​​排队（Queuing）和线程挂起/恢复（Suspending/Resuming）功能​​从锁本身剥离，移到一个全局的、基于地址哈希的并发哈希表 ParkingLot 中。
- 优势：​​
  - ​​空间解耦：​Lock/Condition 只需维护很小的原子状态（1字节），ParkingLot 的空间消耗​​只与活跃线程数相关​​，与锁的数量无关（解决了海量锁带来的空间问题）。
  - ​​灵活性：提供了一个强大的底层 API (parkConditionally, unparkOne, unparkAll)，使得高效实现各种锁协议（如 WTF::Lock, WTF::Condition）以及 FUTEX 类操作成为可能。
  ​​- 避免虚假唤醒：parkConditionally 的设计确保了 Condition::wait 只在被通知时返回。

可以举一个 parkinglot 的最小实现示例么？

---

lambda 在 parkinglot 实现中起到什么作用？
ParkingLot 提供停车/唤醒的​​核心流程框架​​和​​队列管理​​。Lambda 允许调用者 (WTF_Lock, WTF_Condition) 将​​特定于自身同步原语的状态管理逻辑​​（检查条件、更新标志、释放用户锁）​​注入​​到这个框架的​​关键决策点​​。

