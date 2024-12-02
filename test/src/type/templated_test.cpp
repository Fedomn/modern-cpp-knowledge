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

// Forward declaration
template<typename Event, typename State, typename... States>
struct TransitionTrait;

// State machine definition
template<typename... States>
class StateMachine {
 public:
  using StateVarint = std::variant<States...>;
  template<typename State>
  explicit StateMachine(State&& initial) : current_state_(std::forward<State>(initial)) {
  }

  template<typename Event>
  void handle_event(const Event& event) {
    current_state_ = std::visit(
        [&event](auto& state) -> StateVarint {
          return TransitionTrait<Event, std::decay_t<decltype(state)>, States...>::next(event, state);
        },
        current_state_);
  }

  template<typename F>
  auto visit(F&& f) {
    return std::visit(std::forward<F>(f), current_state_);
  }

 private:
  StateVarint current_state_;
};

// Base trait with default behavior
template<typename Event, typename State, typename... States>
struct TransitionTrait {
  static std::variant<States...> next(const Event&, const State& state) {
    return state;
  }
};
struct Red {
  std::string msg = "stop";
};
struct Yellow {
  std::string msg = "ready";
};
struct Green {
  std::string msg = "go";
};
struct TimerExpired { };
using TrafficLight = StateMachine<Red, Yellow, Green>;

template<typename... States>
struct TransitionTrait<TimerExpired, Red, States...> {
  static std::variant<States...> next(const TimerExpired&, const Red&) {
    return Green{};
  }
};
template<typename... States>
struct TransitionTrait<TimerExpired, Green, States...> {
  static std::variant<States...> next(const TimerExpired&, const Green&) {
    return Yellow{};
  }
};
template<typename... States>
struct TransitionTrait<TimerExpired, Yellow, States...> {
  static std::variant<States...> next(const TimerExpired&, const Yellow&) {
    return Red{};
  }
};

TEST(TemplatedTest, StateMachineTest) {
  TrafficLight sm(Red{});

  for (int i = 0; i < 7; ++i) {
    sm.visit([](const auto& state) { std::cout << state.msg << std::endl; });
    sm.handle_event(TimerExpired{});
  }
}

// NOLINTEND