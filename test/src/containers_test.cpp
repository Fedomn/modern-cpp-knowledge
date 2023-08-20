#include <gtest/gtest.h>

#include <algorithm>
#include <list>
#include <map>
#include <vector>

TEST(ContainersTest, VectorTest)
{
  // A vector is a sequence of elements of a given type. The elements are stored contiguously in memory.
  std::vector<int> v{ 1, 2, 3 };
  ASSERT_EQ(v.at(2), 3);
  ASSERT_THROW(v.at(3), std::out_of_range);

  v.push_back(4);
  ASSERT_EQ(v[3], 4);

  std::sort(v.begin(), v.end(), std::greater<int>());
  ASSERT_EQ(v, std::vector<int>({ 4, 3, 2, 1 }));

  // back_inserter, unique_copy(移除相邻重复元素)
  v.push_back(1);
  std::vector<int> v2 = {};
  std::unique_copy(v.begin(), v.end(), std::back_inserter(v2));
  ASSERT_EQ(v2, std::vector<int>({ 4, 3, 2, 1 }));
}

TEST(ContainersTest, ListTest)
{
  // list : doubly-linked list
  std::list<int> l{ 1, 2, 3 };
  l.insert(l.begin(), 0);
  ASSERT_EQ(l.front(), 0);
  l.erase(l.begin());
  ASSERT_EQ(l.front(), 1);
}

TEST(ContainersTest, MapTest)
{
  // The standard library offers a search tree (a redblack tree) called map
  // In other contexts, a map is known as an associative array or a dictionary. It is implemented as a balanced binary tree.
  std::map<std::string, int> m{ { "a", 1 }, { "b", 2 }, { "c", 3 } };
  ASSERT_EQ(m.at("b"), 2);
  m["b"] = 22;
  ASSERT_EQ(m["b"], 22);
}

TEST(ContainersTest, UnorderedMapTest)
{
  // The standard-library hashed containers are referred to as ‘‘unordered’’ because they don’t require an ordering function
  std::unordered_map<std::string, int> m{ { "a", 1 }, { "b", 2 }, { "c", 3 } };
  ASSERT_EQ(m["b"], 2);

  ASSERT_EQ(m.find("unknown"), m.end());
  ASSERT_EQ(m.find("b")->second, 2);

  auto res = std::find_if(m.begin(), m.end(), [](const auto& p) { return p.second == 2; });
  ASSERT_EQ(res->first, "b");
}
