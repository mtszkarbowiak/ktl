// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Language/Communism.h"
#include "Language/Templates.h"
#include "Types/Span.h"

class Sorting
{
public:
    /// <summary> 
    /// Strategy of selecting the pivot element for the partitioning in QuickSort. 
    /// </summary>
    enum class PivotMode
    {
        /// <summary>
        /// Always select the first element as the pivot.
        /// To be used when the data is already sorted or nearly sorted.
        /// </summary>
        First,

        /// <summary>
        /// Always select the middle element as the pivot.
        /// To be when the data is neither sorted nor reversed.
        /// </summary>
        Middle,

        /// <summary>
        /// Always select the last element as the pivot.
        /// To be for datasets that have a random or unknown distribution.
        /// </summary>
        Last,

        /// <summary>
        /// Use the median of the first, middle, and last elements to select the pivot.
        /// To be used to always get the best-case performance.
        /// </summary>
        Median,
    };


PRIVATE:
    template<typename T>
    static auto SelectMedian(T* begin, T* end) -> T*
    {
        T* first  = begin;
        T* middle = begin + (end - begin) / 2;
        T* last   = end - 1;

        if (*first < *middle)
        {
            if (*middle < *last)
                return middle;
            if (*first < *last)
                return last;
            return first;
        }
        else
        {
            if (*first < *last)
                return first;
            if (*middle < *last)
                return last;
            return middle;
        }
    }

    template<typename T>
    static auto Partition(T* begin, T* end, const PivotMode pivotMode) -> T*
    {
        T* pivot = nullptr;

        // Select the pivot based on the strategy.
        switch (pivotMode)
        {
        case PivotMode::First:
            pivot = begin;
            break;
        case PivotMode::Middle:
            pivot = begin + (end - begin) / 2;
            break;
        case PivotMode::Last:
            pivot = end - 1;
            break;
        case PivotMode::Median:
            pivot = SelectMedian(begin, end);
            break;
        }

        // Move pivot to the end for partitioning
        ::Swap<T>(*pivot, *(end - 1));
        pivot = end - 1;

        T* left = begin;
        T* right = end - 2;  // Exclude pivot element (pivot is now at the end).

        while (left <= right)  // Adjust loop condition to ensure left and right don't cross too early.
        {
            // Move left pointer to the right until we find a value greater than or equal to pivot.
            while (*left < *pivot)
                ++left;

            // Move right pointer to the left until we find a value smaller than or equal to pivot.
            while (*right > *pivot)
                --right;

            // If left pointer is less than or equal to right pointer, swap the elements.
            if (left <= right)
            {
                ::Swap<T>(*left, *right);
                ++left;
                --right;
            }
        }

        // Place pivot in its correct position.
        ::Swap<T>(*left, *(end - 1));

        return left;  // Return the position of the pivot.
    }


public:
    template<typename T>
    static void QuickSort(T* begin, T* end, const PivotMode pivotMode = PivotMode::Middle)
    {
        if (begin < end)
        {
            T* pivot = Partition<T>(begin, end, pivotMode);

            QuickSort(begin, pivot, pivotMode);
            QuickSort(pivot + 1, end, pivotMode);
        }
    }

    template<typename T>
    static void QuickSort(Span<T> span)
    {
        QuickSort(
            span.Data(), 
            span.Data() + span.Count()
        );
    }



    //template<typename T>
    //static void TimSort(T* begin, T* end)
    //{
    //    //TODO TimSort
    //}
};
