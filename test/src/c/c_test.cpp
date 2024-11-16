#include <gtest/gtest.h>

// NOLINTBEGIN

// Pointers are considered scalars:
// arithmetic operations are defined for them, offset additions and subtractions they
// have state and they have a dedicated “null” state.

// Array:
// We already have seen that a valid pointer holds the address of an object of its reference type, but actually C assumes
// more than that, A valid pointer refers to the first element of an array of the reference type.
TEST(CTest, LongArrayTest)
{
  const int ARRAY_SIZE = 5;
  // 分配内存空间存储 long 类型的数组, 或者像下面的 double 数组初始化分配在栈上
  long* longArray = new long[ARRAY_SIZE];
  for (int i = 0; i < ARRAY_SIZE; ++i)
  {
    longArray[i] = i * 1000L;
  }
  for (int i = 0; i < ARRAY_SIZE; ++i)
  {
    EXPECT_EQ(longArray[i], i * 1000L);
  }
  delete[] longArray;

  // pointer arithmetic operations
  double A[4] = { 0.0, 1.0, 2.0, -3.0 };
  double* p = &A[1];
  double* p1 = p - 1;
  // p1+i is a pointer that points to the `i` element in the array
  double* p2 = p + 1;
  // evaluates to an integer value their distance apart in number of element
  EXPECT_EQ(p2 - p1, 2);
}

TEST(CTest, StringArrayTest)
{
  const int NUM_STRINGS = 3;
  const int MAX_STRING_LEN = 100;
  // 分配内存空间存储字符串数组（字符指针数组）
  char** stringArray = new char*[NUM_STRINGS];

  // 为每个字符串分配内存空间
  for (int i = 0; i < NUM_STRINGS; ++i)
  {
    stringArray[i] = new char[MAX_STRING_LEN];
  }
  // 将字符串的地址赋值给字符指针数组的相应元素
  strcpy(stringArray[0], "Hello");
  strcpy(stringArray[1], "World");
  strcpy(stringArray[2], "!");

  // 输出字符串数组
  for (int i = 0; i < NUM_STRINGS; ++i)
  {
    std::cout << stringArray[i] << std::endl;
  }

  // 释放内存空间
  for (int i = 0; i < NUM_STRINGS; ++i)
  {
    delete[] stringArray[i];
  }
  delete[] stringArray;
}

TEST(CTest, StructTest) {
  // Pointers as opaque types: they do not directly contain the information that we are interested in: rather, they refer, or
  // point, to the data. C’s syntax for pointers always has the peculiar *
  struct bird
  {
    char const* a;
    char const* b;
  };

  // the member b1.a refers to an entity "a" is located outside the box and is not considered part of the
  // struct itself.
  bird b1 = { "a", "b" };
  EXPECT_EQ(b1.a, "a");
}

// void *
// Any object pointer converts to and from void*.
// Think of a void* pointer that holds the address of an existing object as a pointer into a storage instance that holds the
// object.
TEST(CTest, VoidTest) {
  char32_t a;
}

// dynamic allocation:
//
TEST(CTest, DynamicAllocationTest)
{
  int* intVec = (int*)malloc(sizeof(int[10]));
  for (int i = 0; i < 10; ++i)
  {
    intVec[i] = i;
  }
  EXPECT_EQ(intVec[0], 0);
  EXPECT_EQ(intVec[9], 9);
  free(intVec);
}

// NOLINTEND