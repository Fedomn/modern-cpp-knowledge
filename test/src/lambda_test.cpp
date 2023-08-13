#include <gtest/gtest.h>

//[capture list] (parameter list) mutable(optional) exception attribute -> return type {
//  // function body
//}
TEST(LambdaTest, LambdaValueCapture)
{
  int value = 1;
  auto copy_value = [value] { return value; };
  value = 100;
  EXPECT_EQ(copy_value(), 1);
}

TEST(LambdaTest, LambdaReferenceCapture)
{
  int value = 1;
  auto copy_value = [&value] { return value; };
  value = 100;
  EXPECT_EQ(copy_value(), 100);
}

TEST(LambdaTest, LambdaMoveCapture)
{
  auto important = std::make_unique<int>(1);
  auto add = [v1 = 1, v2 = std::move(important)](int x, int y) -> int { return x + y + v1 + (*v2); };
  EXPECT_EQ(add(3, 4), 9);
}

TEST(LambdaTest, LambdaGeneric)
{
  auto generic = [](auto x, auto y) { return x + y; };
  EXPECT_EQ(generic(1, 2), 3);
  EXPECT_EQ(generic(1.1, 2), 3.1);
  EXPECT_EQ(generic(std::string("1"), std::string("2")), "12");
}

TEST(LambdaTest, LambdaStdFunction)
{
  std::function<int(int, int)> add = [](int x, int y) { return x + y; };
  EXPECT_EQ(add(1, 2), 3);
}

TEST(LambdaTest, LambdaStdBind)
{
  auto add = [](int x, int y) { return x + y; };
  auto add_1 = std::bind(add, std::placeholders::_1, 1);
  EXPECT_EQ(add_1(2), 3);
}