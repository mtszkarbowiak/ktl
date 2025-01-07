// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Numbers.h"

template<typename T>
struct HashOf
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const T& key)
    {
        return key;
    }
};

template<>
struct HashOf<int32>
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const int32 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint32>
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const uint32 key)
    {
        return key;
    }
};

template<>
struct HashOf<int16>
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const int16 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint16>
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const uint16 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<int8>
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const int8 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint8>
{
    NO_DISCARD static FORCE_INLINE
    uint32 GetHash(const uint8 key)
    {
        return key;
    }
};
