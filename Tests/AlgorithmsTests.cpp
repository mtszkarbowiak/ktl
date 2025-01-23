// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Algorithms/Sorting.h"
#include "Collections/Array.h"

using SortingAlgorithmTestParams = ::testing::Types<
    QuickSort<int32_t>,
    InsertionSort<int32_t>,
    MergeSort<int32_t>,
    TimSort<int32_t>
>;

template <typename Param>
struct SortingAlgorithmsFixture : public ::testing::Test
{
    const Array<int32> Input    = Array<int32>::Of<int32>({ 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 });
    const Array<int32> Expected = Array<int32>::Of<int32>({ 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 });
};


TYPED_TEST_SUITE(SortingAlgorithmsFixture, SortingAlgorithmTestParams);

TYPED_TEST(SortingAlgorithmsFixture, Sort)
{
    Array<int32> inputCopy = this->Input;

    // Apply the sorting algorithm
    TypeParam::Sort(inputCopy.AsSpan());

    // Check that the array is sorted correctly
    for (int32 i = 0; i < inputCopy.Count(); ++i)
    {
        EXPECT_EQ(inputCopy[i], this->Expected[i]);
    }
}
