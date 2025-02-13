// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Templates.h"

namespace Statistics 
{
    /// <summary>
    /// Returns a pointer to the smallest element in the puller.
    /// </summary>
    template<typename _C>
    NO_DISCARD
    auto Min(_C&& puller) -> TRemoveCVRefT<decltype(&*puller)>*
    {
        if (!puller)
            return nullptr;

        auto* min = &*puller;

        for (; puller; ++puller) {
            if (*puller < *min)
                min = &*puller;
        }

        return min;
    }


    /// <summary>
    /// Returns a pointer to the biggest element in the puller.
    /// </summary>
    template<typename _C>
    NO_DISCARD
    auto Max(_C&& puller) -> TRemoveCVRefT<decltype(&*puller)>*
    {
        if (!puller)
            return nullptr;

        auto* min = &*puller;

        for (; puller; ++puller) {
            if (*puller > *min)
                min = &*puller;
        }

        return min;
    }


    /// <summary>
    /// Sums all elements of the puller.
    /// </summary>
    /// <param name="puller"> The puller to sum. If empty, identity (default ctor) is returned. </param>
    template<typename _C>
    NO_DISCARD
    auto Sum(_C&& puller)
    {
        // Note: The type must decay (remove reference and const) to avoid returning a reference to a temporary.
        using ValueType = TRemoveCVRefT<decltype(*puller)>;

        ValueType sum{};
        for (; puller; ++puller)
            sum += *puller;

        return sum;
    }

    /// <summary>
    /// Averages all elements of the puller.
    /// </summary>
    /// <param name="puller"> The puller to average. Must not be empty. </param>
    template<typename _C>
    NO_DISCARD   
    auto Average(_C&& puller)
    {
        ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(puller)); // Enumerator must not be empty.

        // Note: The type must decay (remove reference and const) to avoid returning a reference to a temporary.
        using ValueType = TRemoveCVRefT<decltype(*puller)>;

        ValueType sum{ *puller };
        int32 count = 1;
        ++puller;

        for (; puller; ++puller)
        {
            sum += *puller;
            count += 1;
        }

        return sum / static_cast<decltype(sum)>(count);
    }


    struct ToMin {};

    struct ToMax {};

    struct ToSum {};

    struct ToAverage {};


    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& puller, ToMin)
    {
        return Min(FORWARD(_C, puller));
    }

    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& puller, ToMax)
    {
        return Max(FORWARD(_C, puller));
    }

    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& puller, ToSum)
    {
        return Sum(FORWARD(_C, puller));
    }

    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& puller, ToAverage)
    {
        return Average(FORWARD(_C, puller));
    }


    /// <summary> Calculates Residual Sum of Squares. </summary>
    template<typename _CA, typename _CB>
    NO_DISCARD
    auto Rss(
        _CA&& pullerA,
        _CB&& pullerB
    )
    {
        using Number = decltype(*pullerA);

        auto rss = Number{};

        while (pullerA)
        {
            ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(pullerB));

            const Number& a = *pullerA;
            const Number& b = *pullerB;

            const auto residual        = b - a;
            const auto residualSquared = residual * residual;

            rss += residualSquared;

            ++pullerA;
            ++pullerB;
        }

        return rss;
    }


    template<typename T = float>
    struct ConfusionMatrix
    {
        T TruePositive;
        T TrueNegative;
        T FalsePositive;
        T FalseNegative;


        NO_DISCARD FORCE_INLINE constexpr
        auto Sum() const -> T
        {
            return TruePositive + TrueNegative + FalsePositive + FalseNegative;
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Accuracy() const -> T
        {
            return (TruePositive + TrueNegative) / Sum();
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Precision() const -> T
        {
            return TruePositive / (TruePositive + FalsePositive);
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Recall() const -> T
        {
            return TruePositive / (TruePositive + FalseNegative);
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto F1() const -> T
        {
            const auto precision = Precision();
            const auto recall = Recall();
            return (2.0f * precision * recall) / (precision + recall);
        }
    };
}