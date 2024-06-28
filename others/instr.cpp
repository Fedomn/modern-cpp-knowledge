#include <cstdint>
#include <cstdio>
// https://stackoverflow.com/questions/13772567/how-to-get-the-cpu-cycle-count-in-x86-64-from-c/51907627#51907627
#include <x86intrin.h>

struct tick_t {
    uint64_t val0;
    uint64_t val1;
};

static tick_t g_tick{};

extern "C" {
__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void*, void*) {
    g_tick.val0 = __rdtsc();
}

__attribute__((no_instrument_function))
void __cyg_profile_func_exit(void*, void*) {
    g_tick.val1 = __rdtsc();
}
}

void hello_world() {}

auto main() -> int {
    hello_world();
    auto res = g_tick.val1 - g_tick.val0;
    printf("val1: %ld, val2: %ld, delta clock cycle: %lu\n", g_tick.val1, g_tick.val0, res);
    return 0;
}