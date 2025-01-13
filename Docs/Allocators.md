# Allocators

> - This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


## Intro

One of the fundamental elements of the library is its use of allocators. As the name suggests, their role is to help manage memory. While this definition is broad, the following chapters will explore the details of how allocators work in this context.

**All allocators must operate under the assumption that their sole responsibility is to allocate and deallocate memory, with no awareness of the type stored in that memory.**

This decision to use type-erased allocators has profound implications. Before diving deeper, here are some constraints that allocators impose, requiring collections to handle these aspects:

1. Allocators must not relocate memory. Relocation logic should be defined within the collections.
2. Allocators have no compile-time information about allocation alignment.

These two constraints necessitate additional code in collections to handle these features. However, they enable a simple and versatile API.


## Features

### Capacity

Capacity constraints are a fundamental aspect of allocators, defining the predetermined maximum and minimum allocation sizes. These constraints help collections calculate the number of elements they can hold. Since allocators always operate on raw memory, capacity is expressed in bytes.

```cpp
class MyAlloc
{
    static constexpr int32 MinCapacity = 1; // Minimum can never be zero (or less)
    static constexpr int32 MaxCapacity = INT32_MAX; // Maximum must be not smaller than minimum 

    //...
};
```

### Nullability

Nullability is a compile-time feature that indicates whether the allocator can distinguish between its active and inactive states. An allocator declares its inactive state by returning null from its data getter.

```cpp
class MyAlloc
{
    static constexpr bool IsNullable = true;

    //...
};
```

> This feature may be subject to future changes. Currently, it is used only by `Box`, as collections track allocation activity via capacity.

### Binding Type

The binding type is the type used to store information required for allocation and deallocation within collections. It also defines methods for interacting with the allocated data.

```cpp
class MyAlloc
{
    class Data
    {
        auto Allocate(int32 size) -> int32;
        void Deallocate();

        auto Get() -> byte*;
        auto Get() const -> const byte*;

        //...
    };

    //...
};
```

### Dragging

Dragging is a critical runtime feature that determines whether allocator data can move its memory without losing track of the stored memory. This interaction with moves is explored further in the next section. Here's what dragging changes:

**Dragging**: Allocator data (binding) can be moved along with active allocations (via constructors or assignments). The move should be efficient and must not alter the returned address (e.g., heap allocators simply move a pointer).

**Non-dragging**: Allocator data cannot be moved along with active allocations. The collection is responsible for manually transferring stored objects.

Dragging is a runtime feature, allowing polymorphic allocators to exhibit different behaviors depending on which suballocator is engaged. However, **there is no guarantee of how much time will pass between a dragging check and the actual move.**

> Compilers can optimize branches for dragging checks if the returned value is determined at compile time. This eliminates concerns about performance overhead.

To declare dragging, include a specific method in the Data class:

```cpp
class MyAlloc
{
    class Data
    {
        auto MovesItems() const -> bool;
    };
};
```

## Lifecycle

The allocation lifecycle is closely tied to the lifecycle of the data object. A single binding represents one allocation; thus, requesting allocation twice without deallocation in between is illegal. Deallocation must occur before destruction or reassignment. This lifecycle can also be represented as an [automaton](Images/AllocDataAutomata.svg).

```cpp
MyAlloc alloc{};
while(...)
{
    // alloc.Get(); // Illegal
    int32 allocated = alloc.Allocate(requestedCapacity);
    byte* memory = alloc.Get();
    alloc.Deallocate();
    // alloc.Get(); // Illegal
}
// ~MyAlloc()
```

Note that allocation data does not retain information about the allocated space. After allocation, the collection is responsible for maintaining this information.


## Basic Allocators

### `HeapAlloc`

This is the simplest context-less allocator, using `malloc(...)`. While often the slowest (especially for large allocations), it serves as the default allocator because it offers the largest potential allocation sizes. If this allocator fails, the issue is likely critical.

### `FixedAlloc<int32 Size>`

This allocator is a more advanced, context-less implementation, storing all its data within a single fixed-size object. It is specifically designed for small collections, delivering excellent performance by eliminating additional allocation steps. Key points include:

- Fixed capacity: The size is fixed and immutable, disallowing reallocations.
- Compatibility checks: Collections with specific capacity requirements (e.g., HashSet power-of-two constraints) might require verification against the fixed size.
- Byte-based sizing: The Size parameter represents the buffer size in bytes, not the count of individual elements, as allocators have no informationa about the element type.

If allocation failures are a concern, consider pairing this with FallbackAlloc for added robustness.

### `FallbackAlloc<typename A1, typename A2>` (Not implemented yet)

This allocator chains other allocators, switching between them as needed. If one fails, it moves to the next. Example: `FallbackAlloc<FixedAlloc, HeapAlloc>`. This feature provides extreme flexibility by allowing cascading fallbacks.

### `BumpAlloc`

The simplest context-full allocator, using a preallocated memory block. It only moves a pointer to allocate the next block. Once all deallocations are complete, the pointer can be reset to the start of the block.


## Allocator Utility Tools

Collections often perform similar operations on allocators. To simplify code reuse, a specialized helper class is provided:

```cpp
template<
    typename Element, 
    typename Alloc, 
    int32 DefaultUncapped,
    int32(&Grow)(int32)
>
class AllocHelperOf;
```

This class offers various tools for managing allocations. Review it before implementing your own capacity management functions.


## Capacity

Using allocators requires a meticulous approach to capacity management. Capacity is influenced by constraints derived from both the collection and the allocator. Its calculation adheres to a specific convention:

- **Required**: The minimum capacity necessary to store the number of elements the collection aims to allocate.
- **Requested**: The capacity requested by the collection, which may exceed the required capacity to meet a collection-specific minimum, known as the default capacity.
- **Default**: A predefined, non-zero minimum capacity specific to the collection type. This boosts the number of elements for collections that are inefficient at low capacities, such as hash-based collections.
- **Allocated**: The size of the memory block that the allocator ultimately decides to allocate. This may exceed the requested capacity.
- **Used**: The effective size of the memory block utilized by the collection. This value can never exceed the allocated capacity.

This system allows allocators to optimize memory usage by allocating additional space to minimize reallocations or even avoid future allocation failures that might occur due to insufficient memory.

> In the future, an example with specific numerical values may be added for better illustration.
