// Created by Mateusz Karbowiak 2024

#pragma once

#include "Numbers.h"

/// <summary>
/// Hint from the iterator about the number of elements to iterate over.
/// Both numbers are inclusive, to allow for entire range [0, Count] to be covered.
/// Of course, 0 means no elements to iterate over.
/// </summary>
struct IterHint final
{
    /// <summary>
    /// Minimal number of elements to iterate over.
    /// Non-nullable.
    /// Includes the current element.
    /// </summary>
    int32 Min{ 0 };

    /// <summary>
    /// Maximal number of elements to iterate over.
    /// Nullable, where -1 means no limit.
    /// Includes the current element.
    /// </summary>
    int32 Max{ -1 };
};
