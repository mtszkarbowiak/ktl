// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include <Types/EnumPuller.h>
#include <Types/SizeHint.h>

template <typename T, T Last, T First, typename U>
auto EnumIterator<T, Last, First, U>::Hint() const -> SizeHint
{
    // SelectPuller does not change the number of elements.
    static constexpr int32 Count = static_cast<int32>(UCount);
    return SizeHint{ Count, Nullable<int32>{ Count } };
}
