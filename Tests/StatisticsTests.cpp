// Created by Mateusz Karbowiak 2024

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
    
    ASSERT_EQ(    Sum(array.Enumerate()), 6);
    ASSERT_EQ(Average(array.Enumerate()), 2);
    ASSERT_EQ(    Min(array.Enumerate()), 1);
    ASSERT_EQ(    Max(array.Enumerate()), 3);
}

TEST(StatisticsTests, DescribeRing)
{
    using namespace Statistics;

    const auto ring = Ring<int32, FixedAlloc<4 * sizeof(int32)>>::Of({ 1, 2, 3 });

    ASSERT_EQ(    Sum(ring.Enumerate()), 6);
    ASSERT_EQ(Average(ring.Enumerate()), 2);
    ASSERT_EQ(    Min(ring.Enumerate()), 1);
    ASSERT_EQ(    Max(ring.Enumerate()), 3);
}


TEST(StasticsTests, Rss)
{
    const auto set1 = Array<float>::Of({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f });
    const auto set2 = Array<float>::Of({ 1.0f, 2.0f, 3.0f, 4.0f, 6.0f });

    const auto rss = Statistics::Rss(
        set1.Enumerate(),
        set2.Enumerate()
    );

    EXPECT_FLOAT_EQ(rss, 1.0f);
}
