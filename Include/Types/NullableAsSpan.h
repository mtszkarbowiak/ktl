// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Nullable.h"
#include "Types/Span.h"

template <typename T>
template <typename U>
constexpr 
auto Nullable<T, true>::AsSpan() -> Span<Element>
{
    if (HasValue())
    {
        return Span<Element>{ &_value, 1 };
    }
    else
    {
        return Span<Element>{};
    }
}

template <typename T>
template <typename U>
constexpr 
auto Nullable<T, false>::AsSpan() -> Span<Element>
{
    if (HasValue())
    {
        return Span<Element>{ &_value, 1 };
    }
    else
    {
        return Span<Element>{};
    }
}

template <typename T>
template <typename U>
constexpr 
auto Nullable<T, false>::AsSpan() const -> Span<const Element>
{
    if (HasValue())
    {
        return Span<const Element>{ &_value, 1 };
    }
    else
    {
        return Span<const Element>{};
    }
}

template <typename T>
template <typename U>
constexpr 
auto Nullable<T, true>::AsSpan() const -> Span<const Element>
{
    if (HasValue())
    {
        return Span<const Element>{ &_value, 1 };
    }
    else
    {
        return Span<const Element>{};
    }
}
