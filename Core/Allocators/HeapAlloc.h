// Created by Mateusz Karbowiak 2024

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
    constexpr static bool  IsNullable  = true;
    constexpr static int32 MinCapacity = 1;
    constexpr static int32 MaxCapacity = INT32_MAX;

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
        bool MovesItems() const
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
            ASSERT_MEMORY(_ptr == nullptr);
        }


        FORCE_INLINE
        Data& operator=(const Data&)
        {
            Free();

            // No allocator context to bind to the new data
            return *this;
        }

        FORCE_INLINE
        Data& operator=(Data&& other) noexcept
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


        FORCE_INLINE
        int32 Allocate(const int32 size)
        {
            ASSERT_MEMORY(_ptr == nullptr);

            _ptr = static_cast<byte*>(MALLOC(size));
            return _ptr ? size : 0;
        }

        FORCE_INLINE
        void Free()
        {
            ASSERT_MEMORY(_ptr != nullptr);

            FREE(_ptr);
            _ptr = nullptr;
        }


        FORCE_INLINE
        const byte* Get() const
        {
            return _ptr;
        }

        FORCE_INLINE
        byte* Get()
        {
            return _ptr;
        }
    };
};
