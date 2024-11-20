// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Array.h"
#include "Algorithms/Querying.h"
#include "Allocators/FixedAlloc.h"
#include "Collections/Ring.h"

TEST(QueryingTests, Array)
{
    using namespace Querying;

    Array<int32, FixedAlloc<4 * sizeof(int32)>> array;
    array.Add(1);
    array.Add(2);
    array.Add(3);

    ASSERT_EQ(Count   (array.Enumerate()), 3);
    ASSERT_EQ(Sum     (array.Enumerate()), 6);
    ASSERT_EQ(Average (array.Enumerate()), 2);
    ASSERT_EQ(Min     (array.Enumerate()), 1);
    ASSERT_EQ(Max     (array.Enumerate()), 3);
}

TEST(QueryingTests, Ring)
{
    using namespace Querying;

    Ring<int32, FixedAlloc<4 * sizeof(int32)>> ring;
    ring.PushBack(1);
    ring.PushBack(2);
    ring.PushBack(3);

    ASSERT_EQ(Count   (ring.Enumerate()), 3);
    ASSERT_EQ(Sum     (ring.Enumerate()), 6);
    ASSERT_EQ(Average (ring.Enumerate()), 2);
    ASSERT_EQ(Min     (ring.Enumerate()), 1);
    ASSERT_EQ(Max     (ring.Enumerate()), 3);
}
