# Code Style

> This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


### Base

The foundation of these guidelines is the [Goole C++ Style Guide](https://google.github.io/styleguide/cppguide.html). This guide provides high-quality, well-tested rules with examples and the reasoning behind each recommandation. Topics covered include exceptions, linkage, inheritance, operator overloading, etc.

The project uses its the following formatting convention:

- **Indentation of 4 spaces** per level. The style is widely used in many different languages, including C#, Rust and C++ itself.
- **Line endings of LF** 

Using spaces ensures consitent code appearance across various environments, whether developers are using Visual Studio, NeoVim, CodeBlocks, CLion, Notepad++ or a pen and paper with gridlines. This eliminates the need for individual editor reconfiguration.



# Rules

## 1. Trailing Return Types

**Any method that returns anything other than `void` must use trailing return types.**

This choice might be controversial, as many programmers initially find this syntax uncomfortable. To address these concerns, the following section evaluates the decision by examining arguments for and against normal and trailing return types:


### Common Arguments for Normal Return Types

1. **Grammar Similarities** - Normal return types adhere to the ubiquitous `Type Name()` grammar. While this is a valid point, the very similarity of this grammar with field declarations, etc. is a source of [issues with parsing](https://en.wikipedia.org/wiki/Most_vexing_parse).

```cpp
int MaybeFunction();
int maybeVariable();
```

```cpp
auto Function() -> int;
int variable{};
```

2. **Conciseness** - At first glance, normal return types appear shorter. However, this benefit works only for short signatures, where clarity isn't an issue. In contrast, for long and complex signatures, trailing return types can singnificantly improve readability by reducing repetition.

```cpp
bool IsTrue(); // Normal r.t. is shorter
auto IsTrue() -> bool; // Trailing r.t. is definitelly longer here
```


```cpp
my_namespace::MyClass::MethodResult my_namespace::MyClass::MyMethod(); // Normal
auto my_namespace::MyClass::MyMethod() -> MethodResult; // Trailing is shorter here!
```

3. **Inlined Result Type Declaration** - This argument is valid. The technique is rarely used though.

```cpp
enum class OperationStatus { SuccessFailued } ExecuteOperation(); // Legal

// auto ExecuteOperation() -> enum class OperationStatus { SuccessFailued }; // Illegal
```

4. **Programmers Familiarity** - While it is a valid argument that most programmers are more accustomed to normal return types, this familiarity is superficial. Trailing return types offer advantages that justify the initial learning curve.

5. **IDE Familiarity** - Some IDEs may not support trailing return types as well as normal return types. However, this is a temporary issue, as IDEs are constantly updated to support modern C++ features.


### Other Advantages of Trailing Return Types

5. **Simplifies Complex Return Types** - Trailing return types eliminate the need for `decltype` in some contexts, making it easier to handle template-heavy code.

6. **Consistency with Modern C++ Features** - Trailing return types align with other modern C++ features, such as lambda expressions and concepts. This ensures that method declaration syntax is uniform in different areas.

```cpp
auto doStuff = [&](int i) -> bool { /* ... */ }
auto DoStuff(int i) -> bool { /* ... */ }
bool DoStuff(int i);
bool someValue{ i };
```

7. **Function Flow Mirroring** - Trailing return types are more similar to natural flow of information if a function, where input is processed at the beginning and the output at the end.

8. **Intendation of Method Names** - In classes with high number of methods, programmer can keep sight at the same column, increasing speed of searching method by name.

```cpp
struct Person 
{
    int GetAge() -> int;
    const Array<const Person*>& GetSiblings();
    bool IsMale();
};
```

```cpp
struct Person
{
    auto GetAge() -> int;
    auto GetSiblings() -> const Array<const Person*>&;
    auto IsMale() -> bool;
};
```

9. **Consistency with Other Languages** - Trailing return types are used in many other languages, such as Rust, Swift, Kotlin, etc. This consistency can help programmers who are familiar with these languages.

```rust
fn do_stuff(i: i32) -> bool { /* ... */ }
```



## 2. Explicit Type Conversions

**Avoid declaring implicit conversions, unless the type is trivially convertible.**

This rule is rationalized by the fact that using implicit type conversions may produce code which is difficult to work with, as it effectively suppresses effects of type checking.

```cpp
struct AssetRef 
{ 
    AssetRef(uint64 guid); // Initializes asset reference based on asset GUID.
    operator bool(); // Checks if there is referenced asset
}

struct Float3
{
    Float3(float f); // Initializes a vector equal to [f, f, f]
    operator float(); // Returns first coordinate
}

void AddView(Float3, AssetRef);

// Both are now legal, but only one is correct
AddView(Float3{ 0.3, 0.2, 0.1f }, GetAssetRef());
AddView(GetAssetRef(), Float3{ 0.3, 0.2, 0.1f });
```


### Explicit Conversions in `if` Satetements

It is worth noting that certain constructs, such as `if` statement, allow explicit type conversions, without `const_cast`. This behavior is often overlooked and can lead to confusion.


```cpp
struct BoolConvertible
{
    explicit operator bool() { ... }
};

BoolConvertible instance;
// bool hasValue = instance; // Illegal

if (instance) 
{
     // LEGAL!!! If statement ensures the conversion happens.
}

```

There are many such constructs: `if`, `while`, ternary operator. For more details, check [C++ reference](https://en.cppreference.com/w/cpp/language/implicit_conversion).



## 3. Template Type Arguments

**All template type arguments must use single letter abbraviations. To prevent ambiguities, the type should have full name in public using statement with suffix `Type`.**

The goal of this technique is to minimize length of the names of types using the template. To show its effect we can compare two entries that could appear in a call stack:

```
5. Dictionary<Key=int32, Value=const Texture*, Alloc=InlinedAlloc<128>, Hash=HashOf<int23>>::Add<_Key&&=const Key&>()
6. Dictionary<Key=int32, Value=const Texture*, Alloc=InlinedAlloc<128>, Hash=HashOf<int23>>::FindSlot(const Key&)
```

This call stack is shortened to:

```
5. Dictionary<K=int32, V=const Texture*, A=InlinedAlloc<128>, H=HashOf<int23>>::Add<_K&&=const K&>()
6. Dictionary<K=int32, V=const Texture*, A=InlinedAlloc<128>, H=HashOf<int23>>::FindSlot(const K&)
```

```cpp
template<typename T>
class TheCollection 
{
    // ...
    using Element = T;
};
```



## 3. Universal Reference Annotations

Move semantics in C++ are inherently complex and often lead to confusion, especially when r-value references become universal references in certain contexts. While the language does not provide a distinct syntax (for example triple-appersand `T&&&` operator) to distinguish universal references, clear annotations can mitigate misunderstanding.

To ensure code readability and clarify the developer's intent, universal references should be explicitly documented with comments. This practice helps future readers understand the purpose of the code.

```cpp
void DuStuff(T&&);

template<typename _T>
void DoStuff2(_T&&); // Universal reference
```

> Currently an idea for a rule to use underscore before universal reference equivalent of stored type is explored.



## 4. Comparison Operator Order

**In all comparison operators, put the constant on the left side and the variable on the right side.**

This rule is simple and easy to follow. It is based on the fact that it is easy to overlook the `==` operator and write `=` instead. This mistake is less likely to occur if the constant is on the left side.

Using this rule consistently also makes the code easier to read, as the tested value is always on the right side.

```cpp
if (0 == x) {} // Good
if (x == 0) {} // Bad
```
