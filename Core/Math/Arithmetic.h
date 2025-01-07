// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Types/Numbers.h"

namespace Math
{
    /// <summary> Returns the smallest of two values. </summary>
    /// <remarks> The values must be comparable with the less-than operator. </remarks>
    template<typename T>
    static FORCE_INLINE constexpr
    auto Min(const T a, const T b) -> T
    {
        return a < b ? a : b;
    }

    /// <summary> Returns the largest of two values. </summary>
    /// <remarks> The values must be comparable with the less-than operator. </remarks>
    template<typename T>
    static FORCE_INLINE constexpr
    auto Max(const T a, const T b) -> T
    {
        return b < a ? a : b;
    }

    /// <summary> Clamps the value between the given minimum and maximum. </summary>
    /// <remarks> The values must be comparable with the less-than operator. </remarks>
    template<typename T>
    static FORCE_INLINE constexpr
    auto Clamp(const T value, const T min, const T max) -> T
    {
        if (value < min)
        {
            return min;
        }

        if (max < value)
        {
            return max;
        }

        return value;
    }


    /// <summary> Returns the smallest power of 2 greater or equal to the given number. </summary>
    static FORCE_INLINE constexpr
    auto NextPow2(const int32 count) -> int32
    {
        int32 capacity = count - 1;
        capacity |= capacity >> 1;
        capacity |= capacity >> 2;
        capacity |= capacity >> 4;
        capacity |= capacity >> 8;
        capacity |= capacity >> 16;
        return capacity + 1;
    }

    /// <summary> Returns the highest power of 2 smaller or equal to the given number. </summary>
    static FORCE_INLINE constexpr
    auto PrevPow2(const int32 count) -> int32
    {
        if (count <= 0) return 0; // Handle invalid input gracefully.
        int32 capacity = count;
        capacity |= capacity >> 1;
        capacity |= capacity >> 2;
        capacity |= capacity >> 4;
        capacity |= capacity >> 8;
        capacity |= capacity >> 16;
        return capacity - (capacity >> 1);
    }

    /// <summary> Checks if the given number is a power of 2. </summary>
    static FORCE_INLINE constexpr
    auto IsPow2(const int32 count) -> bool
    {
        return count > 0 && (count & (count - 1)) == 0;
    }
}
