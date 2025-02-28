// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Types/Nullable.h"
#include "Types/Numbers.h"
#include "Types/SizeHint.h"

/// <summary>
/// Psuedo-random number generator based on the Xorwow algorithm.
/// It can be used as a pull iterator to generate random numbers.
/// </summary>
class XorwowRng final
{
PRIVATE:
    uint32 _state[5];
    uint32 _d;


public:
    explicit
    XorwowRng(uint32 seed = 0);

    NO_DISCARD FORCE_INLINE
    operator bool() const
    {
        return true; // Always valid.
    }

    NO_DISCARD FORCE_INLINE
    auto operator*() const -> uint32
    {
        return _state[4] + _d;
    }

    MAY_DISCARD
    auto operator++() -> XorwowRng&;

    MAY_DISCARD
    auto operator++(int) -> XorwowRng
    {
        auto copy = *this;
        ++*this;
        return copy;
    }

    NO_DISCARD FORCE_INLINE
    auto Hint() const -> SizeHint
    {
        return SizeHint::Empty();
    }
};
