#include <gtest/gtest.h>

template<typename T>
class Foo
{
 public:
  explicit Foo(T var) : var_(var)
  {
  }
  std::string print()
  {
    return std::to_string(var_);
  }

 private:
  T var_;
};

// Specialized templated class, specialized on the long type.
template<>
class Foo<long>
{
 public:
  explicit Foo(long var) : var_(var)
  {
  }
  std::string print() const
  {
    return "hello float! " + std::to_string(var_);
  }

 private:
  long var_;
};

// Template parameters can also be values.
template<int T>
class Bar
{
 public:
  explicit Bar(int var) : var_(var)
  {
  }
  std::string print()
  {
    return std::to_string(var_);
  }

 private:
  int var_;
};

template<>
class Bar<10>
{
 public:
  explicit Bar(int var) : var_(var)
  {
  }
  std::string print()
  {
    return "hello int 10! " + std::to_string(var_);
  }

 private:
  int var_;
};

template<bool T>
int add3(int a)
{
  if (T)
  {
    return a + 3;
  }

  return a;
}

TEST(TemplatedTest, ClassTest)
{
  auto f1 = Foo<int>(1);
  ASSERT_EQ(f1.print(), "1");

  auto f2 = Foo<long>(2);
  ASSERT_EQ(f2.print(), "hello float! 2");

  auto b1 = Bar<1>(1);
  ASSERT_EQ(b1.print(), "1");

  auto b2 = Bar<10>(1);
  ASSERT_EQ(b2.print(), "hello int 10! 1");

  EXPECT_EQ(add3<true>(3), 6);
  EXPECT_EQ(add3<false>(3), 3);
}
