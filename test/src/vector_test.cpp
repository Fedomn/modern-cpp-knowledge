#include "modern-cpp-knowledge/vector.h"

#include <gtest/gtest.h>

using namespace vector;

TEST(VectorTest, Test)
{
  auto vector = Vector<int>(3);
  vector[0] = 1;
  EXPECT_EQ(vector[0], 1);
  EXPECT_EQ(vector.size(), 3);
  EXPECT_EQ(vector[1], 0);

  auto vector2 = Vector<double>{ 1, 2, 3 };
  vector2[2] = 4.1;
  EXPECT_EQ(vector2[2], 4.1);
  EXPECT_EQ(vector2.size(), 3);
}