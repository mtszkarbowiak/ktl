// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Algorithms/Querying.h"
#include "Collections/Array.h"

namespace Querying
{
    /// <summary>
    /// Collects the elements of the collection into an array,
    /// with the specified capacity.
    /// </summary>
    template<
        typename A = HeapAlloc,
        typename G = DefaultGrowth,
        typename _C
    >
    NO_DISCARD
    auto ToArray(_C&& cursor, const int32 capacity) -> Array<decltype(*cursor), A, G>
    {
        using ElementType = decltype(*cursor);
        Array<ElementType, A, G> array{ capacity };

        for (; cursor; ++cursor)
            array.Add(MOVE(*cursor));

        return array;
    }

    /// <summary>
    /// Collects the elements of the collection into an array,
    /// which capacity is predicted by the cursor hint.
    /// </summary>
    template<
        typename A = HeapAlloc,
        typename G = DefaultGrowth,
        typename _C
    >
    NO_DISCARD
    auto ToArray(_C&& cursor) -> Array<decltype(*cursor), A, G>
    {
        const int32 predictedCount = cursor.Hint().Max.ValueOr(0);
        return ToArray<A, G, _C>(FORWARD(_C, cursor), predictedCount);
    }
}

//TODO(mtszkarbowiak) Implement GroupBy with Dictionary<TKey, Array<TValue>>
//TODO(mtszkarbowiak) Implement OrderBy with Array<T>
//TODO(mtszkarbowiak) Implement Reverse with Array<T>
//TODO(mtszkarbowiak) Implement Distinct with HashSet<T>
