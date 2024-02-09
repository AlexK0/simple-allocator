#include "SimpleAllocator.h"

#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>

TEST(SimpleAllocatorTest, InitSetsBufferSize) {
  SimpleAllocator alloc;
  char buffer[100];
  EXPECT_TRUE(alloc.Init(buffer, sizeof(buffer)));
}

TEST(SimpleAllocatorTest, AllocateZeroSize) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Allocate(0);
  EXPECT_EQ(ptr, nullptr);
}

TEST(SimpleAllocatorTest, AllocateReturnsValidMemory) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Allocate(10);
  EXPECT_NE(ptr, nullptr);
}

TEST(SimpleAllocatorTest, DeallocateNullptr) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));

  alloc.Deallocate(nullptr);
}

TEST(SimpleAllocatorTest, AllocateRespectsBufferSize) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Allocate(sizeof(buffer) + 1);
  EXPECT_EQ(ptr, nullptr);
}

TEST(SimpleAllocatorTest, DeallocateCanFreeMemory) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto mem = alloc.Allocate(10);
  alloc.Deallocate(mem);
  auto mem2 = alloc.Allocate(10);
  EXPECT_NE(mem2, nullptr);
}

TEST(SimpleAllocatorTest, ReallocateCanIncreaseAllocation) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto mem = alloc.Allocate(10);
  auto mem2 = alloc.Reallocate(mem, 20);
  EXPECT_NE(mem2, nullptr);
}

TEST(SimpleAllocatorTest, ReallocateCanDecreaseAllocation) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto mem = alloc.Allocate(20);
  auto mem2 = alloc.Reallocate(mem, 10);
  EXPECT_NE(mem2, nullptr);
}

TEST(SimpleAllocatorTest, ReallocateReturnNullIfNoMemory) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Allocate(sizeof(buffer) - 10);
  auto new_ptr = alloc.Reallocate(ptr, sizeof(buffer) + 10);
  EXPECT_EQ(new_ptr, nullptr);
}

TEST(SimpleAllocatorTest, ReallocateReturnNullIfBufferOverflows) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr1 = alloc.Allocate(10);
  auto ptr2 = alloc.Allocate(40);
  EXPECT_NE(ptr2, nullptr);
  auto new_ptr1 = alloc.Reallocate(ptr1, 50);
  EXPECT_EQ(new_ptr1, nullptr);
}

TEST(SimpleAllocatorTest, ReallocateNullptr) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Reallocate(nullptr, 10);
  EXPECT_NE(ptr, nullptr);
}

TEST(SimpleAllocatorTest, ReallocateToZeroSize) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto mem = alloc.Allocate(20);
  auto new_mem = alloc.Reallocate(mem, 0);
  EXPECT_EQ(new_mem, nullptr);
}

TEST(SimpleAllocatorTest, DeallocateFreesExactAllocatedMemory) {
  SimpleAllocator alloc;
  char buffer[50];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Allocate(10);
  alloc.Deallocate(ptr);
  auto new_ptr = alloc.Allocate(10);
  EXPECT_NE(new_ptr, nullptr);
}

TEST(SimpleAllocatorTest, DeallocateReclaimMemory) {
  SimpleAllocator alloc;
  char buffer[100];
  alloc.Init(buffer, sizeof(buffer));
  auto ptr = alloc.Allocate(50);
  alloc.Deallocate(ptr);
  auto new_ptr = alloc.Allocate(50);
  EXPECT_NE(new_ptr, nullptr);
}

namespace {

struct AllocatedMemory {
  void *ptr;
  size_t size;
  size_t iteration;
};

void MarkMemory(AllocatedMemory &memory) {
  std::memset(memory.ptr, static_cast<uint8_t>(memory.iteration), memory.size);
}

bool CheckMemory(const AllocatedMemory &memory) {
  if (SimpleAllocator::Size(memory.ptr) != memory.size) {
    return false;
  }
  for (size_t n = 0; n != memory.size; ++n) {
    if (static_cast<uint8_t *>(memory.ptr)[n] != static_cast<uint8_t>(memory.iteration)) {
      return false;
    }
  }
  return true;
}

} // namespace

TEST(SimpleAllocatorTest, SmokeTest) {
  constexpr size_t buffer_size = 1024 * 1024 * 512;
  auto buffer = std::make_unique<char[]>(buffer_size);
  SimpleAllocator alloc;
  alloc.Init(buffer.get(), buffer_size);

  std::vector<AllocatedMemory> allocated_memory;
  std::srand(std::time(nullptr));
  for (size_t i = 0; i != 4000000; ++i) {
    const auto r = static_cast<size_t>(std::rand());

    size_t size = (r + 115249) % (1024 * 16);
    switch ((r + 17) % 7) {
      case 0:
        size += 1024 * 64;
      case 1:
        size += 1024 * 32;
      case 2:
        size += 1024 * 16;
    }

    if (!allocated_memory.empty()) {
      const size_t memory_index = (r + 11) % allocated_memory.size();
      std::swap(allocated_memory[memory_index], allocated_memory.back());
      auto &last = allocated_memory.back();
      ASAN_UNPOISON_MEMORY_REGION(last.ptr, last.size);
      ASSERT_TRUE(CheckMemory(last));
      ++last.iteration;
      MarkMemory(last);

      const size_t action = (r + 71) % 5;
      if (action == 0) {
        alloc.Deallocate(last.ptr);
        allocated_memory.pop_back();
        continue;
      }
      if (action == 1) {
        if (auto new_ptr = alloc.Reallocate(last.ptr, size)) {
          last = {new_ptr, SimpleAllocator::Size(new_ptr), i};
          MarkMemory(last);
          ASAN_POISON_MEMORY_REGION(last.ptr, last.size);
        } else if (size) {
          ASSERT_TRUE(CheckMemory(last));
          ASAN_POISON_MEMORY_REGION(last.ptr, last.size);
          break;
        } else {
          allocated_memory.pop_back();
        }
        continue;
      }
    }

    if (auto new_ptr = alloc.Allocate(size)) {
      allocated_memory.push_back({new_ptr, SimpleAllocator::Size(new_ptr), i});
      MarkMemory(allocated_memory.back());
      ASAN_POISON_MEMORY_REGION(allocated_memory.back().ptr, allocated_memory.back().size);
    } else if (size) {
      break;
    }
  }

  for (auto &mem : allocated_memory) {
    ASAN_UNPOISON_MEMORY_REGION(mem.ptr, mem.size);
    ASSERT_TRUE(CheckMemory(mem));
    ++mem.iteration;
    MarkMemory(mem);
    alloc.Deallocate(mem.ptr);
  }
}
