// Simple Allocator 2024
#include "Common.h"

#include <benchmark/benchmark.h>
#include <optional>
#include <vector>

template<AllocatorType ALLOCATOR_TYPE>
static void Vector_PushBack(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::vector<int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->push_back(i);
    }
  }
}

BENCHMARK(Vector_PushBack<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Vector_PushBack<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

BENCHMARK_MAIN();
