// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Communism.h"
#include "Types/Numbers.h"

// --- Identity ---
// Identity type traits are used to return the type they are given.

/// <summary>
/// Type trait that returns the type it is given, as a nested type alias.
/// </summary>
template<typename T>
struct TIdentity
{
    using Type = T;
};

/// <summary>
/// Not-callable function that returns the type it is given.
/// </summary>
template<typename T>
auto Declval() noexcept -> T;

/// <summary>
/// Type to trigger evaluation of SFINAE expressions.
/// </summary>
template<typename T>
using VoidT = void;


/// <summary>
/// Type trait that always returns <c>true</c>.
/// </summary>
struct TTrueType
{
    static constexpr bool Value = true;
};

/// <summary>
/// Type trait that always returns <c>false</c>.
/// </summary>
struct TFalseType
{
    static constexpr bool Value = false;
};

// --- Flow Control ---
// Flow control type traits are used to conditionally select between two types.

// Conditional

template<bool C, typename T, typename F>
struct TConditional
{
    using Type = T;
};

template<typename T, typename F>
struct TConditional<false, T, F>
{
    using Type = F;
};

template<bool C, typename T, typename F>
using TConditionalT = typename TConditional<C, T, F>::Type;


// Enable If

template<bool C, typename T = void>
struct TEnableIf;

template<typename T>
struct TEnableIf<true, T>
{
    using Type = T;
};

template<bool C, typename T = void>
using TEnableIfT = typename TEnableIf<C, T>::Type;


// Disable If

template <bool C, typename T = void>
struct TDisableIf;

template <typename T>
struct TDisableIf<false, T>
{
    using Type = T;
};

template<bool C, typename T = void>
using TDisableIfT = typename TDisableIf<C, T>::Type;


// --- Logic Operators ---
// Logic operators allow you to combine multiple type traits into a single type trait.

// Not

/// <summary>
/// Computes the negation of a given trait.
/// T must have a constexpr static member <c>Value</c> of type bool.
/// </summary>
template<typename T>
struct TNot
{
    static constexpr bool Value = !T::Value;
};

template<typename T>
static constexpr bool TNotV = TNot<T>::Value;


// And

/// <summary>
/// Computes the logical AND of multiple traits.
/// It evaluates to true if all the traits have <c>Value == true</c>.
/// </summary>
template<typename... Ts>
struct TAnd; // Primary template for variadic traits.

template<bool L, typename... Ts>
struct TAndEval
{
    // Recursively evaluate `L && TAnd<Rest...>::Value`.
    static constexpr bool Value = L && TAnd<Ts...>::Value;
};

template<typename... Ts>
struct TAndEval<false, Ts...>
{
    // If `L == false`, no need to evaluate further. Result is false.
    static constexpr bool Value = false;
};

template<typename T1, typename... Ts>
struct TAnd<T1, Ts...>
{
    // Evaluate the Value of `T1` and pass the rest recursively.
    static constexpr bool Value = TAndEval<T1::Value, Ts...>::Value;
};

template<>
struct TAnd<>
{
    // Base case: Logical AND of an empty pack is true.
    static constexpr bool Value = true;
};

template<typename... Ts>
static constexpr bool TAndV = TAnd<Ts...>::Value;


// Or

/// <summary>
/// Computes the logical OR of multiple traits.
/// It evaluates to true if at least one of the traits has <c>Value == true</c>.
/// </summary>
template<typename... Ts>
struct TOr; // Primary template for variadic traits.

template<bool L, typename... Ts>
struct TOrEval
{
    // Recursively evaluate `L || TOr<Rest...>::Value`.
    static constexpr bool Value = L || TOr<Ts...>::Value;
};

template<typename... Ts>
struct TOrEval<true, Ts...>
{
    // If `L == true`, no need to evaluate further. Result is true.
    static constexpr bool Value = true;
};

template<typename T1, typename... Ts>
struct TOr<T1, Ts...>
{
    // Evaluate the Value of `T1` and pass the rest recursively.
    static constexpr bool Value = TOrEval<T1::Value, Ts...>::Value;
};

template<>
struct TOr<>
{
    // Base case: Logical OR of an empty pack is false.
    static constexpr bool Value = false;
};

template<typename... Ts>
static constexpr bool TOrV = TOr<Ts...>::Value;


// --- Checkers ---
// Checkers are used to determine the properties of a type.

// Equality

/// <summary>
/// Type trait that returns <c>true</c> if the two types are the same.
/// </summary>
template<typename T1, typename T2>
struct TIsSame
{
    static constexpr bool Value = false;
};

template<typename T>
struct TIsSame<T, T>
{
    static constexpr bool Value = true;
};

template<typename T1, typename T2>
static constexpr bool TIsSameV = TIsSame<T1, T2>::Value;


// Const Check

template<typename T>
struct TIsConst
{
    static constexpr bool Value = false;
};

