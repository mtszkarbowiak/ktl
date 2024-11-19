// Created by Mateusz Karbowiak 2024

#pragma once

namespace Querying
{
    /// <summary>
    /// Counts the number of elements in the enumerator.
    /// </summary>
    template<typename Enumerator>
    auto static Count(Enumerator&& enumerator) -> int32
    {
        int32 count = 0;
        for (; enumerator; ++enumerator)
            count += 1;
        return count;
    }

    /// <summary>
    /// Sums all elements in the enumerator.
    /// </summary>
    /// <remarks>
    /// If the enumerator is empty, the result is the default value of the element type.
    /// Check for emptiness before calling this method if the default value is not the desired result.
    /// </remarks>
    template<typename Enumerator>
    auto static Sum(Enumerator&& enumerator)
    {
        using Aggregate = std::remove_reference_t<decltype(*enumerator)>;

        Aggregate sum{};
        for (; enumerator; ++enumerator)
            sum += *enumerator;

        return sum;
    }

    /// <summary>
    /// Averages all elements in the enumerator.
    /// </summary>
    /// <remarks>
    /// If the enumerator is empty, the result is the default value of the element type.
    /// Check for emptiness before calling this method if the default value is not the desired result.
    /// </remarks>
    template<typename Enumerator>
    auto static Average(Enumerator&& enumerator)
    {
        using Aggregate = std::remove_reference_t<decltype(*enumerator)>;

        Aggregate sum{};
        int32 count = 0;
        for (; enumerator; ++enumerator)
        {
            sum   += *enumerator;
            count += 1;
        }

        return sum / static_cast<Aggregate>(count);
    }

    /// <summary>
    /// Averages all elements in the enumerator.
    /// </summary>
    /// <remarks> Calling this method on an empty enumerator is UB. </remarks>
    template<typename Enumerator>
    auto static Min(Enumerator&& enumerator)
    {
        using Aggregate = std::remove_reference_t<decltype(*enumerator)>;

        ASSERT((enumerator)); // Enumerator must not be empty.

        Aggregate min = *enumerator;
        for (; enumerator; ++enumerator)
            min = Math::Min(min, *enumerator);

        return min;
    }

    /// <summary>
    /// Averages all elements in the enumerator.
    /// </summary>
    /// <remarks> Calling this method on an empty enumerator is UB. </remarks>
    template<typename Enumerator>
    auto static Max(Enumerator&& enumerator)
    {
        using Aggregate = std::remove_reference_t<decltype(*enumerator)>;

        ASSERT((enumerator)); // Enumerator must not be empty.

        Aggregate max = *enumerator;
        for (; enumerator; ++enumerator)
            max = Math::Max(max, *enumerator);

        return max;
    }


    /// <summary>
    /// Finds the extremum element in the enumerator.
    /// Predicate returns true if the first argument is better than the second.
    /// </summary>
    /// <remarks> Calling this method on an empty enumerator is UB. </remarks>
    template<typename Enumerator, typename Predicate>
    auto static Extremum(Enumerator&& enumerator, Predicate&& predicate) -> decltype(*enumerator)
    {
        ASSERT((enumerator)); // Enumerator must not be empty.

        auto& extremum = *enumerator;
        for (; enumerator; ++enumerator)

        if (predicate(*enumerator, extremum))
        {
            extremum = *enumerator;
        }

        return extremum;
    }
};
