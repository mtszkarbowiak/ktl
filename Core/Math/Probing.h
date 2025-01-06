// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"

/// <summary>
/// General-purpose strategy of resolving hash collisions in open-addressing hash collections.
/// It moves through elements one-by-one. Can be used with arbitrary collection sizes.
/// Its downside is it increases chances of clustering in case of ineffective hash functions.
/// Produced sequence: 0, 1, 2, 3, ...
/// </summary>
class LinearProbing
{
    // Linear probing works for every load factor.

public:
    NO_DISCARD static FORCE_INLINE
    int32 Next(const int32 size, const int32 numChecks)
    {
        return numChecks;
    }
};

/// <summary>
/// Specialized strategy of resolving hash collisions in open-addressing hash collections.
/// It moves through elements by squares. Use with extreme caution, as it has very high chance
/// of not covering all elements. Produced sequence: 0, 1, 4, 9, 16, ...
/// </summary>
class QuadraticProbing
{
public:
    NO_DISCARD static FORCE_INLINE
    int32 Next(const int32 size, const int32 numChecks)
    {
        return numChecks * numChecks;
    }
};

/// <summary>
/// Specialized strategy of resolving hash collisions in open-addressing hash collections.
/// It moves through first n elements linearly and then starts jumping by n. Use with caution,
/// as it has some chance of not covering all elements, especially with high load factors.
/// Produced sequence: 0, 1, 2, ..., n, 2n, 3n, ...
/// </summary>
///
/// <typeparam name="N">
/// The number of elements to jump after the initial linear search.
/// Always should be a prime number, higher than 2.
/// </typeparam>
template<int32 N = 7>
class JumpProbing
{
    // Check if higher than 2
    static_assert(N > 2, "The jump must be higher than 2.");

public:
    NO_DISCARD static FORCE_INLINE
    int32 Next(const int32 size, const int32 numChecks)
    {
        return numChecks < N
            ? numChecks
            : (numChecks - N) * N;
    }
};
