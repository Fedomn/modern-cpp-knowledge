#include <gtest/gtest.h>

#include <array>
#include <string>
#include <thread>
#include <type_traits>
#include <variant>

TEST(BasicTest, VarsSize)  // NOLINT
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

TEST(BasicTest, Constants)  // NOLINT
{
  // const: meaning roughly ‘‘I promise not to change this value.’’ This is used primarily to specify interfaces, so that
  // data can be passed to functions without fear of it being modified. The compiler enforces the promise made by const.
  const int a = 1;

  // constexpr: meaning roughly ‘‘to be evaluated at compile time.’’ This is used primarily to specify constants, to allow
  // placement of data in read-only memory (where it is unlikely to be corrupted) and for performance
  constexpr int b = 2 * a;
  EXPECT_EQ(b, 2);

  //  一个 constexpr 变量是一个编译时完全确定的常数。
  //  一个 constexpr 函数至少对于某一组实参可以在编译期间产生一个编译期常数。
  //  一个 constexpr 函数不保证在所有情况下都会产生一个编译期常数（因而也是可以作为普通函数来使用的）。

  //  要检验一个 constexpr 函数能不能产生一个真正的编译期常量，可以把结果赋给一个 constexpr
  //  变量。成功的话，我们就确认了，至少在这种调用情况下，我们能真正得到一个编译期常量
  constexpr auto sqr = [](int n) { return n * n; };
  constexpr int i = sqr(3);
  EXPECT_EQ(i, 9);
}

TEST(BasicTest, BasicVars)  // NOLINT
{
  int const a = 1;
  double const b = 2.0;
  char const c = 'c';
  std::string const d = "af";
  EXPECT_EQ(a, 1);
  EXPECT_EQ(b, 2.0);
  EXPECT_EQ(c, 'c');
  EXPECT_EQ(d, "af");

  // array
  std::array<std::string, 4> const cars = { "Volvo", "BMW", "Ford", "Mazda" };
  EXPECT_EQ(cars[0], "Volvo");
  std::array<int, 4> myNum = { 10, 20, 30 };
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
  myStructure const myObj = { 5, "some string" };
  EXPECT_EQ(myObj.myNum, 5);
  EXPECT_EQ(myObj.myString, "some string");

  // pointer
  std::string food = "Pizza";
  std::string* ptr = &food;
  EXPECT_EQ(*ptr, "Pizza");
  *ptr = "Hamburger";
  EXPECT_EQ(food, "Hamburger");

  std::variant<int, std::string> myValue = { "2" };
  auto myValue1 = std::get<std::string>(myValue);
  EXPECT_EQ(myValue1, "2");
  myValue = 1;
  auto myValue2 = std::get<int>(myValue);
  EXPECT_EQ(myValue2, 1);
}

TEST(BasicTest, BasicTime)  // NOLINT
{
  auto t0 = std::chrono::high_resolution_clock::now();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  auto t1 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  EXPECT_GE(duration, 100);
}

TEST(BasicTest, BasicIf)  // NOLINT
{
  std::vector<int> vec = { 1, 2, 3, 4 };
  if (const auto itr = std::find(vec.begin(), vec.end(), 3); itr != vec.end())
  {
    *itr = 4;
  }
  EXPECT_EQ(vec, std::vector<int>({ 1, 2, 4, 4 }));
}

auto f() -> std::tuple<int, double, std::string>
{
  return std::make_tuple(1, 2.3, "456");
}
TEST(BasicTest, BasicStructuredBinding)  // NOLINT
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
TEST(BasicTest, BasicControlFlow)  // NOLINT
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

// Return type is `int`.
auto f(const int& i)
{
  return i;
}
// Return type is `const int&`.
auto g(const int& i) -> decltype(auto)
{
  return i;
}
TEST(BasicTest, DecltypeAutoTest)  // NOLINT
{
  // The decltype(auto) type-specifier also deduces a type like auto does. However, it deduces return types while keeping
  // their references and cv-qualifiers, while auto will not.
  int const x = 123;
  static_assert(!std::is_same<const int&, decltype(f(x))>::value);
  static_assert(std::is_same<int, decltype(f(x))>::value);
  static_assert(std::is_same<const int&, decltype(g(x))>::value);
}

TEST(BasicTest, CompareTest)  // NOLINT
{
  // unsigned int const i = 0;
  // signed int const j = -1;
  // ASSERT_TRUE(i < j);
  // ASSERT_EQ(static_cast<unsigned int>(j), 4294967295);
}
