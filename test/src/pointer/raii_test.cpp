#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>

/// Resource Acquisition Is Initialization (RAII)
/// When we need pointer semantics:
/// - When we share an object, we need pointers (or references) to refer to the shared object, so a shared_ptr becomes the
/// obvious choice (unless there is an obvious single owner).
///
/// - When we refer to a polymorphic object, we need a pointer (or a reference) because we don’t know the exact type of the
/// object referred to (or even its size), so a unique_ptr becomes the obvious choice.
///
/// - A shared polymorphic object typically requires shared_ptrs.

TEST(RAIITest, PtrTest)  // NOLINT
{
  // unique_ptr ensures that its object is properly destroyed whichever way we exit f() (by throwing an exception, by
  // executing return, or by ‘‘falling off the end’’.
  auto ptr = std::make_unique<int>(1);
  EXPECT_EQ(*ptr, 1);
  EXPECT_EQ(ptr.get(), std::addressof(*ptr));
  EXPECT_EQ(ptr.get(), ptr.operator->());
  EXPECT_EQ(ptr.get(), &*ptr);
  EXPECT_EQ(ptr.get(), &ptr.operator*());
  auto l = ptr.get();
  auto r = ptr.release();
  EXPECT_EQ(l, r);
  EXPECT_EQ(ptr.get(), nullptr);
  ptr.reset(new int(2));
  EXPECT_EQ(*ptr, 2);
  ptr.reset();
  EXPECT_EQ(ptr.get(), nullptr);

  // The shared_ptr is similar to unique_ptr except that shared_ptrs are copied rather than moved.
  // The shared_ptrs for an object share ownership of an object and that object is destroyed when the last of its shared_ptrs
  // is destroyed.
  auto ptr1 = std::make_shared<int>(1);
  std::shared_ptr<int> const a = std::make_shared<int>();
  if (a == nullptr)
  {
    std::cout << "a is nullptr" << std::endl;
  }
  auto ptr2 = ptr1;
  EXPECT_EQ(ptr1, ptr2);
  EXPECT_EQ(ptr1.use_count(), 2);
  EXPECT_EQ(ptr2.use_count(), 2);
  ptr1.reset();
  EXPECT_EQ(ptr1, nullptr);
  EXPECT_EQ(ptr2.use_count(), 1);
  ptr2.reset();
  EXPECT_EQ(ptr2, nullptr);
}

class T
{
 public:
  T()
  {
    std::cout << "T()" << std::endl;
  };
  // copy constructor, see vector_test.cpp
  T(const T &) = delete;
  auto operator=(const T &) -> T & = delete;
  // move constructor, see vector_test.cpp
  auto operator=(T &&) -> T & = delete;
  T(T &&) = delete;
  ~T()
  {
    std::cout << "~T()" << std::endl;
  };
  // 分配在堆上(make_shared自动new)，智能指针自动释放(自动调用delete)
  static auto init1() -> std::shared_ptr<T>
  {
    return std::make_shared<T>();
  }
  static auto init2() -> std::optional<std::unique_ptr<T>>
  {
    return std::make_optional(std::make_unique<T>());
  }
  // 使用copy constructor，分配在栈上
  static auto init3() -> T
  {
    // compile error: 'T' has been explicitly marked deleted here
    // T t = T();
    // return t;

    // 编译器执行返回值优化，不会调用copy constructor
    // 返回值优化是一种编译器优化技术，它通过将函数的返回值直接构造在调用者分配的内存空间中，从而避免了额外的拷贝操作。
    return T();
  }
};

TEST(RAIITest, SharedPtrTest)  // NOLINT
{
  {
    std::cout << "start1" << std::endl;
    std::optional<std::shared_ptr<T>> t1 = T::init1();
    if (t1.has_value())
    {
      auto a = t1.value();
    }
    std::cout << "use_count: " << t1->use_count() << std::endl;
    std::cout << "end1" << std::endl;
  }
  {
    std::cout << "start2" << std::endl;
    std::optional<std::shared_ptr<T>> t2 = T::init2();
    if (t2.has_value())
    {
      auto a = t2.value();
    }
    std::cout << "use_count: " << t2->use_count() << std::endl;
    std::cout << "end2" << std::endl;
  }
  {
    std::cout << "start3" << std::endl;
    auto t3 = T::init3();
    std::cout << "end3" << std::endl;
  }

  struct C
  {
    int x;
    int y;
  };
  auto *c = new C{ 1, 2 };
  std::cout << c->x << std::endl;
  std::cout << c->y << std::endl;
}

// function(std::shared_ptr<int>& shr>:
// The above (& shr) suggests, that you might reseat the smart pointer in the method, but the method does not have any intent
// to do so.
// This magic is an overkill if you are only interested in the underlying resource of the shared pointer.
