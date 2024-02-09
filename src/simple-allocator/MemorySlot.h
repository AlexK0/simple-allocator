// Simple Allocator 2024
#ifndef MEMORYSLOT_H
#define MEMORYSLOT_H
#include <new>

#include "MemoryBlock.h"
#include "SimpleAllocatorTraits.h"

class MemorySlot {
public:
  constexpr MemorySlot() = default;

  MemoryBlock *GetNext() noexcept {
    if (next_) {
      auto *next = next_;
      next_ = next->next_;
      return MemoryBlock::FromUserMemory(next);
    }
    return nullptr;
  }

  void AddNext(MemoryBlock *memory_block) noexcept {
    next_ = new (memory_block->UserMemoryBegin()) MemorySlot{next_};
  }

private:
  constexpr explicit MemorySlot(MemorySlot *next) noexcept
    : next_{next} {}

  MemorySlot *next_{nullptr};
};

#endif // MEMORYSLOT_H