template<typename T>
struct TIsConst<const T>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsConstV = TIsConst<T>::Value;


// Volatile Check

template<typename T>
struct TIsVolatile
{
    static constexpr bool Value = false;
};

template<typename T>
struct TIsVolatile<volatile T>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsVolatileV = TIsVolatile<T>::Value;


// L-Value Reference Check

template<typename T>
struct TIsLValRef
{
    static constexpr bool Value = false;
};

template<typename T>
struct TIsLValRef<T&>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsLValRefV = TIsLValRef<T>::Value;


// R-Value Reference Check

template<typename T>
struct TIsRValRef
{
    static constexpr bool Value = false;
};

template<typename T>
struct TIsRValRef<T&&>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsRValRefV = TIsRValRef<T>::Value;


// Any-Value Reference Check

template<typename T>
struct TIsRef
{
    static constexpr bool Value = TIsLValRef<T>::Value || TIsRValRef<T>::Value;
};

template<typename T>
static constexpr bool TIsRefV = TIsRef<T>::Value;


// Pointer Check

template<typename T>
struct TIsPtr
{
    static constexpr bool Value = false;
};

template<typename T>
struct TIsPtr<T*>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsPtrV = TIsPtr<T>::Value;


// Function Pointer Check

template<typename T>
struct TIsFuncPtr
{
    static constexpr bool Value = false;
};

template<typename Ret, typename... Args>
struct TIsFuncPtr<Ret(*)(Args...)>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsFuncPtrV = TIsFuncPtr<T>::Value;


// Function Check

template<typename T>
struct TIsFunc
{
    static constexpr bool Value = false;
};

template<typename Ret, typename... Args>
struct TIsFunc<Ret(Args...)>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsFuncV = TIsFunc<T>::Value;


// Array Check

template<typename T>
struct TIsArray
{
    static constexpr bool Value = false;
};

template<typename T, uintptr N>
struct TIsArray<T[N]>
{
    static constexpr bool Value = true;
};

template<typename T>
struct TIsArray<T[]>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsArrayV = TIsArray<T>::Value;


// Void Check

template<typename T>
struct TIsVoid
{
    static constexpr bool Value = false;
};

template<>
struct TIsVoid<void>
{
    static constexpr bool Value = true;
};

template<>
struct TIsVoid<const void>
{
    static constexpr bool Value = true;
};

template<>
struct TIsVoid<volatile void>
{
    static constexpr bool Value = true;
};

template<>
struct TIsVoid<const volatile void>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsVoidV = TIsVoid<T>::Value;


// --- Type Modifiers ---

// Remove Reference

template<typename T>
struct TRemoveRef
{
    using Type = T;
};

template<typename T>
struct TRemoveRef<T&>
{
    using Type = T;
};

template<typename T>
struct TRemoveRef<T&&>
{
    using Type = T;
};

template<typename T>
using TRemoveRefT = typename TRemoveRef<T>::Type;


// Remove Pointer

template<typename T>
struct TRemovePtr
{
    using Type = T;
};

template<typename T>
struct TRemovePtr<T*>
{
    using Type = T;
};

template<typename T>
using TRemovePtrT = typename TRemovePtr<T>::Type;


// Remove Const

template<typename T>
struct TRemoveConst
{
    using Type = T;
};

template<typename T>
struct TRemoveConst<const T>
{
    using Type = T;
};

template<typename T>
using TRemoveConstT = typename TRemoveConst<T>::Type;


// Remove Volatile

template<typename T>
struct TRemoveVolatile
{
    using Type = T;
};

template<typename T>
struct TRemoveVolatile<volatile T>
{
    using Type = T;
};

template<typename T>
using TRemoveVolatileT = typename TRemoveVolatile<T>::Type;


// Remove Both CV Qualifiers

template<typename T>
struct TRemoveCV
{
    using Type = T;
};

template<typename T>
struct TRemoveCV<const T>
{
    using Type = T;
};

template<typename T>
struct TRemoveCV<volatile T>
{
    using Type = T;
};

template<typename T>
struct TRemoveCV<const volatile T>
{
    using Type = T;
};

template<typename T>
using TRemoveCVT = typename TRemoveCV<T>::Type;


// Remove Const, Volatile, and Reference

template<typename T>
struct TRemoveCVRef
{
    using Type = TRemoveCVT<TRemoveRefT<T>>;
};

template<typename T>
using TRemoveCVRefT = typename TRemoveCVRef<T>::Type;


// Remove Extent

template<typename T>
struct TRemoveExtent
{
    using Type = T;
};

template<typename T>
struct TRemoveExtent<T[]>
{
    using Type = T;
};

template<typename T, uintptr N>
struct TRemoveExtent<T[N]>
{
    using Type = T;
};

template<typename T>
using TRemoveExtentT = typename TRemoveExtent<T>::Type;


