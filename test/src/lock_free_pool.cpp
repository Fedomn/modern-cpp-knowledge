// NOLINTBEGIN
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>


/**
  reference:
  https://stackoverflow.com/questions/43540943/using-boost-lockfree-queue-is-slower-than-using-mutexes
  https://blog.afach.de/?p=825
*/
class LockFreePool
{
  class Object
  {
   public:
    Object(std::string obj, uint32_t obj_idx, LockFreePool *pool_ptr) : obj_(obj), obj_idx_(obj_idx), pool_ptr_(pool_ptr)
    {
    }
    Object() = delete;
    Object(const Object &) = delete;
    Object &operator=(const Object &) = delete;
    Object(Object &&) = delete;
    Object &operator=(Object &&) = delete;

    ~Object();

    std::string GetRawobj()
    {
      return obj_;
    }

   private:
    std::string obj_;
    uint32_t obj_idx_;
    LockFreePool *pool_ptr_;
  };

 public:
  LockFreePool(uint32_t size, std::function<std::string()> initializer);
  LockFreePool() = delete;
  LockFreePool(const LockFreePool &) = delete;
  LockFreePool &operator=(const LockFreePool &) = delete;
  LockFreePool(LockFreePool &&) = delete;
  LockFreePool &operator=(LockFreePool &&) = delete;

  ~LockFreePool();

  Object Getobj();

  void StartEvictionThread();

 protected:
  void EvictionThread();

  uint32_t size_;
  std::vector<std::string> objs_pool_;

  // track free obj indexes for creating new obj, and the num is also the real
  // obj index in `objs_` vector.
  std::vector<uint32_t> free_obj_indexes_;

  // obj initializer to create new obj object.
  std::function<std::string()> obj_initializer_;

  // available obj index queue used for geting a available obj in concurrent.
  boost::lockfree::queue<uint32_t, boost::lockfree::fixed_sized<true>> available_objs_queue_;

  // the available obj count of the current pool.
  std::atomic_uint32_t available_objs_cnt_ = 0;

  // track the total obj count of the current pool.
  std::atomic_uint32_t total_objs_cnt_ = 0;

  // a mutex to protect `objs_pool_` when creating new obj.
  std::mutex mutex_;

  std::atomic_bool shutdown = false;
};

LockFreePool::Object::~Object()
{
  if (!obj_.empty())
  {
    pool_ptr_->available_objs_queue_.push(obj_idx_);
    pool_ptr_->available_objs_cnt_++;
    std::cout << "objPool release obj, obj_idx: " << obj_idx_;
  }
}

LockFreePool::LockFreePool(uint32_t size, std::function<std::string()> initializer) : available_objs_queue_(size)
{
  size_ = size;
  obj_initializer_ = initializer;
  objs_pool_.reserve(size);
  objs_pool_.resize(size);
  for (uint32_t i = 0; i < size; i++)
  {
    free_obj_indexes_.push_back(i);
  }
  std::reverse(free_obj_indexes_.begin(), free_obj_indexes_.end());
}

LockFreePool::~LockFreePool()
{
  shutdown = true;
  available_objs_queue_.consume_all([](uint32_t &) {});
  objs_pool_.clear();
}

/**
  Get an available obj, or create a new one if none exists.

  The returned `obj` will automatically push back obj index to
  `objPool` when it goes out of scope, so that `available_objs_queue_`
  could get an available obj.

  Note: `Getobj` method will pop a obj idx from queue in turn that similar
  with Round‑Robin, so in theory, every obj will be used once in one loop.
*/
LockFreePool::Object LockFreePool::Getobj()
{
  uint32_t obj_idx = -1;
  while (!available_objs_queue_.pop(obj_idx))
  {
    // create new obj when there no available one.
    if (total_objs_cnt_ < size_)
    {
      // guarantee `total_objs_cnt_ < size_` is safe in concurrent cases.
      std::unique_lock<std::mutex> lock_guard(mutex_);
      if (total_objs_cnt_ < size_)
      {
        // pop a available idx to create new obj
        auto idx = free_obj_indexes_.back();
        free_obj_indexes_.pop_back();

        // insert a new obj at the idx position of obj vector
        objs_pool_[idx] = obj_initializer_();

        std::string obj = objs_pool_.at(idx);
        std::cout << "objPool create new obj, obj_idx: " << total_objs_cnt_.load();
        total_objs_cnt_++;
        return Object(obj, idx, this);
      }
    }
    // wait a while for next loop to get available obj.
    std::cout << "objPool wait for an available obj";
    std::this_thread::sleep_for(std::chrono::microseconds{ 100 * 1000 });
  }
  available_objs_cnt_--;
  std::cout << "objPool use an available obj, obj_idx: " << obj_idx;
  return Object(objs_pool_.at(obj_idx), obj_idx, this);
}

void LockFreePool::StartEvictionThread()
{
  std::thread _obj_pool_eviction_thread(
      [this]()
      {
        pthread_setname_np(pthread_self(), "objPoolEvictionThread");
        EvictionThread();
      });
  _obj_pool_eviction_thread.detach();
}

void LockFreePool::EvictionThread()
{
  while (!shutdown.load())
  {
    uint32_t obj_idx = -1;
    if (available_objs_queue_.pop(obj_idx))
    {
      if (available_objs_cnt_ <= 1)
      {
        std::cout << "objPool pop a last one obj, obj_idx: " << obj_idx << ", skip eviction";
        available_objs_queue_.push(obj_idx);
        goto wait;
      }

      // evict this available obj and close it
      {
        std::unique_lock<std::mutex> lock_guard(mutex_);
        auto to_be_removed_obj = objs_pool_.at(obj_idx);
        // if obj is unavailable, skip eviction in this loop
        if (to_be_removed_obj.empty())
          goto wait;

        // clear the obj
        objs_pool_[obj_idx] = nullptr;

        total_objs_cnt_--;
        available_objs_cnt_--;
        free_obj_indexes_.push_back(obj_idx);
        std::cout << "objPool evict a obj, obj_idx: " << obj_idx
                  << ", available obj cnt: " << available_objs_cnt_.load();
      }
    }

  wait:
    // wait 30 seconds for next loop to get available obj to check
    std::this_thread::sleep_for(std::chrono::microseconds{ 30 * 1000 * 1000 });
  }
}
// NOLINTEND