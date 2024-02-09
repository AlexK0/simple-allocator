cmake_minimum_required(VERSION 3.20)
project(simple-allocator)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

add_library(simple-allocator STATIC
    src/simple-allocator/MemoryTree.cpp
    src/simple-allocator/SimpleAllocator.cpp
)

add_executable(simple-allocator-tests
    src/tests/Main.cpp
    src/tests/SimpleAllocatorTests.cpp
)

target_include_directories(simple-allocator-tests PRIVATE ${GTEST_INCLUDE_DIRS} src/simple-allocator)
target_link_libraries(simple-allocator-tests ${GTEST_BOTH_LIBRARIES} Threads::Threads simple-allocator)
target_compile_options(simple-allocator-tests PRIVATE -fsanitize=address)
target_link_options(simple-allocator-tests PRIVATE -fsanitize=address)

add_library(malloc-replacement SHARED
    src/malloc-replacement/Malloc.cpp
)

target_include_directories(malloc-replacement PRIVATE src/simple-allocator)
target_link_libraries(malloc-replacement PRIVATE simple-allocator)

add_executable(benchmark-deque src/benchmarks/Deque.cpp)
target_link_libraries(benchmark-deque PRIVATE malloc-replacement benchmark::benchmark)

add_executable(benchmark-list src/benchmarks/List.cpp)
target_link_libraries(benchmark-list PRIVATE malloc-replacement benchmark::benchmark)

add_executable(benchmark-list-huge-element src/benchmarks/ListHugeElement.cpp)
target_link_libraries(benchmark-list-huge-element PRIVATE malloc-replacement benchmark::benchmark)

add_executable(benchmark-map src/benchmarks/Map.cpp)
target_link_libraries(benchmark-map PRIVATE malloc-replacement benchmark::benchmark)

add_executable(benchmark-unordered-map src/benchmarks/UnorderedMap.cpp)
target_link_libraries(benchmark-unordered-map PRIVATE malloc-replacement benchmark::benchmark)

add_executable(benchmark-vector src/benchmarks/Vector.cpp)
target_link_libraries(benchmark-vector PRIVATE malloc-replacement benchmark::benchmark)
