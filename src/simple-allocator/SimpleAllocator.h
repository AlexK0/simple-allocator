// Simple Allocator 2024
#ifndef SIMPLEALLOCATOR_H
#define SIMPLEALLOCATOR_H
#include "MemorySlot.h"
#include "MemoryTree.h"

#include <array>
#include <cstdint>

class SimpleAllocatorBase {
private:
  static constexpr size_t ConstExprLog2(size_t x) {
    return x == 1 ? 0 : 1 + ConstExprLog2(x >> 1);
  }

protected:
  static constexpr size_t GetSlotIndex(size_t size) noexcept {
    constexpr size_t alignment_shift = ConstExprLog2(SimpleAllocatorTraits::ALIGNMENT);
    return (size >> alignment_shift) - 1;
  }
};

class SimpleAllocator : SimpleAllocatorBase {
public:
  SimpleAllocator() = default;
  bool Init(void *buffer, size_t buffer_size) noexcept;

  void *Allocate(size_t size) noexcept;
  void Deallocate(void *ptr) noexcept;
  void *Reallocate(void *ptr, size_t new_size) noexcept;
  static size_t Size(void *ptr) noexcept;

private:
  uint8_t *CutBuffer(size_t size) noexcept;

  constexpr static size_t MAX_SLOT_SIZE_{16 * 1024};

  std::array<MemorySlot, GetSlotIndex(MAX_SLOT_SIZE_)> slots_{};
  MemoryTree memory_tree_;

  uint8_t *buffer_begin_{nullptr};
  uint8_t *buffer_end_{nullptr};
  uint8_t *current_{nullptr};
};

#endif // SIMPLEALLOCATOR_H
