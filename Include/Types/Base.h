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


template<
    typename T,
    bool C = (THasTrivialLifecycleV<T>),
    bool M = (TMaxTombstoneDepth<T>::Value > 0)
>
class Nullable;

template<typename T>
using SentinelNullable = Nullable<T, false, false>;
template<typename T>
using TombstoneNullable = Nullable<T, false, true>;
