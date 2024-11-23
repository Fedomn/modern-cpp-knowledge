// NOLINTBEGIN
#include <gtest/gtest.h>

template<typename T>
class Foo {
 public:
  explicit Foo(T var) : var_(var) {
  }
  std::string print() {
    return std::to_string(var_);
  }

 private:
  T var_;
};

// Specialized templated class, specialized on the long type.
template<>
class Foo<long> {
 public:
  explicit Foo(long var) : var_(var) {
  }
  std::string print() const {
    return "hello float! " + std::to_string(var_);
  }

 private:
  long var_;
};

// Template parameters can also be values.
template<int T>
class Bar {
 public:
  explicit Bar(int var) : var_(var) {
  }
  std::string print() {
    return std::to_string(var_);
  }

 private:
  int var_;
};

template<>
class Bar<10> {
 public:
  explicit Bar(int var) : var_(var) {
  }
  std::string print() {
    return "hello int 10! " + std::to_string(var_);
  }

 private:
  int var_;
};

template<bool T>
int add3(int a) {
  if (T) {
    return a + 3;
  }

  return a;
}

TEST(TemplatedTest, ClassTemplateTest) {
  auto f1 = Foo<int>(1);
  ASSERT_EQ(f1.print(), "1");

  auto f2 = Foo<long>(2);
  ASSERT_EQ(f2.print(), "hello float! 2");

  auto b1 = Bar<1>(1);
  ASSERT_EQ(b1.print(), "1");

  auto b2 = Bar<10>(1);
  ASSERT_EQ(b2.print(), "hello int 10! 1");

  EXPECT_EQ(add3<true>(3), 6);
  EXPECT_EQ(add3<false>(3), 3);
}

template<typename T>
T Add(T a, T b) {
  return a + b;
}
TEST(TemplatedTest, FunctionTemplateTest) {
  Add(1, 2);
  EXPECT_EQ(3, Add(1, 2));
  EXPECT_EQ("12", Add(std::string("1"), std::string("2")));
}

class Base {
 public:
  void display() {
    std::cout << "Base display" << std::endl;
  }
};

class Derived : public Base {
 public:
  template<typename T>
  void process(T value) {
    std::cout << "Processing value: " << value << std::endl;
  }

  void process(int value) {
    std::cout << "Processing2 value: " << value << std::endl;
  }

  void callProcess() {
    this->template process<int>(42);
    this->template process<double>(3.14);
    this->process(2);
  }
};

TEST(TemplatedTest, TemplateMemberFunctionTest) {
  Derived d;
  d.display();
  d.callProcess();
}

// NOLINTEND