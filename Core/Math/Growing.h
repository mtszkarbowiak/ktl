// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"

/// <summary>
/// Set of functions used to calculate the size of a collection.
/// </summary>
/// <remarks>
/// This class explicitly avoids using any references to allocations or collections.
/// It is because the functions calculated requested capacity, not the actual capacity.
/// </remarks>
class Growing final
{
public:
    /// <summary>
    /// Multiplies the capacity by 1.5.
    /// Default growth strategy for MSVC.
    /// </summary>
    FORCE_INLINE
    static int32 Natural(const int32 capacity)
    {
        ASSERT_COLLECTION_SAFE_MOD(capacity >= 2); // The collection must have at least 2 elements.
        return capacity + (capacity >> 1);
    }

    /// <summary>
    /// Multiplies the capacity by 2.
    /// Default growth strategy for Clang and GCC.
    /// </summary>
    FORCE_INLINE
    static int32 Double(const int32 capacity)
    {
        return capacity << 1;
    }

    /// <summary>
    /// Multiplies the capacity by 2 until it reaches the threshold.
    /// Then, it multiplies the capacity by 1.5.
    /// </summary>
    template<int32 Threshold = 64>
    FORCE_INLINE
    static int32 Balanced(const int32 capacity)
    {
        ASSERT_COLLECTION_SAFE_MOD(capacity >= 2); // The collection must have at least 2 elements.
        return capacity < Threshold
            ? capacity << 1
            : capacity + (capacity >> 1);
    }

    /// <summary>
    /// Multiplies the capacity by 1.25.
    /// To be used when the collection is expected to grow very slowly.
    /// </summary>
    FORCE_INLINE
    static int32 Relaxed(const int32 capacity)
    {
        ASSERT_COLLECTION_SAFE_MOD(capacity >= 4); // The collection must have at least 4 elements.
        return capacity + (capacity >> 2);
    }


    /// <summary>
    /// Default growth strategy, to be used when the strategy is not important.
    /// </summary>
    FORCE_INLINE
    static int32 Default(const int32 capacity)
    {
        return Balanced(capacity);
    }
};
