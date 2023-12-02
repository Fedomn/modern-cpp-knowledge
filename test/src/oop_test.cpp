// NOLINTBEGIN
#include <gtest/gtest.h>

#include <cstdio>
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

namespace OOP
{
  class Base
  {
   public:
    // `=0`表示该函数是一个纯虚函数。纯虚函数是一种特殊的虚函数，在基类中声明但没有提供实现。
    // 由于纯虚函数没有实现，所以基类对象不能直接实例化。派生类必须实现纯虚函数，否则它自身也会成为抽象类，
    // 即不能初始化构造，如下 SubClass(); 在 SubClass的foo注释后，会报错
    virtual void foo(int) = 0;
    // 表示该函数可以在派生类中被重写。虚函数允许在运行时根据对象的实际类型来调用相应的函数实现。
    virtual void boo();

    virtual void default_boo()
    {
      puts("Base::default_boo()");
    }

    virtual ~Base() = default;
  };
  void Base::boo()
  {
    puts("Base::boo()");
  }

  class SubClass : public Base
  {
   public:
    void foo(int) override{};  // explict override
    // void foo(double) override;  // compile error

    void boo() override;

    void default_boo() override;
  };
  void SubClass::boo()
  {
    puts("SubClass::boo()");
  }
  void SubClass::default_boo()
  {
    puts("SubClass::default_boo()");
  }
}  // namespace OOP

TEST(OOPTest, OOPVirtualFunctionOverride)
{
  using namespace OOP;
  auto a = SubClass();
  a.foo(1);
  a.boo();
  a.default_boo();
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