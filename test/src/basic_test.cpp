#include <gtest/gtest.h>

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

  auto f = [](auto&& t) {};
  int x2 = 0;
  f(0);  // T is int, deduces as f(int &&) => f(int&&)
  f(x);  // T is int&, deduces as f(int& &&) => f(int&)

  int& y = x2;
  f(y);  // T is int&, deduces as f(int& &&) => f(int&)
}
