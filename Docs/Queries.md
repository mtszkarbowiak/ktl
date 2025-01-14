# Queries

> - This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


## Intro

The main way of processing data can be called sequential. That's why all major programming languages offer tools to simply those operations:

| Language  | Equivalent | Equivalent        | Interface        | 
|-----------|------------|-------------------|------------------|
| Java      | Streams    | `Iterator<T>`     | `Stream<T>`      |
| C#        | LINQ       | `IEnumerator<T>`  | `IEnumerable<T>` |
| Rust      | Iterators  | `Into<Iter<T>>`   | `Iterator<T>`    |


While C++ offers an equivalent in form of `std::ranges`, it comes with two problems. Firstly, the are supported only for C++ 20. Secondly, they are significantly more complicated to this solution.

The library gives up the idea of a view completelly, as their potential advantages would be outweighted by problems. Yet, it recognizes the power of non-reentrant iterators uitility tools.


## Cursors

> The name cursor is inspired by cursors in database management systems. Old name was *enumerators* but it has been changed, due to collision with `enum`.

Cursors are simple stateful objects which allow for single-pass sequencial access to all elements of a collection. Cursors de facto comply with STL forward-only iterators, but have additional feature: length estimator.

```cpp
template<typename T>
class Collection
{
    class ValuesCursor
    {
        // Checks if cursor points to an object
        auto operator bool() -> bool;
        // Reference to the object
        auto operator *() -> T&;
        // Moves the cursor forward
        auto operator++()

        // Returns length hint
        auto Hint() const -> CursorHint;
    };

    auto Values() -> ValuesCursor;

    // ...
};
```

`CursorHint` helps the user of the cursor estimate number of elements to be encountered. Thus it can use it to reserve space for incoming objects.

### Loops

Cursors can be used directly in for loops:

```cpp
Collection<T> collection{};

for (auto cursor = collection.Values(); cursor; ++cursor)
{
    T& element = *cursor;
    //...
}
```


## Queries

Queries is a broad term of types and functions used to transform collections data. Their source of values are cursors and their naming is inspired by LINQ.

```cpp
int sum = Sum(collection.Values());
```

The chaining of the chaining of queries is accomplished by bitwise OR operator, which resembles pipe operator in UNIX: The left side defines source cursor, the right side has operation tag with necessary metadata.

```cpp
int sum = collection.Values()
    | Where{ ... }
    | Select{ ... }
    | ToSum{};
```

### Type Inference

Unfortunatelly, C++ versions before 17 can look very verbose, as they can not infer the type. Using queries is recommended only for re-used statements.

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


### Recommendations

1. Always evaluate query immediately after creating it, as some queries may require invoking the source cursor right after construction.
2. To distinguish tags with collecting functions use prefix `To`.
