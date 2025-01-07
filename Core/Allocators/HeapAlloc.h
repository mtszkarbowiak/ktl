// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include <cstdint>

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Memory.h"
#include "Types/Numbers.h"

/// <summary>
/// Memory allocator that uses the system heap.
/// </summary>
class HeapAlloc
{
public:
    static constexpr bool  IsNullable  = true;
    static constexpr int32 MinCapacity = 1;
    static constexpr int32 MaxCapacity = INT32_MAX;

    class Data
    {
        byte* _ptr;

    public:

        FORCE_INLINE
        Data()
            : _ptr{ nullptr }
        {
        }

        FORCE_INLINE
        Data(const Data&)
            : _ptr{ nullptr }
        {
            // No allocator context to bind to the new data
        }

        FORCE_INLINE
        auto MovesItems() const -> bool
        {
            return true;
        }

        FORCE_INLINE
        Data(Data&& other) noexcept
            : _ptr{ other._ptr }
        {
            // Move the data
            other._ptr = nullptr;
        }

        FORCE_INLINE
        ~Data()
        {
            ASSERT_ALLOCATOR_SAFETY(_ptr == nullptr);
        }


        MAY_DISCARD FORCE_INLINE
        auto operator=(const Data&) -> Data&
        {
            Free();

            // No allocator context to bind to the new data
            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator=(Data&& other) noexcept -> Data&
        {
            if (this != &other)
            {
                if (_ptr)
                {
                    FREE(_ptr);
                }

                _ptr = other._ptr;
                other._ptr = nullptr;
            }

            return *this;
        }


        NO_DISCARD FORCE_INLINE
        auto Allocate(const int32 size) -> int32
        {
            ASSERT_ALLOCATOR_SAFETY(_ptr == nullptr);

            _ptr = static_cast<byte*>(MALLOC(size));
            return _ptr ? size : 0;
        }

        FORCE_INLINE
        void Free()
        {
            ASSERT_ALLOCATOR_SAFETY(_ptr != nullptr);

            FREE(_ptr);
            _ptr = nullptr;
        }


        NO_DISCARD FORCE_INLINE
        auto Get() const -> const byte*
        {
            return _ptr;
        }

        NO_DISCARD FORCE_INLINE
        auto Get() -> byte*
        {
            return _ptr;
        }
    };
};