// Decay

template<typename T>
struct TDecay
{
PRIVATE:
    using RawType = typename TRemoveCV<typename TRemoveRef<T>::Type>::Type;

public:
    using Type = TConditionalT<
        TIsArrayV<RawType>,
        TRemoveExtentT<RawType>*,
        TConditionalT<TIsFuncV<RawType>, RawType*, RawType>
    >;
};

template<typename T>
using TDecayT = typename TDecay<T>::Type;


// Add Reference

template<typename T>
struct TAddLValRef
{
    using Type = T&;
};

template<typename T>
struct TAddLValRef<T&>
{
    using Type = T&;
};

template<typename T>
struct TAddRValRef
{
    using Type = T&&;
};

template<typename T>
struct TAddRValRef<T&&>
{
    using Type = T&&;
};

template<typename T>
using TAddLValRefT = typename TAddLValRef<T>::Type;


// Add Const

template<typename T>
struct TAddConst
{
    using Type = const T;
};

template<typename T>
struct TAddConst<const T>
{
    using Type = const T;
};

template<typename T>
using TAddConstT = typename TAddConst<T>::Type;


// --- Variadic Type Traits ---

template<typename T, typename... Types>
struct TGetIndex;

template<typename T, typename TFirst, typename... Types>
struct TGetIndex<T, TFirst, Types...>
{
    enum {
        Value = (TIsSameV<T, TFirst>) ? (0) : (
            (TGetIndex<T, Types...>::Value == -1) ? (-1) : (
                1 + TGetIndex<T, Types...>::Value
        ))
    };
};

template<typename T, typename TLast>
struct TGetIndex<T, TLast>
{
    enum { Value = (TIsSameV<T, TLast>) ? (0) : (-1) };
};

template<typename T>
struct TGetIndex<T>
{
    enum { Value = (-1) };
};

template<typename T, typename... Types>
static constexpr int TGetIndexV = TGetIndex<T, Types...>::Value;


// --- Compiler Intrinsics ---

// Enum Check

template<typename T>
struct TIsEnum
{
    static constexpr bool Value = __is_enum(T);
};

template<typename T>
static constexpr bool TIsEnumV = TIsEnum<T>::Value;


// Plain Old Data Check

template<typename T>
struct TIsPOD
{
    static constexpr bool Value = __is_pod(T);
};

template<typename T>
static constexpr bool TIsPODV = TIsPOD<T>::Value;


// Trivially Constructible Check

template<typename T>
struct THasTrivialCtor
{
#if defined(__clang__) && __clang_major__ >= 15
    enum { Value = __is_trivially_constructible(T) };
#else
    enum { Value = __has_trivial_constructor(T) };
#endif
};

template<typename T>
static constexpr bool THasTrivialCtorV = THasTrivialCtor<T>::Value;


// Trivially Copyable Check

template<typename T>
struct THasTrivialCopy
{
#if defined(__clang__) && __clang_major__ >= 15
    enum { Value = __is_trivially_copyable(T) };
#else
    enum { Value = __has_trivial_copy(T) };
#endif
};

template<typename T>
static constexpr bool THasTrivialCopyV = THasTrivialCopy<T>::Value;


// Trivially Destructible Check

template<typename T>
struct THasTrivialDtor
{
#if defined(__clang__) && __clang_major__ >= 15
    enum { Value = __is_trivially_destructible(T) };
#else
    enum { Value = __has_trivial_destructor(T) };
#endif
};

template<typename T>
static constexpr bool THasTrivialDtorV = THasTrivialDtor<T>::Value;


// Copyable Check

template<typename T>
struct THasCopyCtor
{
    enum { Value = __is_constructible(T, const T&) };
};

template<typename T>
static constexpr bool THasCopyCtorV = THasCopyCtor<T>::Value;

template<typename T>
struct THasCopyAssign
{
    enum { Value = __is_assignable(T&, const T&) };
};

template<typename T>
static constexpr bool THasCopyAssignV = THasCopyAssign<T>::Value;

template<typename T>
static constexpr bool TIsCopyableV = THasCopyCtorV<T> && THasCopyAssignV<T>;


// Moveable Check

template<typename T>
struct THasMoveCtor
{
    enum { Value = __is_constructible(T, T&&) };
};

template<typename T>
static constexpr bool THasMoveCtorV = THasMoveCtor<T>::Value;

template<typename T>
struct THasMoveAssign
{
    enum { Value = __is_assignable(T&, T&&) };
};

template<typename T>
static constexpr bool THasMoveAssignV = THasMoveAssign<T>::Value;

template<typename T>
static constexpr bool TIsMoveableV = THasMoveCtorV<T> && THasMoveAssignV<T>;


// Unerlying Type

template<typename T>
struct TUnderlyingType
{
    using Type = __underlying_type(T);
};
