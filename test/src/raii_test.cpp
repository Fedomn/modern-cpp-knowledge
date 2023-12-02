#include <gtest/gtest.h>

#include <cstddef>
#include <memory>

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
  EXPECT_EQ(ptr.get(), ptr.release());
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
