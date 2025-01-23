// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"
#include "Language/Memory.h"
#include "Language/Yolo.h"

// Member Hashing

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


// Trivial Hashing

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


// POD Hashing

class HashUtils final
{
PRIVATE:
    static constexpr uint32 FnvOffestBasis = 0x811C9DC5;
    static constexpr uint32 FnvPrime = 0x01000193;

public:
    NO_DISCARD static FORCE_INLINE
    auto FowlerNollVo(const void* key, const uintptr size) NOEXCEPT_Y -> uint32
    {
        const byte* data = static_cast<const byte*>(key);
        uint32_t hash = FnvOffestBasis;

        hash ^= static_cast<uint32>(size & 0xFFFFFFFF); // Mix with size for more entropy

        for (uintptr i = 0; i < size; ++i)
        {
            hash ^= (data[i] ^ static_cast<byte>(i)); // Mix with index for more entropy
            hash *= FnvPrime;
        }

        return hash;
    }
};

template<typename T>
struct PodHashOf
{
    static_assert(TIsPODV<T>, "Type must be a POD type.");

    NO_DISCARD static FORCE_INLINE
    auto GetHash(const T& key) NOEXCEPT_Y -> uint32
    {
        return HashUtils::FowlerNollVo(&key, sizeof(T));
    }
};
