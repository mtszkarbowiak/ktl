// Created by Mateusz Karbowiak 2024

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
    constexpr static bool  IsNullable  = false;
    constexpr static int32 MinCapacity = Size;
    constexpr static int32 MaxCapacity = Size;

    class Data
    {
        alignas(Alignment) byte _data[Size];

    public:
        constexpr Data() = default;

        constexpr Data(const Data&)
        {
            // Pass
        }

        constexpr Data(Data&&) noexcept
        {
            // Pass
        }

        FORCE_INLINE NODISCARD
        constexpr auto MovesItems() const -> bool
        {
            return false;
        }


        constexpr auto operator=(const Data&) -> Data&
        {
            // Pass
            return *this;
        }

        constexpr auto operator=(Data&&) noexcept -> Data&
        {
            // Pass
            return *this;
        }


        FORCE_INLINE NODISCARD
        constexpr auto Allocate(const int32 size) -> int32
        {
            ASSERT(size == Size || size == 0);
            return (size == Size) ? size : 0;
        }

        FORCE_INLINE
        constexpr void Free()
        {
            // Pass
        }


        FORCE_INLINE NODISCARD
        constexpr auto Get() const -> const void*
        {
            return _data;
        }

        FORCE_INLINE NODISCARD
        constexpr auto Get() -> void*
        {
            return _data;
        }
    };
};
