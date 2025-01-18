# KTL

> Note: This library is very work-in-progress. Originally, it was supposed to be only a proof-of-concept. Features and stability currently take priority over performance. When the library has all the features, specific benchmarks will be added.

> For examples, chec the test directories.


# History

This library began as a personal project during my time working with the Flax game engine. While Flax offers incredible features like a stunning renderer, I encountered limitations in its core library that hindered my workflow. I saw numerous opportunities for improvement and started making changes. However, the scope of these changes quickly grew so large that I decided to create a new library from the ground up—a proof of concept that could potentially replace Flax's core library.

An unexpected benefit of this endeavor was its versatility. The library is now adaptable for use in my own projects, whether they involve Flax, Unreal, Unity, or even non-gaming applications. It serves as a centralized solution to meet my diverse needs.

Another rewarding aspect of this project is the chance to push my own boundaries. C++ is a highly intricate language, and working on this library has been an incredible learning experience, helping me deepen my understanding of advanced concepts.

I currently develop this library in my free time and hope it proves useful to others as well. Feedback is always welcome!


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

- Type safety measures: `SafeBool`, `SafeInt`, etc.
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


# References

The library is inspired by the following projects: 

- [Flax Engine](https://github.com/FlaxEngine/FlaxEngine)
- [MSVC STL](https://github.com/microsoft/STL)
- Rust Standard Library

The project uses the following tools:

- [CMake](https://cmake.org/)
- [Google Test](https://github.com/google/googletest)


# License

Created by Mateusz Karbowiak 2024

The project is available under the [MIT License](LICENSE.md).
