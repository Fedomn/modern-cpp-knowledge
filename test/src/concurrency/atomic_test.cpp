// NOLINTBEGIN

#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <numeric>
#include <shared_mutex>
#include <thread>

std::atomic<bool> x, y;
std::atomic<int> z;

// https://nj.gitbooks.io/c/content/content/chapter5/chapter5-chinese.html
// https://bartoszmilewski.com/2008/12/01/c-atomics-and-memory-ordering/
// memory_order_acquire:
// guarantees that subsequent loads are not moved before the current load or any preceding loads.
// 保证后续 loads 不会被被重排到 当前点load 之前，即保证 fence 前后 load 的顺序
//
// memory_order_release:
// preceding stores are not moved past the current store or any subsequent stores.
// 保证前面的 stores 不会被重排到 当前点store 之后，即保证 fence 前后 store 的顺序

// 下面的 write_x_then_y 和 read_y_then_x 组合：
// 保证 x 先于 y store
// 保证 y 先于 x load

void write_x_then_y()
{
  x.store(true, std::memory_order_relaxed);             // 1
  std::atomic_thread_fence(std::memory_order_release);    // 2
  y.store(true, std::memory_order_relaxed);             // 3
}

void read_y_then_x()
{
  while (!y.load(std::memory_order_relaxed));           // 4
  std::atomic_thread_fence(std::memory_order_acquire);  // 5
  if (x.load(std::memory_order_relaxed))                // 6
    ++z;
}

void write_x_then_y2()
{
  // 不保证 x 和 y 的 store 先后，如果 y 先于 x store，则 z 可能不被调用到
  std::atomic_thread_fence(std::memory_order_release);
  x.store(true,std::memory_order_relaxed);
  y.store(true,std::memory_order_relaxed);
}

TEST(AtomicTest, AtomicThreadFence)
{
  for (size_t i = 0; i < 9000; i++)
  {
    x = false;
    y = false;
    z = 0;
    std::thread a(write_x_then_y2);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    assert(z.load() != 0);  // 7
  }
}


// https://downdemo.github.io/Cpp-Concurrency-in-Action-2ed/04_the_cpp_memory_model_and_operations_on_atomic_type.html
// memory_order_seq_cst 是所有原子操作的默认选项，可以省略不写。

//
//
// ------------------------------ concurrency-primer ------------------------------
//
//
/*
  有时候希望 read-modify-write (RMW) 为一个原子操作。下面是一些 C++ 提供的 RMW 操作。
*/
TEST(AtomicTest, AtomicRMW)
{
  // test-and-set
  // works on a Boolean value: we read it, set it to true.
  std::atomic_flag f;
  f.test_and_set();
  f.clear();

  // compare-and-swap
  // It allows us to conditionally exchange a value if its previous value matches some expected one.
  std::atomic<int> c = 3;
  int expected = 4;
  // https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
  // true if the underlying atomic value was successfully changed, false otherwise.
  bool res = c.compare_exchange_strong(expected, 1);
  EXPECT_EQ(res, false);
  EXPECT_EQ(expected, 3);
  EXPECT_EQ(c, 3);

  res = c.compare_exchange_strong(expected, 1);
  EXPECT_EQ(res, true);
  EXPECT_EQ(expected, 3);
  EXPECT_EQ(c, 1);

  while (!c.compare_exchange_weak(expected, 1))
  {
  };
}

/*
sequential consistency 顺序一致性，即 set a=1 后，再 get a 一定是 1
x86 下是 strongly-ordered
ARM 下是 weakly-ordered，即不保证读和写是按照程序里定义的顺序执行，除非插入特殊的 CPU 指令(memory barriers) dmb，保证指令不会重排序
比如，插入两个 dmb 指令，在 a = 1 的前后，可以保证：
- 第一个 dmb 保证，a = 1 之前的读写不会被重排到 a = 1 之后
- 第二个 dmb 保证，a = 1 之后的读写不会被重排到 a = 1 之前

因此，需要搞清楚，原子操作在 weakly-ordered 上的行为。
ARM 上缺失 RMW 操作，需要使用 load-link and store-conditional (LL/SC)
LL 从一个地址读取一个值，然后告诉处理器监控这个地址（监控它有没有被其它处理器修改过）
SC 向一个地址写一个值，该地址不能有其它处理器修改过（通过LL实现），否则返回错误
// 因此，外部需要一个 loop 来循环 LL 和 SC 保证一定成功

问题：LL 对 CPU 的消耗过高，因此大多数 CPU 对 LL 的监控都是粗粒度，(可能是其它地址被操作，也算成了当前的 LL)
这意味着，如果 sc 写入了受监控块中的任何地址，而不仅仅是 load-linked 的特定地址，则 sc 可能会失败
造成了存在虚假的 SC 错误。从使用角度来看，即并没有其它线程修改改地址，并且改值修改成功。

compare_exchange_strong 编译器会产生一层 loop，防止虚假的 SC 失败，
如果返回false，则表示 expected 和 actual 不一致，并修改 expected 为 actual。

compare_exchange_weak 即允许虚假的 SC 失败
存在返回false，但 expected 和 actual 一致。需要用户层在外部包装 loop 来处理。

cppreference 提到，如果 compare-and-swap 在 loop 中调用，某些平台下 weak 的性能比 strong 好
*/


