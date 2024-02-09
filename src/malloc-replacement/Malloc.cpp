// Simple Allocator 2024
#define _GNU_SOURCE
#include "SimpleAllocator.h"

#include <cassert>
#include <cstdlib>
#include <malloc/malloc.h>
#include <optional>

namespace {

class BufferedAllocator : SimpleAllocator {
public:
  explicit BufferedAllocator(size_t buffer_size) noexcept
    : buffer_size_(buffer_size)
    , buffer_(static_cast<uint8_t *>(std::malloc(buffer_size_))) {
    Init(buffer_, buffer_size_);
  }

  using SimpleAllocator::Allocate;
  using SimpleAllocator::Deallocate;
  using SimpleAllocator::Reallocate;
  using SimpleAllocator::Size;

  ~BufferedAllocator() noexcept {
    std::free(buffer_);
  }

private:
  const size_t buffer_size_{0};
  uint8_t *buffer_{nullptr};
};

class MallocReplacer {
public:
  static MallocReplacer &Instance() {
    static MallocReplacer malloc_replacer;
    return malloc_replacer;
  }

  BufferedAllocator *GetAllocator() noexcept {
    return active_allocator_;
  }

  void EnableBenchmarkAllocator(bool use_simple_allocator) noexcept {
    assert(!benchmark_allocator_.has_value());
    assert(active_allocator_ == &system_allocator_);
    if (use_simple_allocator) {
      active_allocator_ = &benchmark_allocator_.emplace(1024 * 1024 * 1024);
    } else {
      active_allocator_ = nullptr;
    }
  }

  void DisableBenchmarkAllocator() noexcept {
    assert(!active_allocator_ || active_allocator_ == &(*benchmark_allocator_));
    benchmark_allocator_.reset();
    active_allocator_ = &system_allocator_;
  }

private:
  MallocReplacer() = default;

  BufferedAllocator system_allocator_{256 * 1024 * 1024};
  std::optional<BufferedAllocator> benchmark_allocator_;

  BufferedAllocator *active_allocator_{&system_allocator_};
};

void *Malloc(size_t size) {
  if (auto *allocator = MallocReplacer::Instance().GetAllocator()) {
    auto ptr = allocator->Allocate(size);
    assert(!(reinterpret_cast<uintptr_t>(ptr) & 0xf));
    return ptr;
  }
  return std::malloc(size);
}

size_t MallocSize(void *ptr) {
  if (auto *allocator = MallocReplacer::Instance().GetAllocator()) {
    return allocator->Size(ptr);
  }
  return malloc_size(ptr);
}

void *Calloc(size_t count, size_t size) {
  if (auto *allocator = MallocReplacer::Instance().GetAllocator()) {
    const size_t total = count * size;
    auto *ptr = allocator->Allocate(total);
    assert(!(reinterpret_cast<uintptr_t>(ptr) & 0xf));
    if (ptr) {
      std::memset(ptr, 0x00, total);
    }
    return ptr;
  }
  return std::calloc(count, size);
}

void Free(void *ptr) {
  if (auto *allocator = MallocReplacer::Instance().GetAllocator()) {
    return allocator->Deallocate(ptr);
  }
  std::free(ptr);
}

void *Realloc(void *ptr, size_t size) {
  if (auto *allocator = MallocReplacer::Instance().GetAllocator()) {
    auto new_ptr = allocator->Reallocate(ptr, size);
    assert(!(reinterpret_cast<uintptr_t>(new_ptr) & 0xf));
    return new_ptr;
  }
  return std::realloc(ptr, size);
}

} // namespace

void EnableBenchmarkAllocator(bool use_simple_allocator) noexcept {
  MallocReplacer::Instance().EnableBenchmarkAllocator(use_simple_allocator);
}

void DisableBenchmarkAllocator() noexcept {
  MallocReplacer::Instance().DisableBenchmarkAllocator();
}

#define DYLD_INTERPOSE(_replacment, _replacee)                                                                                                                 \
  __attribute__((used)) static struct {                                                                                                                        \
    const void *replacment;                                                                                                                                    \
    const void *replacee;                                                                                                                                      \
  } _interpose_##_replacee                                                                                                                                     \
    __attribute__((section("__DATA,__interpose"))) = {(const void *)(unsigned long)&_replacment, (const void *)(unsigned long)&_replacee};

DYLD_INTERPOSE(Malloc, malloc);
DYLD_INTERPOSE(MallocSize, malloc_size);
DYLD_INTERPOSE(Calloc, calloc);
DYLD_INTERPOSE(Free, free);
DYLD_INTERPOSE(Realloc, realloc);
