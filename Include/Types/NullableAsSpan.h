// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Base.h"
#include "Types/Nullable.h"
#include "Types/Span.h"

template<typename T, int8 D, bool C, bool M>
NO_DISCARD FORCE_INLINE
auto AsSpan(Nullable<T, D, C, M>& nullable) -> Span<T>
{
    if (nullable.HasValue())
    {
        return Span<T>{ &nullable.Value(), 1 };
    }
    else
    {
        return Span<T>{};
    }
}

template<typename T, int8 D, bool C, bool M>
NO_DISCARD FORCE_INLINE
auto AsSpan(const Nullable<T, D, C, M>& nullable) -> Span<const T>
{
    if (nullable.HasValue())
    {
        return Span<const T>{ &nullable.Value(), 1 };
    }
    else
    {
        return Span<const T>{};
    }
}
