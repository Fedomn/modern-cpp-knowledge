
#include <gtest/gtest.h>

#include <cstring>

// https://icarus.cs.weber.edu/~dab/cs1410/textbook/8.Strings/c_string.html
// https://stackoverflow.com/questions/2037209/what-is-a-null-terminated-string
TEST(StringTest, CString_Null_Terminated)  // NOLINT
{
  // When the compiler processes a string literal, it adds the null termination character at the end of the quoted
  // characters, stores them in memory, and generates code based on the string's address.

  // If the null-terminator doesn't exist, c_str() is guaranteed to append it.
  // See https://cplusplus.com/reference/string/string/c_str/

  std::string my_string = "This is a sample text";
  std::cout << my_string << std::endl;
  my_string = "This is a sam\0ple text";  // check the \0
  std::cout << my_string << std::endl;

  std::cout << sizeof(my_string) << std::endl;
  std::cout << strlen(my_string.c_str()) << std::endl;
}