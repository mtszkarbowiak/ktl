# KTL

Welcome to Mateusz Karbowiak's Template Library (KTL) for C++!


# History

The KTL library began as a personal project during my time working with the Flax game engine. While Flax offers incredible features like a stunning renderer, I encountered limitations in its core library that disrupted my workflow. Recognizing the opportunity for improvement, I started making modifications. However, the scope of these changes quickly outgrew the original intent. What started as a proof of concept grew into a full-fledged library that could benefit a wider audience.

I quickly realized the potential of this library to streamline my work across multiple projects. An unexpected benefit of this endeavor was its versatility. The library is now adaptable for use in my own projects, whether they involve Flax, Unreal, Unity, or even non-gaming applications. It serves as a centralized solution to meet my diverse needs.

Beyond its practical applications, the KTL library is a testament to my passion for C++ and my commitment to pushing the boundaries of what's possible with the language. I currently develop this library in my free time and hope it proves useful to others as well. Feedback is always welcome!


# Goals

The KTL library is guided by a clear set of objectives aimed at delivering a valuable product and meeting high standards of quality:

- **Enhancing Productivity**: Provide a robust, intuitive, and efficient library that simplifies workflows across multiple use cases.
- **Versatility**: Ensure the library is adaptable for use in various projects, including game development, simulations, and other applications.
- **Performance**: Leverage advanced C++ techniques to deliver high-performance solutions that meet the demands of modern applications.
- **Encourage Collaboration**: Take active steps to engage with the community and encourage collaboration.
- **Real-World Applications**: Continuously refine the library by integrating it into my own private projects, using them as a testing ground for new features and improvements.
- **Innovate**: Explore new ideas and concepts to push the boundaries of what's possible with C++.


# Features

- General Types
	- `Box` - Single indirect value container.
	- `Nullable` - Type for optional values.
	- `Ref` - Non-owning pointer to a single element.
	- `Span` - Non-owning pointer to a range of elements.
- Collections
	- `Array` - Dynamic array, stack.
	- `Ring` - Circular buffer, double-ended queue.
	- `Dictionary` - Unordered hash table.
	- `HashSet` - Unordered set.
	- `BitArray` - Specialized array for bits.
- Allocators
	- `HeapAlloc` - Standard allocator for dynamic memory.
	- `FixedAlloc` - Pseudo-allocator for stack-allocated objects and more.
	- `BumpAlloc` - Fast allocator for temporary memory.
- Algorithms
	- Sorting algorithms: `QuickSort`, `InsertionSort`.
	- Queries
		- Transformations: `Select`, `Where`.
		- Collectors: `ToArray`.
		- Statistics: `Sum`, `Average`, `Min`, `Max`.


# Planned

- **Benchmarks**
- General Types
	- Type safety measures: `SafeBool`, `SafeInt`, etc.
	- Error handling: `Result`, `Error`, `Try`.
	- Variant
	- Concepts
- Collections
	- `ChunkedArray` - Array of arrays for very large collections.
	- `ChunkedRing` - Ring of arrays for very large collections.
	- `Heap` - Priority queue.
- Allocators
	- `PolymorphicAlloc` - Allocator that can use other allocators as backups.
	- `BumpConcurrentAlloc` - Thread-safe version of `BumpAlloc`.
	- `SlabAlloc` - Allocator for fixed-size objects.
	- `SlabConcurrentAlloc` - Thread-safe version of `SlabAlloc`.
	- `TlsAlloc` - Thread-local storage allocator.
	- `NumaAlloc` - NUMA-aware allocator.
- Algorithms
	- Hashing: `MurmurHash`, `CityHash`.
	- Sorting: `TimSort`, `RadixSort`.
	- Queries
		- Aggregations: `GroupBy`, `Join`.
		- Set operations: `Union`, `Intersect`, `Except`.
		- Statistics: `Variance`/`StandardDeviation`, `Median`.

> Check out the milestones for more details.


# Specs

- Language: C++14, C++17, C++20
- Compilers: MSVC, GCC, Clang
- Platforms: Windows, Linux
- 64 bits


# Contributing

At this stage, my primary focus is on laying a strong foundation for KTL by designing the core architecture, planning features, and refining concepts. During this initial production phase, I am not seeking contributions to the implementation itself.

However, I welcome ideas, suggestions, and constructive feedback that can help shape the library’s direction. Once the library reaches a more mature stage, I plan to provide clear contribution guidelines, including coding standards, issue tracking, and precise instructions on how to get involved. Stay tuned!


# Documentation

- [Style](Docs/Style.md)
- [Memory Management](Docs/Allocators.md)
- [Tombstones](Docs/Tombstones.md)
- [Queries](Docs/Queries.md)

> More documentation is coming soon!


# References

The library is inspired by the following projects: 

- [Flax Engine](https://github.com/FlaxEngine/FlaxEngine)
- [MSVC STL](https://github.com/microsoft/STL)
- Rust Standard Library

The project uses the following tools:

- [CMake](https://cmake.org/)
- [Google Test](https://github.com/google/googletest)


# License

Created by Mateusz Karbowiak 2024-25.

The project is available under the [MIT License](LICENSE.md).
