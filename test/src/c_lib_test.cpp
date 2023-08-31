#include <gtest/gtest.h>

#include <cstdio>

TEST(CLibTest, WriteToSizedBuffer)
{
  // https://cplusplus.com/reference/cstdio/snprintf/
  // the content is stored as a C string in the buffer pointed by s (taking n as the maximum buffer capacity to fill).
  char buffer[100];
  snprintf(buffer, 100, "Hello World: %s !", "C");
  puts(buffer);
}