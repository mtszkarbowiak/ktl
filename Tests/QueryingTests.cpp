// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Algorithms/Querying.h"
#include "Allocators/FixedAlloc.h"
#include "Collections/Array.h"
#include "Collections/Ring.h"
#include "Math/Stastics.h"


TEST(QueryingTests_Count, ArrayHeap)
{
    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(array.Vals()), 3);
}

TEST(QueryingTests_Count, ArrayFixed)
{
    const auto array = Array<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(array.Vals()), 3);
}

TEST(QueryingTests_Count, RingHeap)
{
    const auto ring = Ring<int32, HeapAlloc>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(ring.Vals()), 3);
}

TEST(QueryingTests_Count, RingFixed)
{
    const auto ring = Ring<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(ring.Vals()), 3);
}


TEST(QueryingTests_Select, Array)
{
    using namespace Querying;
    using namespace Statistics;

    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    const auto selector = [](const int32 value) { return value * 2; };

    using SelectX2 = Select<decltype(selector)>;

    auto selected = array.Vals()
        | Select<decltype(selector)>(selector)
        | SelectX2(selector);

    const auto expected = Array<int32, HeapAlloc>::Of({ 4, 8, 12 });

    GTEST_ASSERT_EQ(
        Sum(MOVE(selected)), 
        Sum(expected.Vals())
    );
}
