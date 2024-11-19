// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Array.h"
#include "Algorithms/Querying.h"
#include "Allocators/FixedAlloc.h"

TEST(QueryingTests, Count)
{
    using namespace Querying;

    Array<int32, FixedAlloc<4 * sizeof(int32)>> array;
    array.Add(1);
    array.Add(2);
    array.Add(3);

    ASSERT_EQ(Count    (array.Enumerate()), 3);
    ASSERT_EQ(Sum      (array.Enumerate()), 6);
    ASSERT_EQ(Average  (array.Enumerate()), 2);
    ASSERT_EQ(Min      (array.Enumerate()), 1);
    ASSERT_EQ(Max      (array.Enumerate()), 3);
}
