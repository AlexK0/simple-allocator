// Simple Allocator 2024
#include "Common.h"

#include <benchmark/benchmark.h>
#include <list>
#include <optional>
#include <string>

template<AllocatorType ALLOCATOR_TYPE>
static void ListHugeElement_PushBack(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::list<std::string>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();

    std::string huge_element(1024 * 16 + 1, 'a');
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      if (i % 4 == 1) {
        huge_element += 'b';
      }
      container->push_back(huge_element);
    }
  }
}

BENCHMARK(ListHugeElement_PushBack<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(ListHugeElement_PushBack<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void ListHugeElement_PushBack_PopFront(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::list<std::string>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    state.ResumeTiming();

    std::string huge_element(1024 * 16 + 1, 'a');
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      if (i % 4 == 1) {
        huge_element += 'b';
      }
      container->push_back(huge_element);
      if (i % 2) {
        container->pop_front();
      }
    }
  }
}

BENCHMARK(ListHugeElement_PushBack_PopFront<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(ListHugeElement_PushBack_PopFront<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

template<AllocatorType ALLOCATOR_TYPE>
static void ListHugeElement_PopFront(benchmark::State &state) {
  ScopedBenchmarkAllocatorReplacement<ALLOCATOR_TYPE> malloc_replacement;
  std::optional<std::list<std::string>> container;
  for (auto _ : state) {
    state.PauseTiming();
    container.emplace();
    std::string huge_element(1024 * 16 + 1, 'a');
    for (int64_t i = 0, size = state.range(0); i != size; ++i) {
      if (i % 4 == 1) {
        huge_element += 'b';
      }
      container->push_back(huge_element);
    }
    state.ResumeTiming();

    while (!container->empty()) {
      container->pop_front();
    }
  }
}

BENCHMARK(ListHugeElement_PopFront<SIMPLE_ALLOCATOR>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);
BENCHMARK(ListHugeElement_PopFront<VANILLA_MALLOC>)->RangeMultiplier(2)->Range(1 << 10, 1 << 15);

BENCHMARK_MAIN();
