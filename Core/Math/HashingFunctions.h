// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

inline uint32 GetHash(const uint8 key)
{
    return key;
}

inline uint32 GetHash(const int8 key)
{
    return key;
}

inline uint32 GetHash(const uint16 key)
{
    return key;
}

inline uint32 GetHash(const int16 key)
{
    return key;
}

inline uint32 GetHash(const int32 key)
{
    return key;
}

inline uint32 GetHash(const uint32 key)
{
    return key;
}
