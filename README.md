# GameDev Fundamentals Library

> Note: This library is very work-in-progress. Originally, it was supposed to be only a proof-of-concept. Adding the features and stability have higher priority than performance. When the library has all the features, specific benchmarks will be added.

> Note: For examples of usage, see the test directory. It contains examples of example [collections](Tests/RingTests.cpp) or [queries](Tests/QueryingTests.cpp).

## I. State of Features

Types:

- [ ] `SafeBool`, `SafeInt` - Types preventing common mistakes: implicit conversions, arithmetic overflows, etc.
- [ ] `Nullable` - Type for optional values.
- [ ] `Result` - Type for returning errors.

Collections:

- [x] `Box` - Single value container.
- [x] `Array` - Dynamic array, stack.
- [x] `Ring` - Circular buffer, double-ended queue.
- [ ] `Dictionary` - Unordered hash table.
- [ ] `HasSet` - Unordered set.
- [x] `BitArray` - Specialized array for bits.
- [ ] `ChunkedArray` - Array of arrays for very large collections.

Allocators:

- [x] `HeapAlloc` - Standard allocator for dynamic memory.
- [ ] `PolymorphicAlloc` - Allocator that can use other allocators as backups.
- [x] `InlinedAlloc` - Psuedo-allocator for stack-allocated objects and more.
- [ ] `BumpAlloc` - Fast allocator for temporary memory.
- [ ] `BumpConcurrentAlloc` - Thread-safe version of `BumpAlloc`.
- [ ] `PoolAlloc` - Allocator for fixed-size objects.
- [ ] `PoolConcurrentAlloc` - Thread-safe version of `PoolAlloc`.

Algorithms

- [ ] `Math` - Math algorithms.
- [ ] `Sort` - Sorting algorithms.
- [ ] `Search` - Searching algorithms.
- [ ] `Hash` - Hashing algorithms.


## II. Objectives

- **Game Development**: The library is designed to be used in a game development environment where performance is a critical concern. As the primary source of performance issues in games is memory management, the library provides a set of data structures and algorithms that are designed to be efficient and flexible in terms of memory usage.
- **Parity with [Flax Engine](https://github.com/FlaxEngine/FlaxEngine)**: The library was originaly created as a replacement for the Flax Engine's core library. Making improvements required changing the design on the conceptual level thus it deviated too much for drop-in replacement.


## III. Key Assumptions

1. **Performance Priority**: The library is designed to be used in a game development environment where performance is a critical concern. It is assumed that handling memory correctly is the cornerstone of performance.
2. **C++14**: The library is written in for Cpp 14. Using higher versions may enable additional features, but compatibility with Flax requires Cpp 14.
3. **Memory Management**: The library is designed to be used in a game development environment where memory management is a critical concern. The library provides a set of data structures and algorithms that are designed to be efficient and flexible in terms of memory usage.
4. **Exception Handling**: The library does not use exceptions for error handling. Making the code handle them correctly enforces very strict rules and performance overhead. They can be used only for critical unrecoverable errors.
5. **Memory Limits**: Allocations are limited to 4GB. This is a reasonable limit for most game development scenarios. Thus, the library uses 32-bit integers for memory sizes. Pointers remain 64-bit.
6. **Move Semantics**: The library extensively uses move semantics to avoid unnecessary copying of objects. This is especially important for large objects such as arrays and dictionaries. A macro to block implicit copying is considered.
7. **No RTTI**: The library does not use RTTI. This is a performance optimization, as RTTI can be expensive in terms of memory and performance.
8. **No Inheritance**: The library does not use inheritance. It creates a lot of problems, best described by the author of the STL itself, Alexander Stepanov.
9. **Allocators Universality**: Collections must be able to accept all types of allocators.


## IV. Design Principles

### 1. General

- 1.1. The library uses different types of assertions groupped by asserted logic, so the programmer can balance between safety and performance.
- 1.2. The library puts a strong emphasis on performance, thus it may use practices that are not recommended in general programming. To counter this, the library provides a set of assertions and tests that can be enabled or disabled based on the build configuration.
- 1.3. The library should give as much information as possible about potential error, be it in compile-time or runtime.

### 2. Allocation Policy and Data

Allocation policy is a class storing general traits of an allocation, it includes data, context, etc. Allocation data serves as a binding between the allocation context and the allocated memory block.

- 2.1. Allocation data may represent maximum one contiguous memory block.
- 2.2. Allocation policy takes responsibility for alignment.
- 2.3. The destructor does not release the memory block.
- 2.4. Allocation data is not agnostic of the stored type. This disables `constexpr` but allows for tricks with the type system.
- 2.5. Accessing allocated block takes place through a method `Get() -> byte*`.
- 2.6. Allocation failures are signaled by returning a null pointer and zero size.
- 2.7. State of the allcation data may change only: on initialization, on allocation, on deallocation.
- 2.8. Allocation policy may declare the minimum and maximum size of allocatable memory blocks. Requesting a size outside this range is an error.
- 2.9. It discovers the size of the allocated block on allocation, but does not retain this information.
- 2.10. All allocations data must have a default constructor, even if it puts the object in an invalid state.
- 2.11. Copying allocation data duplicates the binding, not the memory block.
- 2.12. Moving allocation data transfers the binding, with the possibility of *dragging* the items (moving the owner without invoking move operators), based on collection policy.
- 2.13. Copying and moving allocation must never take place when allocation is active.
- 2.14. Allocation data must share information about dragging with a pure method `MovesItems() -> bool`.

### 3. Collection

Collections manage object lifetimes and manage required memory through allocation policies.

- 3.1. Collections may assume that even object is movable but not copyable.
- 3.2. Collections may not assume that the object is default constructible.
- 3.3. Collections should provide API to manage the memory block, especially reserving and compacting.
- 3.4. Collections should use fast operations for C-style objects (no constructors, destructors, etc.).
- 3.5. Every collection must use doubling strategy for resizing. This allows for heavy optimizations regarding hashign, iterating, etc. which involve modulo of capacity.
- 3.6. Collections may define default capacity i.e. minimal capacity of the collection.
- 3.7. Default capacity must obey constraints of the allocation policy.
- 3.8. Collections must be ready to accept different hash types.

### 4. Iterators

- 4.1. Collections provide two types of iterators: STL-style and enumerators (C#-style).
- 4.2. Enumerators are NOT re-enterant, making them singificantly simpler to implement, [and more importantly, to use](https://www.youtube.com/watch?v=49ZYW4gHBIQ&t=3414s) than [ranges](https://en.cppreference.com/w/cpp/ranges). This also makes them more similar to LINQ in C# (or Rust iterators).
- 4.3. STL-style iterators must obey the STL iterator concept to enable STL algorithms.
- 4.4. Enumerators provide simple API: `operator(bool)` for end condition, `operator++()` for moving to the next element and `operator*()` for accessing the current element.
- 4.5. Enumerators provide an enumeration space hints, so more advanced queries can predict number of elements: `Hint() -> struct IterHint{ Min; Max; }`. This reduces the need for dynamic memory allocation.

## V. Aknowledgements

Created by Mateusz Karbowiak 2024

The project is available under the [MIT License](LICENSE.md).

The library is inspired by the following projects: 

- [Flax Engine](https://github.com/FlaxEngine/FlaxEngine)
- [MSVC STL](https://github.com/microsoft/STL)
- Rust Standard Library

The project uses the following tools:

- [CMake](https://cmake.org/)
- [Google Test](https://github.com/google/googletest)
