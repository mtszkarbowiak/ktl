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
            | ToCount();

        GTEST_ASSERT_EQ(query, 2);
    }
}

//TEST(QueryingTest_Sum, Dictionary)
//{
//    using namespace Querying;
//    using namespace Statistics;
//
//    Dictionary<int32, float> dict;
//    dict.Add(1, 5.0f);
//    dict.Add(2, 8.0f);
//    dict.Add(3, 17.0f);
//    dict.Add(4, 3.0f);
//    dict.Add(5, 9.0f);
//    dict.Add(6, 11.0f);
//    dict.Add(7, 2.0f);
//
//    // Calculate the sum of all even keys
//
//    auto where = [](const int32 key) { return key % 2 == 0; };
//
//    const int32 sum = dict.Keys()
//        | Where<decltype(where)>(where)
//        | ToSum();
//
//    GTEST_ASSERT_EQ(sum, 12);
//
//    // Calculate the average of squares of values above 5.
//
//    auto where2 = [](const float value) { return value > 5.0f; };
//    auto select = [](const float value) { return value * value; };
//
//    const float wtfNumba = dict.Values()
//        | Where<decltype(where2)>(where2)
//        | Select<decltype(select)>(select)
//        | ToAverage();
//
//    GTEST_ASSERT_GE(wtfNumba, 138.0f);
//}
