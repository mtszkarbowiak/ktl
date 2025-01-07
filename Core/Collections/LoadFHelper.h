// Created by Mateusz Karbowiak 2024

#pragma once

#include "Math/Arithmetic.h"

/// <summary>
/// Utility class to help perform calculations related to the load factor of a collection.
/// </summary>
/// <typeparam name="SlackRatio"></typeparam>
template<
    int32 SlackRatio
>
class LoadFHelperOf final
{
public:
    /// <summary>
    /// Calculates how many slots are required to store the specified number of elements,
    /// to preserve the load factor.
    /// </summary>
    NO_DISCARD static FORCE_INLINE constexpr
    auto SlotsForElements(const int32 elementCount) -> int32
    {
        return elementCount + (elementCount) / SlackRatio + 1;
    }
};
