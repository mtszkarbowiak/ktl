// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Types/Numbers.h"

/// <summary>
/// Hint from the iterator about the number of elements to iterate over.
/// Both numbers are inclusive, to allow for entire range [0, Count] to be covered.
/// Of course, 0 means no elements to iterate over.
/// </summary>
class SizeHint final
{
    int32 _min{};
    int32 _max{};

    // Access

public:
    /// <summary>
    /// Minimal number of elements to iterate over.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto Min() const -> int32
    {
        return _min;
    }

    /// <summary>
    /// Checks if the maximal number of elements is defined.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto HasMax() const -> bool
    {
        return _max != -1;
    }

    /// <summary>
    /// Maximal number of elements to iterate over.
    /// <b>Includes the current element.</b>
    /// May be undefined if the iterator does not know the exact maximal number of elements.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto Max() const -> int32
    {
        ASSERT(_max != -1); // Max must be defined.
        return _max;
    }

    /// <summary>
    /// Maximal number of elements to iterate over.
    /// <b>Includes the current element.</b>
    /// May be undefined if the iterator does not know the exact maximal number of elements.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto MaxOr(const int32 fallback) const -> int32
    {
        if (_max == -1)
        {
            return fallback;
        }

        return _max;
    }


    // Factorization

    NO_DISCARD FORCE_INLINE
    static auto Empty() -> SizeHint
    {
        return SizeHint{};
    }

    NO_DISCARD FORCE_INLINE
    static auto Exactly(const int32 count) -> SizeHint
    {
        SizeHint hint;
        hint._min = count;
        hint._max = count;
        return hint;
    }

    NO_DISCARD FORCE_INLINE
    static auto AtLeast(const int32 min) -> SizeHint
    {
        SizeHint hint;
        hint._min = min;
        hint._max = -1;
        return hint;
    }

    NO_DISCARD FORCE_INLINE
    static auto Range(const int32 min, const int32 max) -> SizeHint
    {
        SizeHint hint;
        hint._min = min;
        hint._max = max;
        return hint;
    }
};
