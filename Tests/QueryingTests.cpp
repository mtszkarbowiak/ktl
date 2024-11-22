// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Algorithms/Querying.h"
#include "Allocators/FixedAlloc.h"
#include "Collections/Array.h"
#include "Collections/Ring.h"


TEST(QueryingTests, ArrayHeap_Count)
{
    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    ASSERT_EQ(Querying::Count(array.Vals()), 3);
}

TEST(QueryingTests, ArrayFixed_Count)
{
    const auto array = Array<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    ASSERT_EQ(Querying::Count(array.Vals()), 3);
}

TEST(QueryingTests, RingHeap_Count)
{
    const auto ring = Ring<int32, HeapAlloc>::Of({ 1, 2, 3 });
    ASSERT_EQ(Querying::Count(ring.Enumerate()), 3);
}

TEST(QueryingTests, RingFixed_Count)
{
    const auto ring = Ring<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    ASSERT_EQ(Querying::Count(ring.Enumerate()), 3);
}
