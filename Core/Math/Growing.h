// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"

/// <summary>
/// Multiplies the capacity by 1.5.
/// Default growth strategy for MSVC.
/// </summary>
class NaturalGrowth final
{
public:
    NO_DISCARD static FORCE_INLINE
    auto Grow(const int32 capacity) -> int32
    {
        ASSERT_COLLECTION_SAFE_MOD(capacity >= 2); // The collection must have at least 2 elements.
        return capacity + (capacity >> 1);
    }
};

/// <summary>
/// Multiplies the capacity by 2.
/// Default growth strategy for Clang and GCC.
/// </summary>
class DoubleGrowth final
{
public:
    NO_DISCARD static FORCE_INLINE
    auto Grow(const int32 capacity) -> int32
    {
        return capacity << 1;
    }
};

/// <summary>
/// Multiplies the capacity by 2 until it reaches the threshold.
/// Then, it multiplies the capacity by 1.5.
/// </summary>
template<int32 Threshold = 64>
class BalancedGrowth final
{
public:
    NO_DISCARD static FORCE_INLINE
    auto Grow(const int32 capacity) -> int32
    {
        ASSERT_COLLECTION_SAFE_MOD(capacity >= 2); // The collection must have at least 2 elements.
        return capacity < Threshold
            ? capacity << 1
            : capacity + (capacity >> 1);
    }
};

/// <summary>
/// Multiplies the capacity by 1.25.
/// To be used when the collection is expected to grow very slowly.
/// </summary>
class RelaxedGrowth final
{
public:
    NO_DISCARD static FORCE_INLINE
    auto Grow(const int32 capacity) -> int32
    {
        ASSERT_COLLECTION_SAFE_MOD(capacity >= 4); // The collection must have at least 4 elements.
        return capacity + (capacity >> 2);
    }
};

/// <summary>
/// Default growth strategy, to be used when the strategy is not important.
/// </summary>
class DefaultGrowth final
{
public:
    /// <summary>
    /// Default growth strategy, to be used when the strategy is not important.
    /// </summary>
    NO_DISCARD static FORCE_INLINE
    auto Grow(const int32 capacity) -> int32
    {
        return BalancedGrowth<64>::Grow(capacity);
    }
};

//TODO(mtszkarbowiak) Add minimal capacity for each growth strategy
