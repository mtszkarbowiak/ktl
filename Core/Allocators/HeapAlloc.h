// Created by Mateusz Karbowiak 2024

#pragma once

#include <cstdint>

#include "../Debugging/Assertions.h"
#include "../Language/Keywords.h"
#include "../Language/Memory.h"
#include "../Types/Numbers.h"

/// <summary>
/// Memory allocator that uses the system heap.
/// </summary>
class HeapAlloc
{
public:
    constexpr static bool  IsNullable  = true;
    constexpr static int32 MinCapacity = 1;
    constexpr static int32 MaxCapacity = INT32_MAX;

    class Data
    {
        void* _ptr;

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

        FORCE_INLINE NODISCARD
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

        ~Data()
        {
            ASSERT_MEMORY_BOUNDS(_ptr == nullptr);
        }


        FORCE_INLINE
        auto operator=(const Data&) -> Data&
        {
            Free();

            // No allocator context to bind to the new data
            return *this;
        }

        FORCE_INLINE
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


        FORCE_INLINE NODISCARD
        auto Allocate(const int32 size) -> int32
        {
            ASSERT_MEMORY_BOUNDS(_ptr == nullptr);

            _ptr = MALLOC(size);
            return _ptr ? size : 0;
        }

        FORCE_INLINE
        void Free()
        {
            ASSERT_MEMORY_BOUNDS(_ptr != nullptr);

            FREE(_ptr);
            _ptr = nullptr;
        }


        FORCE_INLINE NODISCARD
        auto Get() const -> const void*
        {
            return _ptr;
        }

        FORCE_INLINE NODISCARD
        auto Get() -> void*
        {
            return _ptr;
        }
    };
};
