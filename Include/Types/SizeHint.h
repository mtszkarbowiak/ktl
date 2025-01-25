// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Types/Index.h"

/// <summary>
/// Hint from the iterator about the number of elements to iterate over.
/// Both numbers are inclusive, to allow for entire range [0, Count] to be covered.
/// Of course, 0 means no elements to iterate over.
/// </summary>
struct SizeHint final
{
    /// <summary>
    /// Minimal number of elements to iterate over.
    /// </summary>
    Index Min{};

    /// <summary>
    /// Maximal number of elements to iterate over.
    /// <b>Includes the current element.</b>
    /// </summary>
    Nullable<Index> Max{};
};
