#include <gtest/gtest.h>

#include <cstdio>

// different C library implementations, such as the GNU C library (glibc), dietlibc, or musl.

// NOLINTBEGIN

TEST(CLibTest, WriteToSizedBuffer)
{
  // https://cplusplus.com/reference/cstdio/snprintf/
  // the content is stored as a C string in the buffer pointed by s (taking n as the maximum buffer capacity to fill).
  char buffer[100];
  snprintf(buffer, 100, "Hello World: %s !", "C");
  puts(buffer);
  islower('a');
}

// %s 会输出整个字符串，直到遇到 \0。
// %.*s 允许你指定输出的字符数，可以输出字符串的前 N 个字符。
TEST(CLibTest, PrintString)
{
  const char* str = "Hello, World!";

  // 使用 %s 输出整个字符串
  printf("%s\n", str);  // 输出: Hello, World!

  // 使用 %.*s 输出前 5 个字符
  int length = 5;
  printf("%.*s\n", length, str);  // 输出: Hello
}

// NOLINTEND
