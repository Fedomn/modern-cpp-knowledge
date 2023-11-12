// NOLINTBEGIN
#include <gtest/gtest.h>

#include <iostream>

TEST(OOPTest, OOPConstructor)
{
  class Base
  {
   public:
    int v1;
    int v2;
    Base()
    {
      v1 = 1;
    }

    Base(int v) : Base()  // delegate Base() constructor
    {
      v2 = v;
    }
  };

  Base b(2);
  EXPECT_EQ(b.v1, 1);
  EXPECT_EQ(b.v2, 2);

  class SubClass : public Base
  {
   public:
    using Base::Base;  // inheritance constructor
  };

  SubClass s(3);
  EXPECT_EQ(s.v1, 1);
  EXPECT_EQ(s.v2, 3);
}

TEST(OOPTest, OOPVirtualFunctionOverride)
{
  class Base
  {
    virtual void foo(int);

   public:
    virtual ~Base() = default;
  };

  class SubClass : Base
  {
    virtual void foo(int) override;  // explict override
    // virtual void foo(double) override;  // compile error
  };
}

TEST(OOPTest, Deconstructor)
{
  class A
  {
   public:
    A()
    {
      std::cout << "A constructor" << std::endl;
    };
    ~A()
    {
      std::cout << "A deconstructor" << std::endl;
    };
  };

  {
    A a;
    std::cout << "scope ending, stack a should be deconstructed\n";
  }

  auto a = new A();
  std::cout << "manually delete heap a\n";
  delete a;

  std::cout << "\n";

  class B : A
  {
   public:
    B() = default;
    ~B() = default;
  };

  {
    std::cout << "inheritance default deconstructor begin\n";
    B b;
  }

  std::cout << "ending\n";
}

// NOLINTEND