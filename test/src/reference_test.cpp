#include <gtest/gtest.h>
#include <modern-cpp-knowledge/output_container.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

TEST(ReferenceTest, ReferencePassInParam)  // NOLINT
{
  // pointer param is value copy
  auto pointerValCopy = [](void* x)
  {
    auto* para = static_cast<std::string*>(x);
    std::cout << "reference: " << *para << std::endl;
    auto a = std::string("1234");
    para = &a;
    std::cout << "reference: " << *para << std::endl;
  };
  std::string str1 = "1";
  pointerValCopy(&str1);
  EXPECT_EQ(str1, "1");
  std::cout << "str1: " << str1 << std::endl;

  auto referenceValCopy = [](std::string& para)
  {
    std::cout << "reference: " << para << std::endl;
    auto a = std::string("1234");
    para = a;
    std::cout << "reference: " << para << std::endl;
  };
  std::string str2 = "2";
  referenceValCopy(str2);
  EXPECT_EQ(str2, "1234");
  std::cout << "str: " << str2 << std::endl;
}

TEST(ReferenceTest, Reference)  // NOLINT
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
TEST(ReferenceTest, RvalueReference_And_LvalueReference)  // NOLINT
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

// NOLINTBEGIN
std::string reference(int& input)
{
  static_assert(std::is_lvalue_reference<decltype(input)>::value);
  return "&";
}
std::string reference(int&& input)
{
  static_assert(std::is_rvalue_reference<decltype(input)>::value);
  return "&&";
}
TEST(ReferenceTest, RvalueReference_Overload)
{
  int x = 0;  // `x` is an lvalue of type `int`
  EXPECT_EQ(reference(42), "&&");
  EXPECT_EQ(reference(x), "&");
  EXPECT_EQ(reference(std::move(x)), "&&");

  int& xl = x;  // `xl` is an lvalue of type `int&`
  EXPECT_EQ(reference(xl), "&");
  EXPECT_EQ(reference(std::move(xl)), "&&");

  int&& xr = 0;  // `xr` is an lvalue of type `int&&` -- binds to the rvalue temporary, `0`
  EXPECT_EQ(reference(xr), "&");
  EXPECT_EQ(reference(std::move(xr)), "&&");
}
// NOLINTEND

TEST(ReferenceTest, PerfectForwarding)  // NOLINT
{
  // 也被称为 universal references
  // A forwarding reference is created with the syntax T&& where T is a template type parameter, or using auto&&.
  // This enables perfect forwarding: the ability to pass arguments while maintaining their value category
  // see more in CPP11.md

  int const x = 0;  // `x` is a lvalue of type `int`
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

TEST(ReferenceTest, MapMoveTest)  // NOLINT
{
  std::map<std::string, std::string> map1 = { { "a", "b" } };
  std::map<std::string, std::string> map2 = std::move(map1);
  EXPECT_EQ(map2["a"], "b");
  EXPECT_EQ(map1.size(), 0);

  std::unordered_map<std::string, std::string> map3 = { { "a", "b" } };
  std::unordered_map<std::string, std::string> map4 = std::move(map3);
  EXPECT_EQ(map4["a"], "b");
  EXPECT_EQ(map3.size(), 0);
}

TEST(ReferenceTest, MoveSemantics)  // NOLINT
{
  std::vector<int> int_array = { 1, 2, 3, 4 };
  // new lvalue -> stealing_ints
  std::vector<int> stealing_ints = std::move(int_array);
  // rvalue reference -> rvalue_stealing_ints, is a reference that refer to the data itself.
  std::vector<int>&& rvalue_stealing_ints = std::move(stealing_ints);

  // Note: stealing_ints still possible to access the data
  EXPECT_EQ(stealing_ints[1], 2);
  EXPECT_EQ(rvalue_stealing_ints[1], 2);

  auto move_add_three_and_print = [](std::vector<int>&& vec)
  {
    // seize the ownership of the vec passed in.
    std::vector<int> vec1 = std::move(vec);
    vec1.push_back(3);
    std::cout << vec1;
  };
  auto add_three_and_print = [](std::vector<int>&& vec)
  {
    // dose not seize the ownership of the vec, so it still be used in outside.
    vec.push_back(3);
    std::cout << vec;
  };

  std::vector<int> int_array2 = { 1, 2, 3, 4 };
  // pass rvalue reference into funciton, however rvalue is moved into new rvalue.
  // so, the real data no longer belongs to int_arrays.
  move_add_three_and_print(std::move(int_array2));
  // It would result in SegFault
  // EXPECT_EQ(int_array2[1], 2);

  std::vector<int> int_array3 = { 1, 2, 3, 4 };
  // pass rvalue reference into funciton, and function only treats the parameter as a reference.
  add_three_and_print(std::move(int_array3));
  // lvalue still owns the real data
  EXPECT_EQ(int_array3[1], 2);
}
