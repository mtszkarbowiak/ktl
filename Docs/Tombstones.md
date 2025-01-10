# Tombstone Semantics

> - This document outlines the coding practices applied within codebase of [KTL](https://github.com/mtszkarbowiak/mk-stl).


## Intro

You may have seen such description in the past...

*The algorithm searches for object and returns its index. If not found, the method returns `-1`.*

Programmer using one of the values, which lies outside of space of correct values as a null is a very common practice. It is rationalized by simplicity and performance. The issue is that this solution requires meticulous documentation and is not generalizable. To unify this technique, the library uses dedicated *tombstone semantics*.

**Why generalizing null state is so important? Because, then it can be used in generics (e.g. collections) with very tangible results!** 


## Usage

### Smaller Tombstone Nullable

In the STL, `optional` uses an additional value, which is reffered to as sentinel to track if the main value is engaged. This method is used by `SentinelNullable`. Unfortunatelly, those sentinel value create overhead and require allocating additional memory.

To address this issue, new kind of nullable is added: `TombstoneNullable`. It does not introduce any new fields. Instead, it uses the store value to remember that it became nullified. It is done by *tombstone constructor*. Then, to track if the object is actually used, *tombstone checks* are used. The lack of sentinel value has big impact on used memory. (Keep it mind, that even a single byte may introduce huge padding if the other value requires specific alignment.)

What is very important is that a tombstone may have different *depths*. This is to help `Nullable` to be nested. This will come useful later.

Of course, a user needs not to explicitly define which method is used, as `Nullable` does it automatically. The types above can be used to manully control the type of nullable being used.

### Collections Tombstone Optimization

Hash collections (`HashSet` and `Dictionary`) are built of slots, small state machines which can take on of three values: *empty*, *occupied* and *deleted*. Occupied slots are those, which have value: be it an element or key-value pair. But slots have 2 separate types of not-having-value. This could be expressed with an enum, making a basic definition of a slot:

```cpp
struct Slot {
    Value _value;
    enum class Status { Empty, Occupied, Deleted } _status;
};
```

But we can express that state machine with nested `Nullables`! We just need to distinguish which one is empty and which one is deleted.

```cpp
using Slot = Nullable<Nullable<Value>>;
```

What are our gains? Lets imagine what the structs look like in memory. We will assign I to the integer bytes, S to sentinel and P for padding. Let's assume we want to store `int32`, which is common in terms of index. Tombstone `Nullable` has only one field of `int32` being 4 bytes. By default, `int32` has alignment of 4, enforcing 3 bytes of padding for 1 byte sentinel, in total 8 bytes. **This means that tombstone slots will take half of memory which would be takes by standard implementation**.

How does it look like in memory? Something like this:

```
--- Original ---
0 1 2 3  4 5 6 7  8 9 A B  C D E F
|--I--|  S |-P-|  |--I--|  S |-P-|

--- Tombstones ---
0 1 2 3  4 5 6 7
|--I--|  |--I--|

```
In the debugger, you may see:

```
D_  E_  O2  O3  // Original
-1  -2   2   3  // Tombstones
```

## Custom Tombstone Type

It is possible to define custom types supporting tombstones. To accomplish this, our candidate type `T` needs:

- Tombstone constructor `T{ TombstoneDepth }` to initialize value to use its nullification mechanism.
- Check, if the type is a tombstone `T::IsTombstone() const -> bool`.
- Tombstone depth check `T::GetTombstoneDepth() const -> byte` to check current depth. (It should never be called if `IsTombstone` is false).
- Max tombstone depth type trait `template<> struct GetMaxTombstoneDepth<T>{ ... }` to inform what is the maximal depth supported by our type.
- Assignments and destructor supporting detection of a tombstone, preferably trivial.

Of course, a type must never ever mutate during tombstone lifetime, the same way using moved variable is UB. Both checks shall remiain pure.


## Recommendations

1. Never use tombstones directly. Use `Nullable`. Otherwise you may confuse nesting mechanism.
1. Use tombstones only mainly for value types. They are the most impactful and easiest to use.
2. The most important are first 2 depth levels. Thus concentrate on the lowest level.
3. Keep all tombstone invariants private, and befriend `Nullable` so someone accidentally doesn't use tombstones manually.


## Rationale

1. Tombstone has been decided as alternative lifetime, instead of external bit manipulation. This is because using invariants of uninitialized object is UB.
