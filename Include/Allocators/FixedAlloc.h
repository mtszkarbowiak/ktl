// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Memory.h"
#include "Types/Numbers.h"

/// <summary>
/// Memory allocator using a fixed-size inlined buffer.
/// </summary>
/// <typeparam name="Size"> Size of the buffer in bytes. </typeparam>
/// <typeparam name="Alignment"> Alignment of the buffer in bytes. </typeparam>
/// <remarks>
/// By default, the alignment is set to the pointer size, which is typically the most common alignment.
/// In some performance-critical scenarios, aligning to a higher value can be beneficial e.g. provoking SIMD instructions.
/// </remarks>
template<int32 Size, int32 Alignment = sizeof(void*)>
class FixedAlloc
{
public:
    static constexpr bool  IsNullable  = false;
    static constexpr int32 MinCapacity = Size;
    static constexpr int32 MaxCapacity = Size;

    class Data
    {
        alignas(Alignment) byte _data[Size];

    public:
        FORCE_INLINE constexpr
        Data() = default;

        FORCE_INLINE constexpr
        Data(const Data&)
        {
            // Pass
        }

        FORCE_INLINE constexpr
        Data(Data&&) noexcept
        {
            DEBUG_BREAK; // FixedAlloc must never be moved!
            // FixedAlloc can never be moved - `MovesItems` is always false.
            // Although, it must compile to allow run-time polymorphism.
        }

        FORCE_INLINE constexpr
        auto MovesItems() const -> bool
        {
            return false;
        }


        MAY_DISCARD FORCE_INLINE constexpr
        auto operator=(const Data&) -> Data&
        {
            // Pass
            return *this;
        }

        MAY_DISCARD FORCE_INLINE constexpr
        auto operator=(Data&&) noexcept -> Data&
        {
            DEBUG_BREAK; // FixedAlloc must never be moved!
            // FixedAlloc can never be moved - `MovesItems` is always false.
            // Although, it must compile to allow run-time polymorphism.
            return *this;
        }


        NO_DISCARD FORCE_INLINE constexpr
        auto Allocate(const int32 size) -> int32
        {
            // Let the allocator gracefully fail if the size is incorrect.
            return (size == Size) ? size : 0;
        }

        FORCE_INLINE constexpr
        void Free()
        {
            // Pass
        }


        NO_DISCARD FORCE_INLINE constexpr
        auto Get() const -> const byte*
        {
            return _data;
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Get() -> byte*
        {
            return _data;
        }
    };
};
