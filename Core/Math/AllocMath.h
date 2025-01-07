// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"
#include "Types/Numbers.h"


class AllocMath
{
public:
    FORCE_INLINE
    static constexpr auto Align(const int32 value, const int32 alignment) -> int32
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }
};
