// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"

namespace Statistics 
{
    /// <summary>
    /// Averages all elements in the enumerator.
    /// </summary>
    /// <remarks> Calling this method on an empty enumerator is UB. </remarks>
    template<typename Enumerator>
    auto static Min(Enumerator&& enumerator)
    {
        ASSERT_COLLECTION_SAFE_ACCESS((enumerator)); // Enumerator must not be empty.

        auto* min = &*enumerator;

        for (; enumerator; ++enumerator) {
            if (*enumerator < *min)
                min = &*enumerator;
        }

        return *min;
    }

    /// <summary>
    /// Averages all elements in the enumerator.
    /// </summary>
    /// <remarks> Calling this method on an empty enumerator is UB. </remarks>
    template<typename Enumerator>
    auto static Max(Enumerator&& enumerator)
    {
        ASSERT_COLLECTION_SAFE_ACCESS((enumerator)); // Enumerator must not be empty.

        auto* max = &*enumerator;

        for (; enumerator; ++enumerator) {
            if (*max < *enumerator)
                max = &*enumerator;
        }

        return *max;
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
        ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(enumerator)); // Enumerator must not be empty.

        auto sum{ *enumerator };
        ++enumerator;

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
        ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(enumerator)); // Enumerator must not be empty.

        auto sum{ *enumerator };
        int32 count = 1;
        ++enumerator;

        for (; enumerator; ++enumerator)
        {
            sum += *enumerator;
            count += 1;
        }

        return sum / static_cast<decltype(sum)>(count);
    }


    struct ToMin {};

    struct ToMax {};

    struct ToSum {};

    struct ToAverage {};


    template<typename Producer>
    auto operator|(Producer&& producer, ToMin)
    {
        return Min(FORWARD(Producer, producer));
    }

    template<typename Producer>
    auto operator|(Producer&& producer, ToMax)
    {
        return Max(FORWARD(Producer, producer));
    }

    template<typename Producer>
    auto operator|(Producer&& producer, ToSum)
    {
        return Sum(FORWARD(Producer, producer));
    }

    template<typename Producer>
    auto operator|(Producer&& producer, ToAverage)
    {
        return Average(FORWARD(Producer, producer));
    }


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
            ASSERT_COLLECTION_SAFE_ACCESS(static_cast<bool>(enumeratorB));

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

    
    struct ConfusionMatrix
    {
        float TruePositive;
        float TrueNegative;
        float FalsePositive;
        float FalseNegative;


        NODISCARD
        constexpr float Sum() const
        {
            return TruePositive + TrueNegative + FalsePositive + FalseNegative;
        }

        NODISCARD
        constexpr float Accuracy() const
        {
            return (TruePositive + TrueNegative) / Sum();
        }

        NODISCARD
        constexpr float Precision() const
        {
            return TruePositive / (TruePositive + FalsePositive);
        }

        NODISCARD
        constexpr float Recall() const
        {
            return TruePositive / (TruePositive + FalseNegative);
        }

        NODISCARD
        constexpr float F1() const
        {
            const auto precision = Precision();
            const auto recall = Recall();
            return (2.0f * precision * recall) / (precision + recall);
        }
    };
}