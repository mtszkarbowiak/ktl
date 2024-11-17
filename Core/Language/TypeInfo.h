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
static constexpr bool TIsCStyle = false;


template<typename T>
static constexpr bool TIsCStyle<T*> = true;

template<typename T, size_t N>
static constexpr bool TIsCStyle<T[N]> = TIsCStyle<T>;

template<typename Ret, typename... Args>
static constexpr bool TIsCStyle<Ret(*)(Args...)> = true;



/// <summary>
/// Macro to declare a type as a C-style type.
/// </summary>
#define DECLARE_C_STYLE_TYPE(Type) \
    template<> static constexpr bool TIsCStyle<Type> = true;
