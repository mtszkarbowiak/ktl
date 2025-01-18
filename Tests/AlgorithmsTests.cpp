// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Algorithms/Sorting.h"
#include "Collections/Array.h"

TEST(Algorithms, QuickSort)
{
    Array<int32> array = Array<int32>::Of({ 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 });
    const Array<int32> expected = Array<int32>::Of({ 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 });

    Sorting::QuickSort(array.AsSpan());

    for (int32 i = 0; i < array.Count(); ++i)
    {
        EXPECT_EQ(array[i], expected[i]);
    }
}
