#include <_types/_uint64_t.h>
#include <gtest/gtest.h>
#include <modern-cpp-knowledge/output_container.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

TEST(ContainersTest, VectorTest)  // NOLINT
{
  // A vector is a sequence of elements of a given type. The elements are stored contiguously in memory.
  std::vector<int> v{ 1, 2, 3 };
  ASSERT_EQ(v.at(2), 3);
  ASSERT_THROW(v.at(3), std::out_of_range);  // NOLINT

  v.push_back(4);
  ASSERT_EQ(v[3], 4);

  std::sort(v.begin(), v.end(), std::greater<>());
  ASSERT_EQ(v, std::vector<int>({ 4, 3, 2, 1 }));

  // back_inserter, unique_copy(移除相邻重复元素)
  v.push_back(1);
  std::vector<int> v2 = {};
  std::unique_copy(v.begin(), v.end(), std::back_inserter(v2));
  ASSERT_EQ(v2, std::vector<int>({ 4, 3, 2, 1 }));
  std::cout << v2 << '\n';

  // https://yasenh.github.io/post/cpp-diary-1-emplace_back/
  // push_back: Adds a new element at the end of the container, after its current last element. The content of val is copied
  // (or moved) to the new element.
  // 1. A constructor will be called to create a temporary object.
  // 2. A copy of the temporary object will be constructed in the memory for the container. Note that the move constructor
  // will be called if exist because the temporary object is an rvalue, otherwise the copy constructor should be called.
  // 3. The destructor will be called to destroy the temporary object after copy.
  //
  // emplace_back: Inserts a new element at the end of the container, right after its current last element. This new element
  // is constructed in place using args as the arguments for its constructor.
  // the emplacement function avoids constructing and destructing temporary objects.
}

TEST(ContainersTest, ListTest)  // NOLINT
{
  // list : doubly-linked list
  std::list<int> l{ 1, 2, 3 };
  l.insert(l.begin(), 0);
  ASSERT_EQ(l.front(), 0);
  l.erase(l.begin());
  ASSERT_EQ(l.front(), 1);
  std::cout << l << '\n';
}

TEST(ContainersTest, MapTest)  // NOLINT
{
  // The standard library offers a search tree (a redblack tree) called map
  // In other contexts, a map is known as an associative array or a dictionary. It is implemented as a balanced binary tree.
  std::map<std::string, int> m{ { "a", 1 }, { "b", 2 }, { "c", 3 } };
  ASSERT_EQ(m.at("b"), 2);
  m["b"] = 22;  // NOLINT
  ASSERT_EQ(m["b"], 22);
  const auto p = m.insert_or_assign("b", 4);
  ASSERT_EQ(p.second, 0);
  ASSERT_EQ(m["b"], 4);
  std::cout << m << '\n';
}

TEST(ContainersTest, UnorderedMapTest)  // NOLINT
{
  // The standard-library hashed containers are referred to as ‘‘unordered’’ because they don’t require an ordering function
  std::unordered_map<std::string, int> _map{ { "a", 1 }, { "b", 2 }, { "c", 3 } };
  ASSERT_EQ(_map["b"], 2);

  ASSERT_EQ(_map.find("unknown"), _map.end());
  ASSERT_EQ(_map.find("b")->second, 2);

  auto res = std::find_if(_map.begin(), _map.end(), [](const auto& p) { return p.second == 2; });
  ASSERT_EQ(res->first, "b");
  std::cout << _map << '\n';
}

TEST(ContainersTest, ArrayTest)  // NOLINT
{
  // An array, defined in <array>, is a fixed-size sequence of elements of a given type where the number of elements is
  // specified at compile time. Thus, an array can be allocated with its elements on the stack, in an object, or in static
  // storage
  std::array<int, 3> a = { 1, 2, 3 };
  ASSERT_EQ(a[2], 3);
  std::cout << a << '\n';
}

TEST(ContainersTest, BitsetTest)  // NOLINT
{
  // A fixed-size sequence of N bits
  std::bitset<8> bs1{ "00000000" };  // NOLINT
  std::bitset<8> bs2{ "11111111" };  // NOLINT
  std::bitset<8> bs3 = bs1 | bs2;    // NOLINT
  uint64_t const ullong = bs3.to_ullong();
  ASSERT_EQ(ullong, 255);
  auto bs3_string = bs3.to_string();
  ASSERT_EQ(bs3_string, "11111111");
  std::cout << bs1 << '\n';
}

class A
{
 public:
  explicit A(int i) : _i(i)
  {
  }
  ~A() = default;
  // copy constructor
  A(const A& a) : _i(a._i)
  {
    std::cout << "copy constructor" << std::endl;
  }
  // copy assignment
  auto operator=(const A& a) -> A&
  {
    std::cout << "copy assignment" << std::endl;
    _i = a._i;
    return *this;
  }
  // move constructor
  A(A&& a) noexcept : _i(a._i)
  {
    std::cout << "move constructor" << std::endl;
    a._i = 0;
  }
  // move assignment
  auto operator=(A&& a) noexcept -> A&
  {
    std::cout << "move assignment" << std::endl;
    _i = a._i;
    a._i = 0;
    return *this;
  }

 private:
  int _i;
};
void test_pair(std::vector<std::pair<A, A>>& res)
{
  std::cout << "start" << std::endl;

  auto p = std::make_pair(A(1), A(2));
  std::cout << "emplace_back" << std::endl;
  res.emplace_back(p);
  std::cout << "push_back" << std::endl;
  res.push_back(p);

  std::cout << "end" << std::endl;
}

TEST(ContainersTest, Pair_Copy_Constructor_Test)  // NOLINT
{
  std::vector<std::pair<A, A>> res;
  test_pair(res);
}

TEST(ContainersTest, Pair_Tuple_Test)  // NOLINT
{
  // Often, we need some data that is just data; that is, a collection of values, rather than an object of a class with a
  // well-defined semantics and an invariant for its value.
  auto res1 = std::make_pair("1", 2);
  auto res2 = std::make_pair("1", 2);
  ASSERT_EQ(res1, res2);
  auto res3 = std::make_pair("1", 3);
  ASSERT_TRUE(res1 < res3);
  std::cout << res1 << '\n';

  auto t1 = std::make_tuple(1, 2.3, "456");  // NOLINT
  ASSERT_EQ(std::get<0>(t1), 1);
  ASSERT_EQ(std::get<1>(t1), 2.3);
  ASSERT_EQ(std::get<2>(t1), "456");
  std::cout << t1 << '\n';
}
