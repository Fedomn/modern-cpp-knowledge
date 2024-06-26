#include <bthread/condition_variable.h>
#include <bthread/mutex.h>
#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

// 虚假唤醒:
//
// 为什么条件锁会产生虚假唤醒现象（spurious wakeup）？
// https://www.zhihu.com/question/271521213
// pthread 的条件变量等待 pthread_cond_wait 是使用阻塞的系统调用实现的（比如 Linux 上的
// futex），这些阻塞的系统调用在进程被信号中断后，通常会中止阻塞、直接返回 EINTR 错误。
//
// 同样是阻塞系统调用，你从 read 拿到 EINTR
// 错误后可以直接决定重试，因为这通常不影响它本身的语义。而条件变量等待则不能，因为本线程拿到 EINTR 错误和重新调用 futex
// 等待之间，可能别的线程已经通过 pthread_cond_signal 或者 pthread_cond_broadcast发过通知了。
//
// 所以，虚假唤醒的一个可能性是条件变量的等待被信号中断。
//
// 举例代码: brpc 里的 wait_pthread 方法里的 while(true) 里的 futex_wait_private 方法被中断后，如果收到 EINTR 会继续循环。
// 比如，资源受限时候，系统调用被中断，导致它被中断唤醒，再重新 while 循环，但是刚好在 futex_wait_private 前，其它线程已经 notify 过了，
// 所以，就导致该 wait 线程被虚假唤醒了，并且 notify 已经执行过了，它只能一直等待。
//
// 修复方式：使用 wait_for + while 条件变量判断，即使 notify 已经执行过了，但条件变量会设置成功，这样再 wait timeout 后，
// 下次 while 循环会判断条件变量，决定是否在 wait。
//

// 唤醒丢失:
//
// https://blog.zzsqwq.cn/posts/why-use-condition-variable/
// wait 线程还未进入系统调用位置，进入等待状态，notify 线程就已经发出了信号，导致 wait 线程未收到信号
// 修复方式：加入 lock 保证时序，即 notify 之前，wait 线程已经释放了锁，进入等待状态了。
// 同时，wait_for + while 条件变量判断，也是必须的。因为，为了防止即使唤醒丢失了，也能通过条件变量判断出，不需要等待了。


//
// 条件变量的虚假唤醒和唤醒丢失问题
// https://zhuanlan.zhihu.com/p/652823880
// 虚假唤醒是指当你对线程进行唤醒时，你不希望被唤醒的线程也被唤醒的现象。
// pthread库之所以允许虚假，是为了性能上的考虑。pthread库希望应用程序某些时候在进入内核态之前就被唤醒，这样就可以避免进入内核态的开销。

bthread::Mutex mutex;
bthread::ConditionVariable condition;

void Producer()
{
  std::unique_lock<bthread::Mutex> lock_guard(mutex);
  condition.notify_all();
}

void Consumer()
{
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::unique_lock<bthread::Mutex> lock_guard(mutex);
  int ret = condition.wait_for(lock_guard, 1 * 1000 * 1000);
  if (ret == ETIMEDOUT)
  {
    std::cout << "wait timeout \n";
  }
}

TEST(LockTest, Const)  // NOLINT
{
  std::thread a(Producer);
  std::thread b(Consumer);
  a.join();
  b.join();
}

// gpt:
// 在执行系统调用时，可能会被中断，原因如下：
// 信号处理：当一个信号（如SIGINT、SIGTERM等）被发送到进程时，操作系统会中断当前正在执行的系统调用，并调用相应的信号处理函数。这样，程序可以在信号处理函数中执行特定的操作，如清理资源、保存状态等。
// 系统资源限制：当系统资源（如内存、CPU时间等）受到限制时，操作系统可能会中断正在执行的系统调用。这可能是由于进程达到了资源限制，或者系统资源不足。在这种情况下，系统调用将返回错误，并将errno设置为EINTR。
// 其他系统事件：操作系统可能会在执行系统调用时检测到其他系统事件（如硬件故障、内核错误等）。在这种情况下，操作系统可能会中断系统调用，并将errno设置为EINTR。
// 多线程竞争：在多线程程序中，一个线程可能会在另一个线程正在执行系统调用时被中断。这可能是由于线程调度、优先级更改等原因导致的。在这种情况下，被中断的线程可能会在系统调用返回时收到EINTR错误。
// 为了处理这些中断，你需要检查系统调用返回时的errno值。如果errno设置为EINTR，你可以选择重新尝试系统调用，或者根据具体情况采取其他措施。
//
// 请注意，并非所有的系统调用都会受到中断的影响。对于那些不会受到中断影响的系统调用，你不需要检查EINTR错误。在处理系统调用时，请确保正确处理中断，以避免程序意外终止或出现其他问题。
//
// 总之，在执行系统调用时，可能会被中断，原因包括信号处理、系统资源限制、其他系统事件和多线程竞争等。在处理系统调用时，你需要检查errno的值，以确定系统调用是否因为其他原因（如错误）而失败。在处理中断时，请确保正确处理中断，以避免程序意外终止或出现其他问题。


// https://man7.org/linux/man-pages/man2/futex.2.html
//
// gpt:
// futex（Fast Userspace Mutex）是一种用户空间互斥锁，它使用系统调用在内核空间进行同步
//
// 互斥锁：futex可以用作互斥锁，以保护共享资源免受多个线程同时访问的影响。当一个线程尝试获取互斥锁时，它会检查锁的状态。如果锁已被其他线程持有，线程将使用futex系统调用进入休眠状态，直到锁被释放。
// 条件变量：futex可以用作条件变量，以实现线程间的同步。当一个线程等待某个条件满足时，它可以使用futex系统调用进入休眠状态。当另一个线程改变条件并唤醒等待的线程时，它可以使用futex系统调用唤醒休眠的线程。
// 读写锁：futex可以用作读写锁，以实现多个线程对共享资源的并发访问。当一个线程尝试获取读锁时，它会检查锁的状态。如果锁已被其他线程持有，线程将使用futex系统调用进入休眠状态，直到锁被释放。当一个线程尝试获取写锁时，它会检查锁的状态。如果锁已被其他线程持有，线程将使用futex系统调用进入休眠状态，直到锁被释放。
// 信号量：futex可以用作信号量，以实现线程间的同步。当一个线程等待信号量时，它可以使用futex系统调用进入休眠状态。当另一个线程发送信号量时，它可以使用futex系统调用唤醒休眠的线程。
// 屏障：futex可以用作屏障，以实现线程间的同步。当一个线程到达屏障时，它可以使用futex系统调用进入休眠状态。当所有线程都到达屏障时，它们可以使用futex系统调用唤醒休眠的线程。
// futex系统调用通常用于实现高性能的同步原语，如互斥锁、条件变量、读写锁、信号量和屏障等。在使用futex时，请确保正确处理中断和错误，以避免程序意外终止或出现其他问题。
//
// 总之，futex系统调用有多种具体用途，如互斥锁、条件变量、读写锁、信号量和屏障等。在使用futex时，请确保正确处理中断和错误，以避免程序意外终止或出现其他问题。在实现高性能同步原语时，futex系统调用是一种非常有用的工具。
