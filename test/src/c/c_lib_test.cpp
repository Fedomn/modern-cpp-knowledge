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

TEST(CLibTest, StringCopy) {
  char* s = new char[100];
  strcpy(s, "hello");
  printf("%s\n", s);
  *s = '\0';
  printf("%s, len: %d\n", s, strlen(s));
  strcpy(s, "Hello, World!");
  printf("%s\n", s);

  char* s2 = "2:3-1684479283-9";
  int pos1 = 1, pos2 = 3, pos3 = 14, pos4 = 16;
  char* end1 = s2 + pos1;
  ASSERT_EQ(2, strtol(s2, &end1, 10));
  char* end2 = s2 + pos2;
  ASSERT_EQ(3, strtol(end1 + 1, &end2, 10));
  char* end3 = s2 + pos3;
  ASSERT_EQ(1684479283, strtol(end2 + 1, &end3, 10));
  char* end4 = s2 + pos4;
  ASSERT_EQ(9, strtol(end3 + 1, &end4, 10));
}

// NOLINTEND
