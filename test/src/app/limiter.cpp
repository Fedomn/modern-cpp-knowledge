#include <gtest/gtest.h>

// NOLINTBEGIN
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>

/**
  A fixed-size limiter.
*/
class FixedSizeLimiter
{
 public:
  FixedSizeLimiter() = default;
  ~FixedSizeLimiter() = default;
  explicit FixedSizeLimiter(uint32_t max_size, uint32_t request_timeout_ms)
      : max_size_(max_size),
        request_timeout_ms_(request_timeout_ms)
  {
  }

  int Request()
  {
    std::unique_lock<std::mutex> lock_guard(mutex_);
    while (total_cnt_ >= max_size_)
    {
      auto timeout = std::chrono::milliseconds(request_timeout_ms_ * 1000);
      auto ret = condition_.wait_for(lock_guard, timeout);
      if (ret == std::cv_status::timeout)
        return 1;
    }
    total_cnt_++;
    std::cout << "FixedSizeLimiter request a token, total_cnt: " << total_cnt_ << "\n";
    return 0;
  }

  void Release()
  {
    std::unique_lock<std::mutex> lock_guard(mutex_);
    total_cnt_--;
    condition_.notify_one();
    std::cout << "FixedSizeLimiter release a token, total_cnt: " << total_cnt_ << "\n";
  }

 private:
  uint32_t max_size_;
  uint32_t total_cnt_ = 0;
  uint32_t request_timeout_ms_ = 0;
  std::mutex mutex_;
  std::condition_variable condition_;
};

// NOLINTEND
TEST(FixSizeLimiterTest, Const)  // NOLINT
{
  auto l = FixedSizeLimiter(10, 1000);
  l.Request();
  l.Request();
  l.Release();
  l.Release();
}