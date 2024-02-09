// Simple Allocator 2024
#ifndef BENCHMARKS_COMMON_H
#define BENCHMARKS_COMMON_H

void EnableBenchmarkAllocator(bool use_simple_allocator) noexcept;
void DisableBenchmarkAllocator() noexcept;

enum AllocatorType { SIMPLE_ALLOCATOR, VANILLA_MALLOC };

template<AllocatorType ALLOCATOR_TYPE>
class ScopedBenchmarkAllocatorReplacement {
public:
  ScopedBenchmarkAllocatorReplacement() noexcept {
    EnableBenchmarkAllocator(ALLOCATOR_TYPE == SIMPLE_ALLOCATOR);
  }

  ~ScopedBenchmarkAllocatorReplacement() noexcept {
    DisableBenchmarkAllocator();
  }
};

#endif // BENCHMARKS_COMMON_H
