#include <benchmark/benchmark.h>

struct T {
  std::string a;
  std::string b;
  //   T(std::string a, std::string b) : a(std::move(a)), b(std::move(b)){};
  T(std::string a, std::string b) : a(a), b(b){};  // better performance
};

static void Vector_PushBack(benchmark::State& state) {
  std::vector<T> nums;
  nums.reserve(state.max_iterations);
  for (auto _ : state) {
    nums.push_back(T("a", "b"));
  }
}
BENCHMARK(Vector_PushBack);

static void Vector_EmplaceBack(benchmark::State& state) {
  std::vector<T> nums;
  nums.reserve(state.max_iterations);
  for (auto _ : state) {
    nums.emplace_back(T("a", "b"));
  }
}
BENCHMARK(Vector_EmplaceBack);

static void Vector_EmplaceBack2(benchmark::State& state) {
  std::vector<T> nums;
  // before reserve: Vector_EmplaceBack2       98.2 ns         98.2 ns     10440242
  nums.reserve(state.max_iterations);
  // after reserve: Vector_EmplaceBack2       35.1 ns         35.1 ns     19976003
  for (auto _ : state) {
    nums.emplace_back("a", "b");
  }
}
BENCHMARK(Vector_EmplaceBack2);

BENCHMARK_MAIN();