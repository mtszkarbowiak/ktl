// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/Array.h"
#include "Language/Communism.h"
#include "Language/Templates.h"
#include "Types/Span.h"


template<typename T>
class InsertionSort
{
public:
    /// <summary>
    /// Sorts the elements by repeatedly selecting the minimum element from the unsorted part
    /// and moving it to the beginning.
    /// </summary>
    static void Sort(T* begin, T* end)
    {
        for (T* current = begin + 1; current < end; ++current)
        {
            T* insertTarget = current;
            while (
                (insertTarget > begin) && // Ensure we don't go out of bounds.
                (*(insertTarget - 1) > *insertTarget) // Compare with the previous element.
                )
            {
                ::Swap<T>(*(insertTarget - 1), *insertTarget);
                --insertTarget;
            }
        }
    }

    /// <summary>
    /// Sorts the elements by repeatedly selecting the minimum element from the unsorted part
    /// and moving it to the beginning.
    /// </summary>
    static void Sort(Span<T> span)
    {
        Sort(span.begin(), span.end());
    }
};


template<typename T>
class MergeSort
{
public:
    /// <summary> 
    /// Merges two sorted arrays into a single sorted array.
    /// Both input arrays occupy the same memory block.
    /// </summary>
    static FORCE_INLINE
    void InplaceMerge(T* begin, T* mid, T* end)
    {
        while (begin < mid && mid < end)
        {
            if (*begin < *mid) 
            {
                ++begin;
            }
            else
            {
                T* temp = mid;
                while (temp != begin)
                {
                    ::Swap<T>(*temp, *(temp - 1));
                    --temp;
                }
                ++mid;
                ++begin;
            }
        }
    }

    static
    void Sort(T* begin, T* end)
    {
        // Sorted array must have at least two elements.
        if (end - begin < 2)
            return;

        // Split the array into two halves: [begin, mid) and [mid, end).
        T* mid = begin + (end - begin) / 2;

        // Recursively sort the two halves.
        Sort(begin, mid);
        Sort(mid, end);

        // Merge the two sorted halves.
        InplaceMerge(begin, mid, end);
    }

    static FORCE_INLINE
    void Sort(Span<T> span)
    {
        Sort(span.begin(), span.end());
    }
};


template<typename T, int32 MinRun = 32>
class TimSort
{
public:
    static
    void Sort(T* begin, T* end)
    {
        const int32 count = end - begin;

        // Sort small runs with insertion sort.
        for (T* current = begin; current < end; current += MinRun)
        {
            T* runEnd = Math::Min(current + MinRun, end);
            InsertionSort<T>::Sort(current, runEnd);
        }

        // Merge the runs.
        for (int32 runSize = MinRun; runSize < count; runSize *= 2)
        {
            for (T* current = begin; current < end; current += 2 * runSize)
            {
                T* runMid = current + runSize;
                T* runEnd = Math::Min(runMid + runSize, end);
                MergeSort<T>::InplaceMerge(current, runMid, runEnd);
            }
        }
    }

    static FORCE_INLINE
    void Sort(Span<T> span)
    {
        Sort(span.begin(), span.end());
    }
};


template<typename T>
class QuickSort
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
    static FORCE_INLINE
    auto SelectMedian(T* begin, T* end) -> T*
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

    static FORCE_INLINE
    auto Partition(T* begin, T* end, const PivotMode pivotMode) -> T*
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
    /// <summary>
    /// Sorts the elements by recursively partitioning the array into two parts,
    /// each storing elements less than and greater than the pivot.
    /// </summary>
    static
    void Sort(T* begin, T* end, const PivotMode pivotMode = PivotMode::Middle)
    {
        if (begin < end)
        {
            T* pivot = Partition(begin, end, pivotMode);

            Sort(begin, pivot, pivotMode);
            Sort(pivot + 1, end, pivotMode);
        }
    }

    /// <summary>
    /// Sorts the elements by recursively partitioning the array into two parts,
    /// each storing elements less than and greater than the pivot.
    /// </summary>
    static
    void Sort(Span<T> span)
    {
        Sort(span.begin(), span.end());
    }
};
