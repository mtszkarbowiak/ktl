// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Numbers.h"
#include "Language/Communism.h"
#include "Language/Keywords.h"
#include "Language/TypeTraits.h"

#include <algorithm>



// Move Semantics

#define MOVE(x)       std::move(x)
#define FORWARD(T, x) std::forward<T>(x)

namespace SwapInternal // ADL Barrier
{
    /// <summary> Dispatch tag for types with a member <c>Swap</c> function. </summary>
    struct HasSwapTag {};
    /// <summary> Dispatch tag for types without a member <c>Swap</c> function. </summary>
    struct NoSwapTag {};

    /// <summary> Default tag for types without a member <c>Swap</c> function. </summary>
    template<typename T, typename = void>
    struct HasSwapFunction : std::false_type {};

    /// <summary> Specialization for types with a member <c>Swap</c> function. </summary>
    template<typename T>
    struct HasSwapFunction<T, VoidT<decltype(std::declval<T>().Swap(std::declval<T&>()))>> : std::true_type {};

    /// <summary>
    /// Returns the appropriate dispatch tag informing whether the type has a member <c>Swap</c> function.
    /// </summary>
    template<typename T>
    using GetSwapTag = std::conditional_t<HasSwapFunction<T>::value, HasSwapTag, NoSwapTag>;

    /// <summary>
    /// Swap implementation for types with a member <c>Swap</c> function.
    /// </summary>
    template<typename T>
    void SwapImpl(T& a, T& b, HasSwapTag) noexcept
    {
        a.Swap(b);
    }

    /// <summary>
    /// Swap implementation for types without a member <c>Swap</c> function.
    /// </summary>
    template<typename T>
    void SwapImpl(T& a, T& b, NoSwapTag) noexcept
    {
        static_assert(
            std::is_nothrow_move_constructible<T>::value,
            "Type must be nothrow move constructible."
        );

        T temp = MOVE(a);
        a = MOVE(b);
        b = MOVE(temp);
    }
}

/// <summary>
/// Swaps the values of two objects. If a type has a member <c>Swap</c> function, it will be used.
/// If it does not, the objects will be moved, assuming the type is no-throw move constructible.
/// </summary>
template<typename T>
void Swap(T& a, T& b) noexcept
{
    ::SwapInternal::SwapImpl(a, b, ::SwapInternal::GetSwapTag<T>{});
}


// Copy Semantics

/// <summary> Dispatch tag for types supporting copy semantics. </summary>
struct IsCopyableTag {};
/// <summary> Dispatch tag for types not supporting copy semantics. </summary>
struct NonCopyableTag {};

/// <summary> Returns the appropriate dispatch tag informing whether the type supports copy semantics. </summary>
template<typename T>
using GetCopyableTag = typename std::conditional<
    std::is_copy_constructible<T>::value,
    IsCopyableTag,
    NonCopyableTag
>::type;


// Null Semantics

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
struct GetMaxTombstoneDepth
{
    enum { Value = 0 };
};


// Null-Check

template<typename T>
NO_DISCARD FORCE_INLINE
auto Ptr2Ref(const T* ptr) -> const T&
{
    ASSERT(ptr != nullptr);
    return *ptr;
}

template<typename T>
NO_DISCARD FORCE_INLINE
auto Ptr2Ref(T* ptr) -> T&
{
    ASSERT(ptr != nullptr);
    return *ptr;
}
