// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

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

template<typename T, size_t N>
struct TIsArray<T[N]>
{
    static constexpr bool Value = true;
};

template<typename T>
static constexpr bool TIsArrayV = TIsArray<T>::Value;


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

template<typename T, size_t N>
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
private:
    using RawType = typename TRemoveCV<typename TRemoveRef<T>::Type>::Type;
    
public:
    using Type = TConditional<
        TIsArray<T>::Value,
        TRemoveExtent<RawType>*,
        RawType
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
