// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/Array.h"
#include "Collections/Ring.h"
#include "Debugging/LifecycleTracker.h"
#include "Math/Stastics.h"

TEST(StatisticsTests, DescribeArray)
{
    using namespace Statistics;

    const auto array = Array<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });
    
    GTEST_ASSERT_EQ(    Sum(array.Values()), 6);
    GTEST_ASSERT_EQ(Average(array.Values()), 2);
    GTEST_ASSERT_EQ(    Min(array.Values()), 1);
    GTEST_ASSERT_EQ(    Max(array.Values()), 3);
}

TEST(StatisticsTests, DescribeRing)
{
    using namespace Statistics;

    const auto ring = Ring<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });

    GTEST_ASSERT_EQ(    Sum(ring.Values()), 6);
    GTEST_ASSERT_EQ(Average(ring.Values()), 2);
    GTEST_ASSERT_EQ(    Min(ring.Values()), 1);
    GTEST_ASSERT_EQ(    Max(ring.Values()), 3);
}


TEST(StasticsTests, Rss)
{
    const auto set1 = Array<float>::Of({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f });
    const auto set2 = Array<float>::Of({ 1.0f, 2.0f, 3.0f, 4.0f, 6.0f });

    const auto rss = Statistics::Rss(
        set1.Values(),
        set2.Values()
    );

    EXPECT_FLOAT_EQ(rss, 1.0f);
}
