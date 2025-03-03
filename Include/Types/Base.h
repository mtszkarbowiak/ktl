// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/TypeTraits.h"
#include "Types/Numbers.h"


// Non-owning References

template<typename T>
class Ref;
template<typename T>
class Span;


// Iteration Utilities

class Index;
class SizeHint;


// Null Semantics

/// <summary>
/// Tag used to request the creation of a null object.
/// </summary>
struct NullOptT{};

/// <summary>
/// Special tag used to request the creation of a tombstone object.
/// Tombstone objects are used to represent null values in nullable types.
/// This allows for the optimization by avoiding the usage of additional flags.
/// </summary>
struct TombstoneDepth
{
    /// <summary>
    /// Depth of the requested tombstone. It must always be greater than zero.
    /// Zero depth means that the type does not support tombstone values.
    /// </summary>
    int8 Value{};
};

/// <summary>
/// Number of tombstone values supported by the type.
/// </summary>
/// <remarks>
/// Tombstone depth is a de facto maximal level of nesting Nullable types, without additional memory:
/// <c> Nullable<Nullable<Nullable<...>>> </c>
/// </remarks>
template<typename T>
struct TMaxTombstoneDepth
{
    enum { Value = 0 };
};

/// <summary>
/// Pre-declaration of the Nullable type.
/// </summary>
///
/// <typeparam name="T">
/// Type of the stored object.
/// </typeparam>
/// <typeparam name="D">
/// (Optional) Tombstone depth drift. It indicates which tombstone level defines the null state.
/// To be used to modify the tombstone depth of trivially constructed nested nullables.
/// Example: <c>Nullable<Nullable<int, 2>></c> produces correct values during trivial construction.
/// </typeparam>
/// <typeparam name="C">
/// (Automatic) Flag indicating if the nullable has trivial lifecycle.
/// </typeparam>
/// <typeparam name="M">
/// (Automatic) Flag indicating if the nullable delegates nullification to the tombstone of wrapped type.
/// </typeparam>
template<
    typename T,
    int8 D = 1,
    bool C = (THasTrivialLifecycleV<T>),
    bool M = (TMaxTombstoneDepth<T>::Value > 0)
>
class Nullable;

template<typename T, int8 D = 1>
using SentinelNullable = Nullable<T, D, false, false>;
template<typename T, int8 D = 1>
using TombstoneNullable = Nullable<T, D, false, true>;
