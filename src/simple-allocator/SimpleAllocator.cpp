// Simple Allocator 2024
#include "SimpleAllocator.h"

#include "MemoryBlock.h"
#include "SimpleAllocatorTraits.h"

#include <cstdint>
#include <cstring>

namespace {

template<size_t N, class T>
constexpr T AlignN(T v) noexcept {
  static_assert(N && ((N - 1) & N) == 0, "power of 2 is expected");
  return static_cast<T>((static_cast<uint64_t>(v) + N - 1) & (~(N - 1)));
}

template<size_t N, class T>
constexpr std::pair<T *, T *> AlignBuffer(T *begin, T *end) noexcept {
  return {reinterpret_cast<T *>(AlignN<N>(reinterpret_cast<uintptr_t>(begin))), reinterpret_cast<T *>(AlignN<N>(reinterpret_cast<uintptr_t>(end) - (N - 1)))};
}

} // namespace

bool SimpleAllocator::Init(void *buffer, size_t buffer_size) noexcept {
  if (buffer_begin_ || buffer_end_ || current_) {
    return false;
  }

  auto being = static_cast<uint8_t *>(buffer);
  auto end = being + buffer_size;
  auto [buffer_begin, buffer_end] = AlignBuffer<SimpleAllocatorTraits::ALIGNMENT>(being, end);
  if (buffer_begin >= buffer_end) {
    return false;
  }

  buffer_begin_ = buffer_begin;
  buffer_end_ = buffer_end;
  current_ = buffer_begin_;
  return true;
}

uint8_t *SimpleAllocator::CutBuffer(size_t size) noexcept {
  if (current_ + size > buffer_end_) {
    return nullptr;
  }

  uint8_t *memory_piece = current_;
  current_ += size;
  return memory_piece;
}

void *SimpleAllocator::Allocate(size_t size) noexcept {
  if (!size) {
    return nullptr;
  }

  size = AlignN<SimpleAllocatorTraits::ALIGNMENT>(size);

  const size_t slot_index = GetSlotIndex(size);
  if (slot_index < slots_.size()) {
    if (MemoryBlock *memory_block = slots_[slot_index].GetNext()) {
      return memory_block->UserMemoryBegin();
    }
  } else if (MemoryBlock *memory_block = memory_tree_.RetrieveBlock(size)) {
    const size_t total_left_size = memory_block->GetBlockSize() - size;
    if (total_left_size > sizeof(MemoryBlock)) {
      const size_t user_left_size = total_left_size - sizeof(MemoryBlock);
      if (GetSlotIndex(user_left_size) >= slots_.size()) {
        memory_block->SetBlockSize(size);
        auto left_memory_block = new (memory_block->UserMemoryEnd()) MemoryBlock{user_left_size};
        memory_tree_.InsertBlock(left_memory_block);
      }
    }
    return memory_block->UserMemoryBegin();
  }

  static_assert(alignof(MemoryBlock) % SimpleAllocatorTraits::ALIGNMENT == 0);
  if (uint8_t *memory_piece = CutBuffer(sizeof(MemoryBlock) + size)) {
    auto *memory_block = new (memory_piece) MemoryBlock{size};
    return memory_block->UserMemoryBegin();
  }
  return nullptr;
}

void *SimpleAllocator::Reallocate(void *ptr, size_t new_size) noexcept {
  if (!ptr) {
    return Allocate(new_size);
  }

  if (!new_size) {
    Deallocate(ptr);
    return nullptr;
  }

  new_size = AlignN<SimpleAllocatorTraits::ALIGNMENT>(new_size);

  auto *memory_block = MemoryBlock::FromUserMemory(ptr);
  if (new_size == memory_block->GetBlockSize()) {
    return ptr;
  }

  if (memory_block->UserMemoryEnd() == current_) {
    if (new_size < memory_block->GetBlockSize()) {
      current_ -= memory_block->GetBlockSize() - new_size;
      memory_block->SetBlockSize(new_size);
      return ptr;
    }
    const size_t extra_size = new_size - memory_block->GetBlockSize();
    if (CutBuffer(extra_size)) {
      memory_block->SetBlockSize(new_size);
      return ptr;
    }
  }

  auto *new_ptr = Allocate(new_size);
  if (new_ptr) {
    std::memcpy(new_ptr, ptr, std::min(memory_block->GetBlockSize(), new_size));
    Deallocate(ptr);
  }
  return new_ptr;
}

void SimpleAllocator::Deallocate(void *ptr) noexcept {
  if (!ptr) {
    return;
  }

  auto *memory_block = MemoryBlock::FromUserMemory(ptr);
  if (memory_block->UserMemoryEnd() == current_) {
    current_ = reinterpret_cast<uint8_t *>(memory_block);
    return;
  }

  const size_t slot_index = GetSlotIndex(memory_block->GetBlockSize());
  if (slot_index < slots_.size()) {
    slots_[slot_index].AddNext(memory_block);
  } else {
    memory_tree_.InsertBlock(memory_block);
  }
}

size_t SimpleAllocator::Size(void *ptr) noexcept {
  return ptr ? MemoryBlock::FromUserMemory(ptr)->GetBlockSize() : 0;
}
