// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Algorithms/Querying.h"
#include "Allocators/FixedAlloc.h"
#include "Collections/Array.h"
#include "Collections/Dictionary.h"
#include "Collections/Ring.h"
#include "Math/Stastics.h"


TEST(QueryingTests_Count, ArrayHeap)
{
    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(array.Values()), 3);
}

TEST(QueryingTests_Count, ArrayFixed)
{
    const auto array = Array<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(array.Values()), 3);
}

TEST(QueryingTests_Count, RingHeap)
{
    const auto ring = Ring<int32, HeapAlloc>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(ring.Values()), 3);
}

TEST(QueryingTests_Count, RingFixed)
{
    const auto ring = Ring<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(ring.Values()), 3);
}


TEST(QueryingTests_Select, Array)
{
    using namespace Querying;
    using namespace Statistics;

    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    const auto selector = [](const int32 value) { return value * 2; };

    using SelectDoubled = Select<decltype(selector)>;

    auto selected = array.Values()
        | Select<decltype(selector)>(selector)
        | SelectDoubled(selector);

    const auto expected = Array<int32, HeapAlloc>::Of({ 4, 8, 12 });

    GTEST_ASSERT_EQ(
        Sum(MOVE(selected)), 
        Sum(expected.Values())
    );
}

TEST(QueryingTests_Where, Array)
{
    using namespace Querying;
    using namespace Statistics;

    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    const auto predicate = [](const int32 value) { return value % 2 == 0; };

    using WhereEven = Where<decltype(predicate)>;

    auto selected = array.Values()
        | Where<decltype(predicate)>(predicate)
        | WhereEven(predicate);

    const auto expected = Array<int32, HeapAlloc>::Of({ 2 });
    GTEST_ASSERT_EQ(
        Sum(MOVE(selected)),
        Sum(expected.Values())
    );
}

TEST(QueryingTests_SelectWhere, Array)
{
    using namespace Querying;
    using namespace Statistics;

    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3, 4, 5 });

    const auto selector  = [](const int32 value) { return value * 2; };
    const auto predicate = [](const int32 value) { return value % 4 == 0; };

    {
        auto query = array.Values()
            | Select<decltype(selector)>(selector)
            | Where<decltype(predicate)>(predicate);

        GTEST_ASSERT_EQ(
            Sum(MOVE(query)),
            (2 * 2) + (4 * 2)
        );
    }

    {
        const int32 query = array.Values()
            | Select<decltype(selector)>(selector)
            | Where<decltype(predicate)>(predicate)
            | CountElements();

        GTEST_ASSERT_EQ(query, 2);
    }
}

TEST(QueryingTest_Sum, Dictionary)
{
    using namespace Querying;
    using namespace Statistics;

    Dictionary<int32, int32> dict;
    dict.Add(1, 5);
    dict.Add(2, 8);
    dict.Add(3, 17);

    const int32 keysSum = Sum(dict.Keys());
    const int32 valuesSum = Sum(dict.Values());

    GTEST_ASSERT_EQ(keysSum, 6);
    GTEST_ASSERT_EQ(valuesSum, 30);
}
