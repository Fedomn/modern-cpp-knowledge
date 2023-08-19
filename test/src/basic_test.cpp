#include <gtest/gtest.h>

TEST(BasicTest, VarsSize)
{
  //  A char variable is of the natural size to hold a character on a given machine (typically an 8-bit byte), and the sizes
  //  of other types are quoted in multiples of the size of a char
  EXPECT_EQ(sizeof(char), 1);
  EXPECT_EQ(sizeof(short), 2);
  EXPECT_EQ(sizeof(int), 4);
  EXPECT_EQ(sizeof(long), 8);
  EXPECT_EQ(sizeof(float), 4);
  EXPECT_EQ(sizeof(double), 8);
  EXPECT_EQ(sizeof(bool), 1);
  EXPECT_EQ(sizeof(std::string), 24);
  EXPECT_EQ(sizeof(std::vector<int>), 24);

  //  narrowing conversions, such as double to int and int to char are allowed and implicitly applied. The problems caused by
  //  implicit narrowing conversions is a price paid for C compatibility
  //  int i1 = 5.6;
  //  EXPECT_EQ(i1, 5);
}

TEST(BasicTest, BasicVars)
{
  int a = 1;
  double b = 2.0;
  char c = 'c';
  std::string d = "af";
  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 2.0);
  EXPECT_EQ(c, 'c');
  EXPECT_EQ(d, "af");

  // array
  std::string cars[4] = { "Volvo", "BMW", "Ford", "Mazda" };
  EXPECT_EQ(cars[0], "Volvo");
  int myNum[] = { 10, 20, 30 };
  for (auto& item : myNum)
  {
    item = 1;
  }
  EXPECT_EQ(myNum[1], 1);

  // Structures (also called structs) are a way to group several related variables into one place. Each variable in the
  // structure is known as a member of the structure.
  struct myStructure
  {
    int myNum;
    std::string myString;
  };
  myStructure myObj = { 5, "some string" };
  EXPECT_EQ(myObj.myNum, 5);
  EXPECT_EQ(myObj.myString, "some string");

  // pointer
  std::string food = "Pizza";
  std::string* ptr = &food;
  EXPECT_EQ(*ptr, "Pizza");
  *ptr = "Hamburger";
  EXPECT_EQ(food, "Hamburger");
}

TEST(BasicTest, BasicIf)
{
  std::vector<int> vec = { 1, 2, 3, 4 };
  if (const auto itr = std::find(vec.begin(), vec.end(), 3); itr != vec.end())
  {
    *itr = 4;
  }
  EXPECT_EQ(vec, std::vector<int>({ 1, 2, 4, 4 }));
}

std::tuple<int, double, std::string> f()
{
  return std::make_tuple(1, 2.3, "456");
}
TEST(BasicTest, BasicStructuredBinding)
{
  auto [a, b, c] = f();
  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 2.3);
  EXPECT_EQ(c, "456");
}

// 利用 constexpr 代码在编译时就完成分支判断
template<typename T>
auto print_type_info(const T& t)
{
  if constexpr (std::is_integral_v<T>)
  {
    return t + 1;
  }
  else
  {
    return t + 0.001;
  }
}
TEST(BasicTest, BasicControlFlow)
{
  EXPECT_EQ(print_type_info(5), 6);
  EXPECT_EQ(print_type_info(3.14), 3.141);

  std::vector<int> vec = { 1, 2, 3, 4 };
  for (auto& element : vec)
  {
    element += 1;  // writeable
  }
  EXPECT_EQ(vec, std::vector<int>({ 2, 3, 4, 5 }));
}

TEST(BasicTest, Reference)
{
  // As mentioned, parameters of fundamental types (such as int, double) are passed-by-value. That is, a clone copy is used
  // inside the function. Change to the cloned copy inside the function has no side-effect to the caller's copy.
  // Nonetheless, you can pass a fundamental type parameter by reference via the so-called reference parameter denoted by &.
  auto reference = [](std::string& x) { x += "1"; };
  std::string str = "2";
  reference(str);
  EXPECT_EQ(str, "21");
}

// see more in CPP11.md and 03-runtime.md
// C++11引入解决大量历史问题：std::string 之类的开销
//
// lvalue: is a persistent object that still exists after an expression (not necessarily an assignment expression).
// rvalue: the value on the right refers to the temporary object that no longer exists after the expression ends.
//
// Rvalue reference to T, is created with the syntax T&&
TEST(BasicTest, RvalueReference_And_LvalueReference)
{
  std::string lv1 = "string, ";  // lv1 is a lvalue
  // std::string&& r1 = lv1;     // illegal, rvalue can't ref to lvalue
  std::string&& rv1 = std::move(lv1);  // legal, std::move can convert lvalue to rvalue
  EXPECT_EQ(rv1, "string, ");

  const std::string& lv2 = lv1 + lv1;  // legal, const lvalue reference can extend temp variable's lifecycle

  std::string&& rv2 = lv1 + lv2;  // legal, rvalue ref extend lifecycle
  rv2 += "string";                // legal, non-const reference can be modified
  EXPECT_EQ(rv2, "string, string, string, string");
}

TEST(BasicTest, PerfectForwarding)
{
  // 也被称为 universal references
  // A forwarding reference is created with the syntax T&& where T is a template type parameter, or using auto&&.
  // This enables perfect forwarding: the ability to pass arguments while maintaining their value category
  // see more in CPP11.md

  int x = 0;      // `x` is a lvalue of type `int`
  auto&& al = x;  // `al` is a lvalue of type `int&` -- binds to the lvalue, `x`
  auto&& ar = 0;  // `ar` is a lvalue of type `int&&` -- binds to the rvalue temporary, `0`
  EXPECT_EQ(al, 0);
  EXPECT_EQ(ar, 0);

  auto f = [](auto&& t) { return t; };
  int x2 = 0;
  f(0);  // T is int, deduces as f(int &&) => f(int&&)
  f(x);  // T is int&, deduces as f(int& &&) => f(int&)

  int& y = x2;
  f(y);  // T is int&, deduces as f(int& &&) => f(int&)
}
