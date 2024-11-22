// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"

namespace Statistics 
{
    /// <summary> Calculates Residual Sum of Squares. </summary>
    template<typename EnumeratorA, typename EnumeratorB>
    NODISCARD
    static auto Rss(
        EnumeratorA&& enumeratorA,
        EnumeratorB&& enumeratorB
    )
    {
        using Number = decltype(*enumeratorA);

        auto rss = Number{};

        while (enumeratorA)
        {
            ASSERT(static_cast<bool>(enumeratorB));

            const Number& a = *enumeratorA;
            const Number& b = *enumeratorB;

            const auto residual        = b - a;
            const auto residualSquared = residual * residual;

            rss += residualSquared;

            ++enumeratorA;
            ++enumeratorB;
        }

        return rss;
    }
}