// Simple Allocator 2024
#include "Common.h"

#include <benchmark/benchmark.h>
#include <deque>
#include <optional>

template<AllocatorType ALLOCATOR_TYPE>
static void Deque_PushBack(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::deque<int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();

    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->push_back(i);
    }
  }
}

BENCHMARK(Deque_PushBack<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Deque_PushBack<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void Deque_PushBack_PopFront(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::deque<int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();

    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->push_back(i);
      if (i % 2) {
        container->pop_front();
      }
    }
  }
}

BENCHMARK(Deque_PushBack_PopFront<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Deque_PushBack_PopFront<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void Deque_PopFront(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::deque<int64_t>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      container->push_back(i);
    }
    state.ResumeTiming();

    while (!container->empty()) {
      container->pop_front();
    }
  }
}

BENCHMARK(Deque_PopFront<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(Deque_PopFront<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

BENCHMARK_MAIN();
