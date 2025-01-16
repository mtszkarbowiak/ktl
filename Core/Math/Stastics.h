// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Keywords.h"

namespace Statistics 
{
    /// <summary>
    /// Returns a pointer to the smallest element in the cursor.
    /// </summary>
    template<typename _C>
    NO_DISCARD
    auto Min(_C&& cursor) -> typename std::remove_reference<decltype(&*cursor)>::type
    {
        if (!cursor)
            return nullptr;

        auto* min = &*cursor;

        for (; cursor; ++cursor) {
            if (*cursor < *min)
                min = &*cursor;
        }

        return min;
    }


    /// <summary>
    /// Returns a pointer to the biggest element in the cursor.
    /// </summary>
    template<typename _C>
    NO_DISCARD
    auto Max(_C&& cursor) -> typename std::remove_reference<decltype(&*cursor)>::type
    {
        if (!cursor)
            return nullptr;

        auto* min = &*cursor;

        for (; cursor; ++cursor) {
            if (*cursor > *min)
                min = &*cursor;
        }

        return min;
    }


    /// <summary>
    /// Sums all elements of the cursor.
    /// </summary>
    /// <remarks>
    /// If the enumerator is empty, the result is the default value of the element type.
    /// Check for emptiness before calling this method if the default value is not the desired result.
    /// </remarks>
    template<typename _C>
    NO_DISCARD
    auto Sum(_C&& cursor) -> std::decay_t<decltype(*cursor)>
    {
        using ValueType = std::decay_t<decltype(*cursor)>;
        ValueType sum{};

        for (; cursor; ++cursor)
            sum += *cursor;

        return sum;
    }

    /// <summary>
    /// Averages all elements of the cursor.
    /// </summary>
    /// <remarks>
    /// If the enumerator is empty, the result is the default value of the element type.
    /// Check for emptiness before calling this method if the default value is not the desired result.
    /// </remarks>
    template<typename _C>
    NO_DISCARD   
    auto Average(_C&& cursor) -> decltype(*cursor)
    {
        ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(cursor)); // Enumerator must not be empty.

        auto sum{ *cursor };
        int32 count = 1;
        ++cursor;

        for (; cursor; ++cursor)
        {
            sum += *cursor;
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
    auto operator|(_C&& cursor, ToMin)
    {
        return Min(FORWARD(_C, cursor));
    }

    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& cursor, ToMax)
    {
        return Max(FORWARD(_C, cursor));
    }

    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& cursor, ToSum)
    {
        return Sum(FORWARD(_C, cursor));
    }

    template<typename _C>
    NO_DISCARD
    auto operator|(_C&& producer, ToAverage)
    {
        return Average(FORWARD(_C, producer));
    }


    /// <summary> Calculates Residual Sum of Squares. </summary>
    template<typename _CA, typename _CB>
    NO_DISCARD
    auto Rss(
        _CA&& cursorA,
        _CB&& cursorB
    )
    {
        using Number = decltype(*cursorA);

        auto rss = Number{};

        while (cursorA)
        {
            ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(cursorB));

            const Number& a = *cursorA;
            const Number& b = *cursorB;

            const auto residual        = b - a;
            const auto residualSquared = residual * residual;

            rss += residualSquared;

            ++cursorA;
            ++cursorB;
        }

        return rss;
    }

    
    struct ConfusionMatrix
    {
        float TruePositive;
        float TrueNegative;
        float FalsePositive;
        float FalseNegative;


        NO_DISCARD FORCE_INLINE constexpr
        auto Sum() const -> float
        {
            return TruePositive + TrueNegative + FalsePositive + FalseNegative;
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Accuracy() const -> float
        {
            return (TruePositive + TrueNegative) / Sum();
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Precision() const -> float
        {
            return TruePositive / (TruePositive + FalsePositive);
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto Recall() const -> float
        {
            return TruePositive / (TruePositive + FalseNegative);
        }

        NO_DISCARD FORCE_INLINE constexpr
        auto F1() const -> float
        {
            const auto precision = Precision();
            const auto recall = Recall();
            return (2.0f * precision * recall) / (precision + recall);
        }
    };
}