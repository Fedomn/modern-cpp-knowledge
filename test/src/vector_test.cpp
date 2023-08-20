#include "modern-cpp-knowledge/vector.h"

#include <gtest/gtest.h>

TEST(VectorTest, Test)
{
  auto vector = vector::Vector(3);
  vector[0] = 1;
  EXPECT_EQ(vector[0], 1);
  EXPECT_EQ(vector.size(), 3);
  EXPECT_EQ(vector[1], 0);

  auto vector2 = vector::Vector{ 1, 2, 3 };
  EXPECT_EQ(vector2[2], 3);
}