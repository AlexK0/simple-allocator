// Simple Allocator 2024
#include "Common.h"

#include <benchmark/benchmark.h>
#include <map>
#include <optional>

template<AllocatorType ALLOCATOR_TYPE>
static void Map_Insert(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::map<int64_t, int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();

    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->insert({i, i});
    }
  }
}

BENCHMARK(Map_Insert<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Map_Insert<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void Map_Erase(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::map<int64_t, int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->insert({i, i});
    }
    state.ResumeTiming();

    while (!container->empty()) {
      container->erase(container->begin());
    }
  }
}

BENCHMARK(Map_Erase<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Map_Erase<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void Map_InsertErase(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::map<int64_t, int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();

    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->insert({i, i});
      if (i % 2) {
        container->erase(i);
      }
    }
  }
}

BENCHMARK(Map_InsertErase<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Map_InsertErase<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void Map_Find(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::map<int64_t, int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->insert({i, i});
    }
    state.ResumeTiming();

    uint64_t sum = 0;
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      sum += static_cast<uint64_t>(container->find(i)->second);
    }

    benchmark::DoNotOptimize(sum);
  }
}

BENCHMARK(Map_Find<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Map_Find<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

BENCHMARK_MAIN();
