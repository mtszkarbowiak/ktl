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
#include "Language/Templates.h"
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

#if (ASSERTIONS_LEVEL >= 2)
        bool _allocated{ false };
#endif


    public:
        FORCE_INLINE constexpr
        Data() = default;

        FORCE_INLINE constexpr explicit
        Data(NullOptT)
            : Data()
        {
            // Pass
        }

        FORCE_INLINE constexpr
        Data(const Data&)
        {
            // Pass
        }

        FORCE_INLINE constexpr
        Data(Data&&) noexcept
        {
            // Pass

            // Note: Even though, the FixedAlloc never moves the items,
            // it can be moved - We treat it as binding-only move.
        }

        FORCE_INLINE
        ~Data()
        {
            // Pass

#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(!_allocated);
#endif
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

#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(!_allocated);
#endif

            return *this;
        }

        MAY_DISCARD FORCE_INLINE constexpr
        auto operator=(Data&&) noexcept -> Data&
        {
            // Pass

#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(!_allocated);
#endif

            // Note: Even though, the FixedAlloc never moves the items,
            // it can be moved - We treat it as binding-only move.

            return *this;
        }


        // NO_DISCARD FORCE_INLINE constexpr
        auto Allocate(const int32 size) -> int32
        {
    #if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(!_allocated);
    #endif

            if (size == Size)
            {
#if (ASSERTIONS_LEVEL >= 2)
                _allocated = true;
#endif
                return size;
            }
            else 
            {
                // Let the allocator gracefully fail if the size is incorrect.
                return 0;
            }
        }

        auto Reallocate(const int32 size) -> int32
        {
#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(!_allocated);
#endif

            return 0; // FixedAlloc does not support reallocation (not yet).
        }

        FORCE_INLINE constexpr
        void Free()
        {
            // Pass

#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(_allocated);
            _allocated = false;
#endif
        }


        NO_DISCARD FORCE_INLINE constexpr
        auto Get() const -> const byte*
        {
#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(_allocated);
#endif

            return _data;
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Get() -> byte*
        {
#if (ASSERTIONS_LEVEL >= 2)
            ASSERT_ALLOCATOR_SAFETY(_allocated);
#endif

            return _data;
        }
    };
};
