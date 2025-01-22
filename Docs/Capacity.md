# Capacity

> This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


## Intro

Capacity is a fundamental aspect of interactions between collections and allocators. It tells how many elements a collection can hold, without the need to reallocate memory.

Capacity can quickly become a complex topic, as it is influenced by many factors ([simple example](https://www.youtube.com/watch?v=algDLvbl1YY)). The following chapters will explore the details of how capacity works in this context.


# Constraints

1. **Allocator limits**: The allocator defines the minimum and maximum capacity that a collection can request. These constraints are expressed in bytes.
2. **Allocator state**: The allocator may use its internal state to alter the capacity constraints. For example, an allocator may have a default capacity (for example slab size, inlined arena size, etc.).
3. **Collection-specific constraints**: Collections may impose additional constraints on capacity, such as a default capacity. This value is the minimum capacity that the collection will request from the allocator, even if the required capacity is lower. This allows to mitigate the performance impact of frequent reallocations for small collections.

**Recognizing how many constraints influence capacity of a collcation, a decision is made to completelly disable API to define exact capacity of collections.** Every collection is in charge of managing its capacity and user can only request minimal capacity and request to potentially shrink capacity.


# Categories

Capacities can be categorized in several ways, depending on the context. Here are some common categories:

- **Required**: The minimum capacity necessary to store the number of elements the collection aims to allocate.
- **Requested**: The capacity requested by the collection, which may exceed the required capacity to meet a collection-specific minimum, known as the default capacity.
- **Default**: A predefined, non-zero minimum capacity specific to the collection type. This boosts the number of elements for collections that are inefficient at low capacities, such as hash-based collections.
- **Allocated**: The size of the memory block that the allocator ultimately decides to allocate. This may exceed the requested capacity.
- **Used**: The effective size of the memory block utilized by the collection. This value can never exceed the allocated capacity.


# Rationale

The decision to completely disable precisely changing the capacity is motived by the fact that changes of capacity are expensive: both transferring data and allocating new memory. Capacity is de facto agreement between collection and allocator - Allocator has the best knowledge about how much memory can be allocated cheaply and collection knows how much memory it needs. This is why collection can only request minimal capacity and ask to potentially shrink capacity.
