#include <benchmark/benchmark.h>

#include <random>
#include <unordered_map>

struct T {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  uint64_t d;
};

static int max_size = 10000;

static void unordered_map_find(benchmark::State& state) {
  std::unordered_map<uint64_t, T> nums;
  std::random_device seed;
  std::mt19937 engine(seed());
  std::uniform_int_distribution<int> dist(0, max_size);
  for (uint64_t i = 0; i < state.max_iterations; i++) {
    nums[dist(engine)] = T{ i, i, i, i };
  }

  for (auto _ : state) {
    nums.find(dist(engine));
  }
}
BENCHMARK(unordered_map_find);

static void unordered_map_erase(benchmark::State& state) {
  std::unordered_map<uint64_t, T> nums;
  std::random_device seed;
  std::mt19937 engine(seed());
  std::uniform_int_distribution<int> dist(0, max_size);
  for (uint64_t i = 0; i < state.max_iterations; i++) {
    nums[dist(engine)] = T{ i, i, i, i };
  }

  for (auto _ : state) {
    nums.erase(dist(engine));
  }
}
BENCHMARK(unordered_map_erase);

static void unordered_map_find_erase(benchmark::State& state) {
  std::unordered_map<uint64_t, T> nums;
  std::random_device seed;
  std::mt19937 engine(seed());
  std::uniform_int_distribution<int> dist(0, max_size);
  for (uint64_t i = 0; i < state.max_iterations; i++) {
    nums[dist(engine)] = T{ i, i, i, i };
  }

  for (auto _ : state) {
    auto it = nums.find(dist(engine));
    if (it != nums.end())
      nums.erase(it);
  }
}
BENCHMARK(unordered_map_find_erase);

BENCHMARK_MAIN();