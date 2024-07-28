#include <gtest/gtest.h>

#ifndef __has_cpp_attribute
#define lifetime_bound
#elif __has_cpp_attribute(msvc::lifetimebound)
#define lifetime_bound [[msvc::lifetimebound]]
#elif __has_cpp_attribute(clang::lifetimebound)
#define lifetime_bound [[clang::lifetimebound]]
#elif __has_cpp_attribute(lifetimebound)
#define lifetime_bound [[lifetimebound]]
#else
#define lifetime_bound
#endif

class Host
{
 private:
  std::string_view value;

 public:
  explicit Host(std::string_view value) noexcept : value{ value }
  {
  }

  [[nodiscard]] auto get_value() const noexcept lifetime_bound->std::string_view
  {
    return value;
  }
};

auto get_host() -> std::string_view
{
  // auto host = Host{ "127.0.0.1" };
  auto host = std::make_unique<Host>("127.0.0.1");
  return host->get_value();
}

TEST(LifetimeTest, Basic)  // NOLINT
{
  EXPECT_EQ(get_host(), "127.0.0.1");
}