// Simple Allocator 2024
#ifndef SIMPLEALLOCATORTRAITS_H
#define SIMPLEALLOCATORTRAITS_H
#include <cstddef>

class SimpleAllocatorTraits {
public:
  static constexpr size_t ALIGNMENT = 16;

  static_assert(ALIGNMENT && ((ALIGNMENT - 1) & ALIGNMENT) == 0, "power of 2 is expected");
};

#endif // SIMPLEALLOCATORTRAITS_H
