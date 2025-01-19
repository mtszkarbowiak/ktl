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
    /// <param name="cursor"> The cursor to sum. If empty, identity (default ctor) is returned. </param>
    template<typename _C>
    NO_DISCARD
    auto Sum(_C&& cursor) -> std::decay_t<decltype(*cursor)>
    {
        // Note: The type must decay (remove reference and const) to avoid returning a reference to a temporary.
        using ValueType = std::decay_t<decltype(*cursor)>;

        ValueType sum{};
        for (; cursor; ++cursor)
            sum += *cursor;

        return sum;
    }

    /// <summary>
    /// Averages all elements of the cursor.
    /// </summary>
    /// <param name="cursor"> The cursor to average. Must not be empty. </param>
    template<typename _C>
    NO_DISCARD   
    auto Average(_C&& cursor) -> std::decay_t<decltype(*cursor)>
    {
        ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(cursor)); // Enumerator must not be empty.

        // Note: The type must decay (remove reference and const) to avoid returning a reference to a temporary.
        using ValueType = std::decay_t<decltype(*cursor)>;

        ValueType sum{ *cursor };
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
    auto operator|(_C&& cursor, ToAverage)
    {
        return Average(FORWARD(_C, cursor));
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