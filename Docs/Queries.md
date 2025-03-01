# Queries

> This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


## Intro

The primary method of processing data can be described as sequential. Consequently, all major programming languages provide tools to simplify such operations:

| Language  | Lib        | Equivalent        | Interface        | 
|-----------|------------|-------------------|------------------|
| Java      | Streams    | `Iterator<T>`     | `Stream<T>`      |
| C#        | LINQ       | `IEnumerator<T>`  | `IEnumerable<T>` |
| Rust      | Iterators  | `Into<Iter<T>>`   | `Iterator<T>`    |

While C++ provides an equivalent in the form of `std::ranges`, this comes with two primary issues. First, they are supported only in C++20 and later. Second, they are significantly more complex compared to this solution ([e.g., a filter view](https://www.youtube.com/watch?v=O8HndvYNvQ4)).

This library abandons the concept of views entirely, as their potential benefits are outweighed by their complexity. However, it embraces the utility of sequential access tools.


## Pull-based Iterators

> In previous versions of the library, the term *cursor* and *enumerator* were used instead of *pull-based iterator*.

Pullers (pull iterators) are simple stateful objects that allow single-pass sequential access to all elements of a collection. Pullers API is inspired by the `IEnumerator<T>` interface in C#, but uses operator overloading to simplify the syntax and comply with C++ standards. One additional feature is the ability to provide a hint about the number of elements to be encountered.

```cpp
template<typename T>
class Collection
{
    class ValuesPuller
    {
        // Checks if cursor points to an object
        auto operator bool() -> bool;
        // Reference to the object
        auto operator *() -> T&;
        // Moves the cursor forward
        auto operator++()
        // Checks if cursors point to the same object
        auto operator==(const ValuesCursor&);

        // ...

        // Returns length hint
        auto Hint() const -> SizeHint;
    };

    auto Values() -> ValuesPuller;

    // ...
};
```

`SizeHint` helps the user of the puller estimate number of elements to be encountered. Thus it can use it to reserve space for incoming objects.

> *Size Hint* will be described in details once the API is stable.

### Loops

Pullers can be used directly in for loops:

```cpp
Collection<T> collection{};

for (auto puller = collection.Values(); puller; ++puller)
{
    T& element = *puller;
    //...
}
```


## Queries

Queries refer to a broad category of types and functions used to transform collection data. They operate on pullers as data sources, with naming conventions inspired by LINQ.

```cpp
int sum = Sum(collection.Values());
```

Query chaining is achieved using the bitwise OR operator `|`, resembling the pipe operator in UNIX. The left side defines the source puller, while the right side specifies an operation tag with any necessary metadata.

```cpp
int sum = collection.Values()
    | Where{ ... }
    | Select{ ... }
    | ToSum{};
```

### Type Inference

Unfortunately, versions of C++ prior to 17 can appear verbose, as they do not support type inference. Queries are therefore recommended primarily for reusable statements in such cases.

```cpp
// C++ 14 looks verobose :(

struct FieldGetter
{
    auto operator()(const T& t)
    {
        return t.GetField();
    }
};

Select<FieldGetter> selectField{ FieldGetter{} };

int sum1 = collection1.Values()
    | selectField
    | ToSum{};
    
int sum2 = collection2.Values()
    | selectField
    | ToSum{};

```

```cpp
// C++ 17 looks significantly better

int sum = collection1.Values()
    | Select{ [](const T& t) -> { return t.GetField(); } }
    | ToSum{};

int sum = collection2.Values()
    | Select<FieldGetter>{ {} }
    | ToSum{};
```


### Important Queries

> Due to the singificant simplification coming with type inference, the examples use C++ 17.

#### `Select`

`Select` transforms the object the puller is currently pointing to into another object. It is one of the most fundamental queries. This query does not trigger evaluation by itself.

#### `Where`

`Where` filters out elements that do not satisfy the given predicate. It is one of the core queries. Evaluation is triggered only for the first element, if it exists.

#### `Count`

`Count` simply counts how many objects the puller has pointed to. This query does not dereference the object. Generally, using this query in simple cases might be considered overkill as it will evaluate the entire puller.

#### `First`*

(Not yet implemented) First returns the first object the puller points to. It evaluates the puller until the first object is found.

#### `ToArray<A, G>`

`ToArray<A, G>` collects all objects the puller points to and stores them in an array. This query evaluates the entire puller.


### Recommendations

1. Always evaluate a query immediately after creating it, as some queries may require invoking the source puller right after construction.
2. Use the `To` prefix to distinguish collecting functions from other tags.
