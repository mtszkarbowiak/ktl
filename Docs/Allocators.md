# Allocators

> - This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


## Intro

One of the fundamental elements of the library are allocators. As the name suggests, their role is to help us interact with memory. This definition is vague though, thus in the further chapters, details of how allocators work will be explored.


**All allocators must operate  under assmption, that allocator's reponsibility is exclusively to allocate and deallocate memory, without any information about type to be stored in that memory.**

This decision to use type-erased allocators has profound impact. First, let's go through things which allocators may not do, thus they need to be taken care of by collections using them:

1. Allocators must not relocate memory. The logic of relocation should be defined in the collections.
2. Allocators have no compile-time information about allocation alignment.

Those two responsibilities enforce lengthy blocks of code in collection to handle those features, but they allow for very simple and versitale API. 


## Features

### Capacity

Capacity constraints is the most fundamental element of allocators telling what is predetermined maximal and minimal size of allocation. It is used by collection to calculate how many elements it can hold. As allocators always operate on raw memory, it is expressed in bytes.

```cpp
class MyAlloc
{
    static constexpr int32 MinCapacity = 1; // Minimum can never be zero (or less)
    static constexpr int32 MaxCapacity = INT32_MAX; // Maximum must be not smaller than minimum 

    //...
};
```

### Nullability

Nullability is a compile-time feature telling, if the allocator can distinguish its active and inactive state. Allocator declares its inactive state by returning null by its data getter.

```cpp
class MyAlloc
{
    static constexpr bool IsNullable = true;

    //...
};
```

> This feature may be a subject of changes in the future. Currently it is used only by `Box` as collections track allocation activity by capacity.

### Binding Type

Binding type is a type, which is used to store information required to perform allocation and deallocation stored in collections. It also defines methods allowing to use the allocated data.

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

Dragging is the most important, run-time feature telling, if the allocator data has ability to move its data without losing track of stored memory. The interaction between this method and moves is futher explored in the next section. Lets see what this feature changes:

**Dragging** allocator data (binding) can be moved with active allocation (both by constructor and assignment). The move should be fast, and never change returned address. For example, heap allocation just moves a pointer.

**Non-dragging** allocator data (binding) can **not** be moved with active allocation. This means that the collection itself is responsible for manual transfer of stored objects.

Dragging is a run-time feature. This allows polymorphic allocators to minifest different behaviors, depending which of their suballocators is engaged. Be cautios though, as **there is not guarantee how much time will pass between the dragging check and the move itself!**

> Compilers can detect dragging (if returned value is compile-time) and optimize entire branch for opposite dragging, qualified as dead code. Thus it should not be concern regarding performance.

To declare dragging, a special method is placed in the `Data` class.

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

Allocation lifecycle is closely bound with lifecycle of data object. One binding can represent one allocation, thus requesting allocation twice without deallocation in between is illegal. **Deallocation must take place before destruction (or assignment).** The lifecycle can also be presented as an [automata](Images/AllocDataAutomata.svg).

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

Remember, that allocation data does not retain information about the allocated space. Once allocation is performed, the collection itself takes charge of saving that information. 


## Basic Allocators

### `HeapAlloc`

The most basic context-less allocator. It uses `malloc(...)`. This one is often the slowest too, especially for big allocations. It is the default allocator, as it is the only one that can be assumed to have always the biggest potential allocation sizes, thus it should be used as an ultimate fallback. It this allocator fails, you are doing something very, very wrong.

### `FixedAlloc<int32 Size>`

More advanced context-less allocator, which stores all values in the data object itself. It can be used for very small collections but has the best possible performance, as allocation requires no additional operations. Remember that its capacity is always a single capacity value (to prevent reallocations). This can trigger capacity checks in collection requiring specific capacities (e.g. `HashSet` powers of two). Remember also, that `Size` is the buffer size in bytes, not number of elements to be stored. If you want to handle allocation failure use `FallbackAlloc`. 

### `FallbackAlloc<typename A1, typename A2>` (Not implemented yet)

Allocator that use other allocators and switch between them if necessary. It works, by detecting if allocation with the first allocator succeeded, then it uses the next allocator. If all of them fail, it itself fails. Example use: `FallbackAlloc<FixedAlloc, HeapAlloc>`. Notably, ability to fallback is given by using by declaring yet another allocator, so that allocators have extreme flexibility.

### `BumpAlloc`

The simplest context-full allocator. It uses preallocated block of memory and and only moves to pointer to allocate next blocks. Once the user is sure, that all deallocations took place, they can request the pointer to be reset to the first byte of the block.


## Allocator Utility Tools

Operations on allocators by the collections are often very similar. To re-use code a specialized class is used, with signature:

```cpp
template<
    typename Element, 
    typename Alloc, 
    int32 DefaultUncapped,
    int32(&Grow)(int32)
>
class AllocHelperOf;
```

It has a lot of stuff to manage allocations. You should check it out before writing your own functions for capacity management.


## Capacity Glossary

> This section is not yet ready.
