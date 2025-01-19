// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"
#include "Language/Yolo.h"

/// <summary>
/// Default hash acquisition for a type. It uses the <c>GetHash</c> method of the type.
/// This solution keeps ability to use different hash functions while preserving
/// the ability to just use a simple method operating on private members.
/// </summary>
template<typename T>
struct HashOf
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const T& key) NOEXCEPT_Y -> uint32
    {
        return key.GetHash();
    }
};

template<>
struct HashOf<int32>
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const int32 key) NOEXCEPT_Y -> uint32
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint32>
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const uint32 key) NOEXCEPT_Y -> uint32
    {
        return key;
    }
};

template<>
struct HashOf<int16>
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const int16 key) NOEXCEPT_Y -> uint32
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint16>
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const uint16 key) NOEXCEPT_Y -> uint32
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<int8>
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const int8 key) NOEXCEPT_Y -> uint32
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint8>
{
    NO_DISCARD static FORCE_INLINE
    auto GetHash(const uint8 key) NOEXCEPT_Y -> uint32
    {
        return key;
    }
};
