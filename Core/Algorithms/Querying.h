// Created by Mateusz Karbowiak 2024

#pragma once

class Querying
{
public:
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
    template<typename Enumerator>
    auto static Average(Enumerator&& enumerator)
    {
        using Aggregate = std::remove_reference_t<decltype(*enumerator)>;

        Aggregate sum {};
        int32 count = 0;
        for (; enumerator; ++enumerator)
        {
            sum   += *enumerator;
            count += 1;
        }

        return sum / static_cast<Aggregate>(count);
    }
};
