// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Debugging/LifecycleTracker.h"
#include "Collections/Ring.h"


// Capacity Management

TEST(Ring_Capacity, Reserve_OnInit)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Ring<int32> ring{ MinReservedCapacity };

    GTEST_ASSERT_TRUE(ring.IsAllocated());
    GTEST_ASSERT_GE  (ring.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (ring.Capacity(), MinReservedCapacity * 2);

    ring.Reset();

    GTEST_ASSERT_FALSE(ring.IsAllocated());
}

TEST(Ring_Capacity, Reserve_OnRequest)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Ring<int32> ring;
    ring.Reserve(MinReservedCapacity);

    GTEST_ASSERT_TRUE(ring.IsAllocated());
    GTEST_ASSERT_GE  (ring.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (ring.Capacity(), MinReservedCapacity * 2);

    ring.Reset();

    GTEST_ASSERT_FALSE(ring.IsAllocated());
}

TEST(Ring_Capacity, Reserve_OnAdd)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Ring<int32> ring;
    for (int32 i = 0; i < MinReservedCapacity; ++i)
        ring.PushBack(i);

    GTEST_ASSERT_TRUE(ring.IsAllocated());
    GTEST_ASSERT_GE  (ring.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (ring.Capacity(), MinReservedCapacity * 2);

    ring.Reset();

    GTEST_ASSERT_FALSE(ring.IsAllocated());
}

TEST(Ring_Capacity, ShrinkToFit_Free)
{
    using Item = int32;
    Ring<Item> array;

    array.PushBack(69);

    GTEST_ASSERT_TRUE (array.IsAllocated());
    GTEST_ASSERT_FALSE(array.IsEmpty());
    GTEST_ASSERT_EQ   (array.Capacity(), RING_DEFAULT_CAPACITY);
    GTEST_ASSERT_EQ   (array.Count(), 1);

    array.PopBack();

    GTEST_ASSERT_EQ  (array.Count(), 0);
    GTEST_ASSERT_TRUE(array.IsEmpty());
    GTEST_ASSERT_TRUE(array.IsAllocated());

    array.ShrinkToFit();

    GTEST_ASSERT_TRUE (array.IsEmpty());
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Ring_Capacity, ShrinkToFit_Reloc)
{
    constexpr int32 TestCapacity1 = 256;
    constexpr int32 TestCapacity2 = 3;

    Ring<int32> array;
    for (int32 i = 0; i < TestCapacity1; ++i)
        array.PushBack(i);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_EQ  (array.Count(),    TestCapacity1);
    GTEST_ASSERT_GE  (array.Capacity(), TestCapacity1);

    while (array.Count() > TestCapacity2)
        array.PopBack();

    array.ShrinkToFit();

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_EQ  (array.Count(),    TestCapacity2);
    GTEST_ASSERT_GE  (array.Capacity(), TestCapacity2);
    GTEST_ASSERT_LE  (array.Capacity(), TestCapacity1 / 2);
}


// Element Lifecycle Management

TEST(Ring_ElementLifecycle, PushBack)
{
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        ring.PushBack(TestTracker{ 69 });
        GTEST_ASSERT_EQ(ring.Count(), 1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2) // Include temporary
}

TEST(Ring_ElementLifecycle, PushFront)
{
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        ring.PushFront(TestTracker{ 69 });
        GTEST_ASSERT_EQ(ring.Count(), 1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2) // Include temporary
}

TEST(Ring_ElementLifecycle, PopBack)
{
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        ring.PushBack(TestTracker{ 69 });
        ring.PopBack();
        GTEST_ASSERT_EQ(ring.Count(), 0);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2) // Include temporary
}

TEST(Ring_ElementLifecycle, PopFront)
{
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        ring.PushBack(TestTracker{ 69 });
        ring.PopFront();
        GTEST_ASSERT_EQ(ring.Count(), 0);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2) // Include temporary
}


// Element Access

//TODO

// Element Relocation

//TODO

// Element Manipulation

//TODO
