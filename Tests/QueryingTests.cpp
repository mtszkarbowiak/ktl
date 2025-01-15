// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Algorithms/Aggregation.h"
#include "Algorithms/Querying.h"
#include "Allocators/FixedAlloc.h"
#include "Collections/Array.h"
#include "Collections/Dictionary.h"
#include "Collections/Ring.h"
#include "Math/Stastics.h"


TEST(QueryCount, ArrayHeap)
{
    const auto array = Array<int32, HeapAlloc>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(array.Values()), 3);
}

TEST(QueryCount, ArrayFixed)
{
    const auto array = Array<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(array.Values()), 3);
}

TEST(QueryCount, RingHeap)
{
    const auto ring = Ring<int32, HeapAlloc>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(ring.Values()), 3);
}

TEST(QueryCount, RingFixed)
{
    const auto ring = Ring<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    GTEST_ASSERT_EQ(Querying::Count(ring.Values()), 3);
}


TEST(QuerySelect, Array)
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

TEST(QueryWhere, Array)
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

TEST(QuerySelectWhere, Array)
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


TEST(QueryToArray, Array)
{
    using namespace Querying;
    using namespace Statistics;
    const auto array = Array<int32>::Of({ 1, 2, 3, 4, 5 });
    const auto selector = [](const int32 value) { return value * 2; };
    const auto predicate = [](const int32 value) { return value % 4 == 0; };

    const auto result = ToArray<FixedAlloc<128>>(
        array.Values()
            | Select<decltype(selector)>(selector)
            | Where<decltype(predicate)>(predicate)
    );

    // Expected: { 4, 8 }
    GTEST_ASSERT_EQ(result.Count(), 2);
    GTEST_ASSERT_EQ(result[0], 4);
    GTEST_ASSERT_EQ(result[1], 8);
}
