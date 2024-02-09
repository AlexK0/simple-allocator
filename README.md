## The Simple Allocator

This is an implementation of the memory allocator discussed in the [Crafting Memory Allocators blog post](https://alexk0.github.io/posts/crafting-memory-allocators/).

The build and benchmarks were tested on **macOS**.

#### Release build
```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -flto" .
make -C build-release
```

#### Debug build
```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O0 -g3" .
make -C build-debug
```

#### Benchmarks

- `std::deque<T>`
```bash
build-release/benchmark-deque
```
- `std::list<T>`
```bash
build-release/benchmark-list
```
- `std::list<T>` with long `std::string`
```bash
build-release/benchmark-list-huge-element
```
- `std::map<K, V>`
```bash
build-release/benchmark-map
```
- `std::unordered_map<K, V>`
```bash
build-release/benchmark-unordered-map
```
- `std::vector<V>`
```bash
build-release/benchmark-vector
```

#### Replacing the default system malloc
```bash
DYLD_INSERT_LIBRARIES=./build-release/libmalloc_replacement.dylib DYLD_FORCE_FLAT_NAMESPACE=1 <command>
```
