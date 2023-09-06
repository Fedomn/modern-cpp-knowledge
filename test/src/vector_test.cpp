#include "modern-cpp-knowledge/vector.h"

#include <gtest/gtest.h>

using namespace vector;

TEST(VectorTest, Test)  // NOLINT
{
  auto vector = Vector<int>(3);
  vector[0] = 1;
  EXPECT_EQ(vector[0], 1);
  EXPECT_EQ(vector.size(), 3);
  EXPECT_EQ(vector[1], 0);

  auto vector2 = Vector<double>{ 1, 2, 3 };
  vector2[2] = 4.1;  // NOLINT
  EXPECT_EQ(vector2[2], 4.1);
  EXPECT_EQ(vector2.size(), 3);

  // copy constructor
  auto vector3 = Vector<int>{ vector };
  EXPECT_EQ(vector3[0], 1);
  // copy assignment
  Vector<int> vector4{};
  vector4 = vector3;
  EXPECT_EQ(vector4[0], 1);

  // move constructor
  auto vector5 = Vector<int>{ std::move(vector4) };
  EXPECT_EQ(vector5[0], 1);
  // move assignment
  Vector<int> vector6{};
  vector6 = std::move(vector5);
  EXPECT_EQ(vector6[0], 1);
}
