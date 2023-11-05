// NOLINTBEGIN
#include <gtest/gtest.h>

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
// NOLINTEND