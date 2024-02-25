
#include <gtest/gtest.h>

#include <string>

// In a struct all members are public by default; in a class, all members are private by default
// The base classes of a struct are public by default; the base classes of a class are private by default
TEST(ClassTest, DefaultVisibility)  // NOLINT
{
  struct A
  {
    int a;
  };

  struct B : A
  {
    int b;
  };

  auto b = B();
  b.a = 1;
  b.b = 2;

  class C
  {
   public:
    int c;
  };
  class D : C
  {
   public:
    int d;
  };
  auto d = D();
  d.d = 3;
  // d.c = 4; not allowed
}

// 何时使用 class 与 struct
// Use class if the class has an invariant; 类整体具有不可变性。
// use struct if the data members can vary independently 结构体的数据成员可以独立变化。

// class big six:
// Default constructor: X()
// Copy constructor: X(const X&)
// Copy assignment: X& operator=(const X&)
// Move constructor: X(X&&)
// Move assignment: X& operator=(X&&)
// Destructor: ~X()
//
// By default, the compiler will generate all six of these methods for you.
// `== default` : ask explicitly compiler to generate them
// `== delete` :  ask explicitly compiler to delete them

TEST(ClassTest, AutoGenerate)  // NOLINT
{
  class A
  {
   public:
    std::string name;
    std::map<int, int> rep;
  };

  A a;
  a.name = "name";
  a.rep[1] = 1;

  // the class A copy constructor works
  // because, it invokes the copy constructor for all members and all bases of the class.
  auto b = a;
  EXPECT_EQ(b.name, "name");
  std::swap(a, b);
  EXPECT_EQ(b.rep[1], 1);
}