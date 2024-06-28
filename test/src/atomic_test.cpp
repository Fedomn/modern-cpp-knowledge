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

// NOLINTEND