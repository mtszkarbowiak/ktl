// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"
#include "Language/TypeTraits.h"
#include "Language/Yolo.h"
#include "Types/Numbers.h"

// Move Semantics

template<typename T>
NO_DISCARD FORCE_INLINE constexpr
auto Move(T&& x) NOEXCEPT_S -> TRemoveRefT<T>&&
{
    return static_cast<TRemoveRefT<T>&&>(x);
}

template<typename T>
NO_DISCARD FORCE_INLINE constexpr
auto Forward(TRemoveRefT<T>& x) NOEXCEPT_S -> T&&
{
    return static_cast<T&&>(x);
}

template<typename T>
NO_DISCARD FORCE_INLINE constexpr
auto Forward(TRemoveRefT<T>&& x) NOEXCEPT_S -> T&&
{
    return static_cast<T&&>(x);

    static_assert(
        !TIsLValRefV<T>,
        "Forwarding an r-value as an l-value reference."
    );
}

#define MOVE(x)       Move(x)
#define FORWARD(T, x) Forward<T>(x)


namespace SwapInternal // ADL Barrier
{
    template<typename T, typename = void>
    struct THasSwap
    {
        static constexpr bool Value = false;
    };

    template<typename T>
    struct THasSwap<T, VoidT<decltype(Declval<T>().Swap(Declval<T&>()))>>
    {
        static constexpr bool Value = true;
    };

    template<typename T>
    static constexpr bool THasSwapFunctionV = THasSwap<T>::Value;

    /// <summary>
    /// Swap implementation for types with a member <c>Swap</c> function.
    /// </summary>
    template<typename T>
    auto SwapImpl(T& a, T& b) NOEXCEPT_S -> TEnableIfT<THasSwapFunctionV<T>, void>
    {
        a.Swap(b);
    }

    /// <summary>
    /// Swap implementation for types without a member <c>Swap</c> function.
    /// </summary>
    template<typename T>
    auto SwapImpl(T& a, T& b) NOEXCEPT_S -> TDisableIfT<THasSwapFunctionV<T>, void>
    {
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
void Swap(T& a, T& b) NOEXCEPT_S
{
    ::SwapInternal::SwapImpl(a, b);
}


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
auto Ptr2Ref(const T* ptr) NOEXCEPT_S -> const T&
{
    ASSERT(ptr != nullptr);
    return *ptr;
}

template<typename T>
NO_DISCARD FORCE_INLINE
auto Ptr2Ref(T* ptr) NOEXCEPT_S -> T&
{
    ASSERT(ptr != nullptr);
    return *ptr;
}


// Const Helper

/// <summary>
/// Returns a const-reference to the referenced object.
/// De facto, it is a no-op function, used to select the correct overload.
/// </summary>
template<typename T>
NO_DISCARD FORCE_INLINE
auto AsConst(T& x) NOEXCEPT_S -> const T&
{
    return (x);
}

/// <summary>
/// Illegal overload, making sure that a temporary object
/// is not bound to a const-reference.
/// </summary>
template<typename T>
auto AsConst(T&&) = delete;


// Other Tags

struct NullOptT{};
