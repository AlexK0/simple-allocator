// Simple Allocator 2024
#ifndef MEMORYBLOCK_H
#define MEMORYBLOCK_H
#include <cassert>
#include <cstdint>
#include <new>

#include "SimpleAllocatorTraits.h"

class MemoryBlock {
public:
  constexpr explicit MemoryBlock(size_t size) noexcept
    : metadata{size} {}

  uint8_t *UserMemoryBegin() noexcept {
    return reinterpret_cast<uint8_t *>(this + 1);
  }

  uint8_t *UserMemoryEnd() noexcept {
    return UserMemoryBegin() + metadata.size;
  }

  constexpr size_t GetBlockSize() const noexcept {
    return metadata.size;
  }

  constexpr void SetBlockSize(size_t size) noexcept {
    metadata.size = size;
  }

  static constexpr MemoryBlock *FromUserMemory(void *ptr) noexcept {
    return static_cast<MemoryBlock *>(ptr) - 1;
  }

private:
  struct alignas(SimpleAllocatorTraits::ALIGNMENT) {
    size_t size;
  } metadata;
};

#endif // MEMORYBLOCK_H
