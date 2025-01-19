// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once


/// <summary>
/// Type trait indicating whether a type is a C-style type.
/// C-Style means that the type is fundamental, pointer, array or struct of C-Style types.
/// This constraint is used to allow for heavy optimization in certain contexts.
/// Be careful about user defined types, which do not have non-trivial constructors or destructors,
/// but compiler generates them implicitly for C++ style fields.
/// </summary>
/// <remarks>
/// An equivalent of this trait is not available in the standard library before C++20,
/// where it's called <c>std::is_standard_layout</c>.
/// </remarks>
template<typename T>
struct TIsCStyle
{
    static constexpr bool Value = false;
};


template<typename T>
struct TIsCStyle<T*>
{
    // Pointers are C-style types.
    static constexpr bool Value = true;
};

template<typename T, size_t N>
struct TIsCStyle<T[N]>
{
    // Arrays are C-style types if their element type is C-style.
    static constexpr bool Value = TIsCStyle<T>::Value;
};

template<typename Ret, typename... Args>
struct TIsCStyle<Ret(*)(Args...)>
{
    // Function pointers are C-style types.
    static constexpr bool Value = true;
};


template<typename T>
struct CanBeCStyle
{
    static constexpr bool Value =
        std::is_trivially_constructible<T>::value &&
        std::is_trivially_copyable<T>::value &&
        std::is_nothrow_move_constructible<T>::value &&
        std::is_trivially_destructible<T>::value;
};

/// <summary>
/// Macro to declare a type as a C-style type. Be very careful with this macro as it may lead to difficult to debug issues.
/// </summary>
#define DECLARE_C_STYLE_TYPE(Type) \
    template<> struct TIsCStyle<Type> { static constexpr bool Value = true; };  \
    static_assert(CanBeCStyle<Type>::Value, "Type does not meet the requirements of a C-style type.");
