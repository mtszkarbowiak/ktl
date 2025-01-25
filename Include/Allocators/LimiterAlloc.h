// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Communism.h"
#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Language/Memory.h"
#include "Math/Arithmetic.h"
#include "Types/Numbers.h"

/// <summary>
/// Very simply proxy allocator that limits the capacity of another allocator, be it lower or upper bound.
/// To be used to fine-tune the behavior of other allocators.
/// </summary>
template<
    typename A, 
    int32 Min = A::MinCapacity,
    int32 Max = A::MaxCapacity
>
class LimiterAlloc
{
public:
    static constexpr bool  IsNullable  = A::IsNullable;
    static constexpr int32 MinCapacity = Min;
    static constexpr int32 MaxCapacity = Max;

    static_assert(MinCapacity <= MaxCapacity,    "The minimum capacity must be never greater than the maximum capacity.");
    static_assert(MinCapacity >= A::MinCapacity, "The minimum capacity must be greater or equal to the inner allocator's minimum capacity.");
    static_assert(MaxCapacity <= A::MaxCapacity, "The maximum capacity must be less or equal to the inner allocator's maximum capacity.");

    class Data
    {
    PRIVATE:
        typename A::Data _data{};


    public:
        NO_DISCARD FORCE_INLINE
        auto MovesItems() const -> bool
        {
            return _data.MovesItems();
        }

        NO_DISCARD FORCE_INLINE
        auto Get() const
        {
            return _data;
        }

        NO_DISCARD FORCE_INLINE
        auto Get()
        {
            return _data;
        }

        //NO_DISCARD FORCE_INLINE
        auto Allocate(int32 size) -> int32
        {
            // Apply the lower bound
            size = Math::Max<int32>(size, MinCapacity);

            // Apply the upper bound
            if (size > MaxCapacity)
            {
                return 0;
            }

            // Allocate the memory
            return _data.Allocate(size);
        }

        NO_DISCARD FORCE_INLINE
        auto Free()
        {
            _data.Free();
        }
    };
};
