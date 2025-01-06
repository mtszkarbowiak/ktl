// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"

/// <summary>
/// Set of functions used in hashing algorithms to resolve collisions.
/// </summary>
class LinearProbing
{
public:
    NO_DISCARD static FORCE_INLINE
    int32 Next(const int32 size, const int32 numChecks)
    {
        return numChecks;
    }
};

class QuadraticProbing
{
public:
    NO_DISCARD static FORCE_INLINE
    int32 Next(const int32 size, const int32 numChecks)
    {
        return numChecks * numChecks;
    }
};
