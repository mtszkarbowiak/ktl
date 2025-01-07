// Created by Mateusz Karbowiak 2024

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Memory.h"
#include "Types/Numbers.h"

#include <cstdint>

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
        byte* _data;
        int32 _size;
        int32 _bump;
        int32 _alignment;

    public:
        explicit
        Context(byte* data, const int32 size, const int32 alignment = sizeof(void*))
            : _data{ data }
            , _size{ size }
            , _bump{ 0 }
            , _alignment{ alignment }
        {
        }

        FORCE_INLINE
        auto Allocate(const int32 size, byte*& result) -> int32
        {
            const int32 newOffset = _bump + size;

            if (newOffset > _size)
            {
                return 0;
            }

            result = _data + _bump;
            _bump = newOffset;

            //TODO(mtszkarbowiak) Add alignment support.

            return size;
        }

        FORCE_INLINE
        void Reset()
        {
            _bump = 0;
        }

        FORCE_INLINE
        auto FreeSpace() const -> int32
        {
            return _size - _bump;
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
        Data()
            : _context{ nullptr }
            , _data{ nullptr }
        {
        }

        Data(const Data& other)
        {
            _context = other._context;
            _data    = other._data;
        }

        Data(Data&& other) noexcept
            : _context{ other._context }
            , _data{ other._data }
        {
            other._context = nullptr;
            other._data    = nullptr;
        }


        auto operator=(const Data& other) -> Data&
        {
            _context = other._context;
            _data    = other._data;
            return *this;
        }

        auto operator=(Data&& other) noexcept -> Data&
        {
            if (this != &other)
            {
                _context = other._context;
                _data    = other._data;
                other._context = nullptr;
                other._data    = nullptr;
            }
            return *this;
        }


        explicit
        Data(Context& context)
            : _context{ &context }
            , _data{ nullptr }
        {
        }


        FORCE_INLINE auto Allocate(const int32 size) -> int32
        {
            ASSERT_ALLOCATOR_SAFETY(_data == nullptr);
            return _context->Allocate(size, _data);
        }

        FORCE_INLINE
        void Free()
        {
            _data = nullptr;
        }


        FORCE_INLINE
        auto Get() const -> const byte*
        {
            return _data;
        }

        FORCE_INLINE
        auto Get() -> byte*
        {
            return _data;
        }
    };
};
