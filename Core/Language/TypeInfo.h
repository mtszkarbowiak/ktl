// Created by Mateusz Karbowiak 2024

#pragma once


/// <summary>
/// Type trait indicating whether a type is a C-style type.
/// C-Style means that the type is fundamental, pointer, array or struct of C-Style types.
/// This constraint is used to allow for heavy optimization in certain contexts.
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

/// <summary>
/// Macro to declare a type as a C-style type.
/// </summary>
#define DECLARE_C_STYLE_TYPE(Type) \
    template<> struct TIsCStyle<Type> { static constexpr bool Value = true; };
