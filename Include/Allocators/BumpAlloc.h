// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include <cstdint>

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Memory.h"
#include "Language/Templates.h"
#include "Types/Numbers.h"

/// <summary>
/// Memory allocator using a bump-mechanism on given buffer.
/// </summary>
class BumpAlloc
{
public:
    static constexpr bool  IsNullable  = true;
    static constexpr int32 MinCapacity = 1;
    static constexpr int32 MaxCapacity = INT32_MAX;

    class Context
    {
        byte* _arenaPtr;   // Pointer to the beginning of the arena.
        int32 _arenaSize;  // Total number of available bytes.
        int32 _bump;       // Index of the next free byte.
        int32 _lastAlloc;  // Index of the last allocation.
        int32 _alignment;

    public:
        FORCE_INLINE explicit
        Context(byte* arenaPtr, const int32 arenaSize, const int32 alignment = sizeof(void*))
            : _arenaPtr{ arenaPtr }
            , _arenaSize{ arenaSize }
            , _bump{ 0 }
            , _lastAlloc{ 0 }
            , _alignment{ alignment }
        {
        }

        NO_DISCARD FORCE_INLINE
        auto Allocate(const int32 size, byte*& result) -> int32
        {
            const int32 newOffset = _bump + size;

            if (newOffset > _arenaSize)
            {
                return 0;
            }

            result     = _arenaPtr + _bump;
            _lastAlloc = _bump;
            _bump      = newOffset;

            //TODO(mtszkarbowiak) Add alignment support to BumpAlloc.

            return size;
        }

        NO_DISCARD FORCE_INLINE
        auto Relocate(const int32 size, byte*& result) -> int32
        {
            // Check if incoming allocation has the same pointer as the last one.
            byte* last = _arenaPtr + _lastAlloc;

            if (result != last)
                return 0;

            // If the last allocation is the same as the incoming one, we can just bump the pointer.
            const int32 prevSize = _bump - _lastAlloc;
            const int32 newOffset = _lastAlloc + (size - prevSize);
            if (newOffset > _arenaSize)
            {
                return 0;
            }
            _bump = newOffset;
            return size;
        }

        FORCE_INLINE
        void Reset()
        {
            _bump = 0;
        }

        NO_DISCARD FORCE_INLINE
        auto FreeSpace() const -> int32
        {
            return _arenaSize - _bump;
        }
    };


    class Data
    {
        Context* _context;
        byte*    _data;

    public:
        FORCE_INLINE
        auto MovesItems() const -> bool
        {
            return true;
        }

        /// <summary> Initializes bump allocator in invalid state. </summary>
        /// <remarks>
        /// This constructor is used only for temporary objects.
        /// Allocator must be properly initialized before use.
        /// </remarks>
        FORCE_INLINE
        Data()
            : _context{ nullptr }
            , _data{ nullptr }
        {
        }

        FORCE_INLINE
        Data(const Data& other)
        {
            _context = other._context;
            _data    = other._data;
        }

        FORCE_INLINE
        Data(Data&& other) noexcept
            : _context{ other._context }
            , _data{ other._data }
        {
            other._context = nullptr;
            other._data    = nullptr;
        }


        MAY_DISCARD FORCE_INLINE
        auto operator=(const Data& other) -> Data&
        {
            ASSERT_ALLOCATOR_SAFETY(_data == nullptr); // Active allocation can never be overwritten!

            _context = other._context;
            _data    = other._data;

            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator=(Data&& other) noexcept -> Data&
        {
            if (this != &other)
            {
                ASSERT_ALLOCATOR_SAFETY(_data == nullptr); // Active allocation can never be overwritten!

                _context = other._context;
                _data    = other._data;
                other._context = nullptr;
                other._data    = nullptr;
            }
            return *this;
        }


        FORCE_INLINE explicit
        Data(Context& context)
            : _context{ &context }
            , _data{ nullptr }
        {
        }


        NO_DISCARD FORCE_INLINE
        auto Allocate(const int32 size) -> int32
        {
            ASSERT_ALLOCATOR_SAFETY(_data == nullptr);
            return _context->Allocate(size, _data);
        }

        NO_DISCARD FORCE_INLINE
        auto Relocate(const int32 size) -> int32
        {
            ASSERT_ALLOCATOR_SAFETY(_data != nullptr); // Active allocation must be present!
            return _context->Relocate(size, _data);
        }

        FORCE_INLINE
        void Free()
        {
            _data = nullptr;
        }


        NO_DISCARD FORCE_INLINE
        auto Get() const -> const byte*
        {
            return _data;
        }

        NO_DISCARD FORCE_INLINE
        auto Get() -> byte*
        {
            return _data;
        }
    };
};
