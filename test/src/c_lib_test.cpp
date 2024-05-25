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



// NOLINTEND