/*
Memory Orderings
- Sequentially Consistent (memory_order_seq_cst)
- Acquire (memory_order_acquire)
- Release (memory_order_release)
- Relaxed (memory_order_relaxed)
- Acquire-Release (memory_order_acq_rel)
- Consume (memory_order_consume)

总结，它控制了一个代码块内，所有操作指令的排序，控制编译器的重排这些指令的方式，
而设置 memory ordering 的代码行相当于一个分界线。
*/

/*
 Acquire / Release 是一个单向的 barrier：
 - acquire：允许其他读取和写入经过它，但需要保证 before -> after 方向
 - release：允许其他读取和写入经过它，但需要保证 after -> before 方向

 如下例子，实现了一个经典的 writer -> reader 的同步示例。
*/
int v;
std::atomic_bool v_ready(false);
void writer()
{
  v = 42;
  v_ready.store(true, std::memory_order_release);
  // 保证 v_ready 被修改为 true 后，v 才被设置
}
void reader()
{
  // 保证 v_ready 为 true 后，v 才能读取
  while (!v_ready.load(std::memory_order_acquire))
  {
    sleep(1);
  }
  assert(v == 42);
}
TEST(AtomicTest, AtomicMemoryOrderingAcquireRelease)
{
  std::thread w(writer);
  std::thread r(reader);
  r.join();
  w.join();
}


/*
  Relaxed 用于一个变量可能在多个线程间访问，但不保证其内存顺序，使用很常见。
  比如，对一个 counter / flag 来说，只要保证它的原子性修改，内存可见顺序并不是必须的。
  比如 stop flag，用户只关心最终结果，比如需要一个统计的 counter，可以不用很精确。
*/
std::atomic<int> foo = 1;
void atomicMultiply(int by)
{
  int expected = foo.load(std::memory_order_relaxed);
  // success: the memory synchronization ordering for the read-modify-write operation if the comparison succeeds
  // failure: the memory synchronization ordering for the load operation if the comparison fails
  while (!foo.compare_exchange_weak(
      expected,
      expected * by,
      /*success*/ std::memory_order_release,  // 保证 exchange 一定在 comparison 之后
      /*failure*/ std::memory_order_relaxed)) // 如果比较失败，则不用关心内存顺序
  {
  }
}

/*
  Acquire-Release (memory_order_acq_rel) 用于原子 RMW 操作，要求 load-acquire 和 store-release。
  典型的场景是 shared_ptr 的引用计数器。
*/
std::atomic_int refCount;
void inc()
{
  refCount.fetch_add(1, std::memory_order_relaxed);
  // 加计数时，它前后读写的 memory ordering 不用关心
}
void dec()
{
  // 减计数时，需要确保 load-acquire 和 store-release (对着 writer/reader 例子分析)
  // 1. load-acquire，即保证 refCount.load(memory_order_acquire) 之前，所有 refCount 的读写不会被重排
  // 2. store-release，即保证 refCount.store(0, memory_order_release) 成功后，才会 delete the data
  if (refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
  {
    // no more references, delete the data
  }
}

/*
  memory_order_seq_cst 提供了一个 global memory ordering，即它的前后行代码对应的指令不会被重排。
*/
/*
  memory_order_consume. data is rarely changed, but often read by many threads.
*/


/*
  volatile 保证两点：
  1. 编译器不会省略那些看起来没必要的 loads 和 stores
  2. 编译器不会 reorder the loads and stores of a volatile variable.

*/
volatile int t = 0;
void volatileW(int *t)
{
  *t = 2; // 看起来 the dead store 将不会被编译器优化省略掉
  *t = 3;
}

// NOLINTEND