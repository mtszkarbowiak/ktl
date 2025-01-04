// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

template<typename T>
struct HashOf
{
    static FORCE_INLINE
    uint32 GetHash(const T& key)
    {
        return key;
    }
};

template<>
struct HashOf<int32>
{
    static FORCE_INLINE
    uint32 GetHash(const int32 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint32>
{
    static FORCE_INLINE
    uint32 GetHash(const uint32 key)
    {
        return key;
    }
};

template<>
struct HashOf<int16>
{
    static FORCE_INLINE
    uint32 GetHash(const int16 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint16>
{
    static FORCE_INLINE
    uint32 GetHash(const uint16 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<int8>
{
    static FORCE_INLINE
    uint32 GetHash(const int8 key)
    {
        return static_cast<uint32>(key);
    }
};

template<>
struct HashOf<uint8>
{
    static FORCE_INLINE
    uint32 GetHash(const uint8 key)
    {
        return key;
    }
};
