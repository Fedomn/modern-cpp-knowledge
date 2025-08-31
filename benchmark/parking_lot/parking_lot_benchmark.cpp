#include <atomic>
#include <condition_variable>
#include <functional>  // for std::function
#include <mutex>
#include <unordered_map>

// 简化的线程队列结构
struct ThreadQueue {
  std::mutex queueMutex;       // 保护这个队列的互斥锁 (ParkingLot桶锁的简化)
  std::condition_variable cv;  // 用于挂起/唤醒此队列上等待的线程
};

// 极度简化的 ParkingLot (仅用于演示lambda作用)
class ParkingLot {
 public:
  // 存储地址->队列的映射 (真实实现是并发哈希表)
  static std::unordered_map<const void*, ThreadQueue> queues;
  // 保护queues映射的全局锁 (真实实现分桶加锁)
  static std::mutex globalMapMutex;

  // 核心1: 有条件地停放线程
  static bool parkConditionally(
      const void* address,
      std::function<bool()> validation,   // **关键Lambda 1: 验证条件**
      std::function<void()> beforeSleep,  // **关键Lambda 2: 准备挂起前操作**
      std::chrono::steady_clock::time_point timeout) {
    // 1. 查找或创建对应地址的队列 (简化: 忽略并发，直接加全局锁)
    std::unique_lock<std::mutex> mapLock(globalMapMutex);
    ThreadQueue& queue = queues[address];  // 获取该地址对应的队列
    mapLock.unlock();                      // (真实实现：锁定特定桶)

    // 2. **锁定目标地址的队列** (模拟获得队列锁)
    std::unique_lock<std::mutex> queueLock(queue.queueMutex);

    // 3. **执行调用者提供的验证Lambda (Validation)**
    //     - 调用者在此检查其锁/条件的状态是否仍满足停车条件
    //     - 此Lambda在队列锁保护下执行，状态稳定！
    if (!validation()) {
      // 条件不再成立！(例如：锁突然可用了) -> 不挂起，返回false
      return false;
    }

    // 4. **当前线程即将进入队列并挂起**
    //     - 在此简化版，我们假设线程ID已隐式加入队列概念
    //     - (真实实现：记录线程信息到队列节点)

    // 5. **释放队列锁** (让其他线程可以操作此队列)
    queueLock.unlock();

    // 6. **执行调用者提供的beforeSleep Lambda**
    //     - **关键时机！** 已出队列临界区，但线程尚未挂起
    //     - 调用者可在此执行重要操作 (如：解锁用户锁！)
    beforeSleep();

    // 7. **真正挂起线程** (等待被unpark唤醒)
    std::unique_lock<std::mutex> waitLock(queue.queueMutex);  // 重新获得锁以使用CV
    if (queue.cv.wait_until(waitLock, timeout) == std::cv_status::timeout) {
      // 处理超时 (简化省略)
      return false;
    }
    // 被唤醒，返回成功
    return true;
  }

  // 核心2: 唤醒一个停放线程
  static void unparkOne(
      const void* address,
      std::function<void(bool didUnpark, bool queueEmpty)> callback) {  // **关键Lambda 3: 处理结果**

    // 1. 查找对应地址的队列
    std::unique_lock<std::mutex> mapLock(globalMapMutex);
    auto it = queues.find(address);
    if (it == queues.end()) {
      // 没人在等！直接返回
      callback(false, true);  // 告知：没唤醒线程，队列空(或不存在)
      return;
    }
    ThreadQueue& queue = it->second;
    mapLock.unlock();

    // 2. **锁定目标队列**
    std::unique_lock<std::mutex> queueLock(queue.queueMutex);

    // 3. **尝试从队列中取出一个等待线程** (简化：我们假设队列非空)
    bool didUnpark = true;    // (真实实现：检查队列并移除一个节点)
    bool queueEmpty = false;  // (真实实现：检查移除后队列是否为空)

    // 4. **执行唤醒操作**
    queue.cv.notify_one();  // 唤醒一个线程

    // 5. **执行调用者提供的回调Lambda (Callback)**
    //     - 此Lambda在**队列锁保护下**执行！
    //     - 调用者可基于唤醒结果(didUnpark, queueEmpty)原子更新其状态
    callback(didUnpark, queueEmpty);

    // 6. (队列锁随queueLock析构自动释放)
  }
};

// 初始化静态成员
std::unordered_map<const void*, ThreadQueue> ParkingLot::queues;
std::mutex ParkingLot::globalMapMutex;

class WTF_Lock {
 private:
  std::atomic<uint8_t> m_state{ 0 };  // 假设8位原子变量
  static constexpr uint8_t isLockedBit = 0x01;
  static constexpr uint8_t hasParkedBit = 0x02;

 public:
  void lock() {
    // ... (尝试快速获取锁、自旋等逻辑省略) ...
    // 最终需要停车：

    // **关键点 1: 调用 parkConditionally 并传入两个Lambda**
    ParkingLot::parkConditionally(
        &m_state,           // 地址：锁的状态变量地址
        [this]() -> bool {  // **Validation Lambda**
          // 在ParkingLot队列锁保护下检查：
          // 锁是否仍被持有(lockBit) 且 我们之前设置的hasParkedBit还在？
          return (m_state.load() & (isLockedBit | hasParkedBit)) == (isLockedBit | hasParkedBit);
        },
        []() { /* beforeSleep Lambda: 在WTF::Lock的lock()中通常为空 */ },
        std::chrono::steady_clock::time_point::max()  // 无超时
    );
    // 被唤醒后，线程已持有锁
  }

  void unlock() {
    // ... (尝试快速释放锁逻辑省略：如果hasParkedBit没设，直接CAS清除lockBit) ...
    // 需要唤醒等待者：

    // **关键点 2: 调用 unparkOne 并传入一个Lambda**
    ParkingLot::unparkOne(
        &m_state,                                  // 地址：锁的状态变量地址
        [this](bool didUnpark, bool queueEmpty) {  // **Callback Lambda**
          // 在ParkingLot队列锁保护下更新状态：
          if (didUnpark) {
            if (queueEmpty) {
              m_state.store(0);  // 唤醒了一个，队列空了：清除所有标志
            } else {
              m_state.store(hasParkedBit);  // 唤醒了一个，但队列还有等：保留hasParkedBit
            }
          } else {
            // 没唤醒任何人 (不应该发生？) 但安全处理：清除lockBit，保留hasParkedBit?
            m_state.store(hasParkedBit);
          }
        });
  }
};