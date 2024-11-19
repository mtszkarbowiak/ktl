// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Debugging/LifecycleTracker.h"
#include "Collections/Array.h"


// Capacity Management

TEST(Array_Capacity, Reserve_OnInit)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Array<int32> array{ MinReservedCapacity };
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Array_Capacity, Reserve_OnRequest)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Array<int32> array;

    array.Reserve(MinReservedCapacity);
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Array_Capacity, Reserve_OnAdd)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Array<int32> array;
    for (int32 i = 0; i < MinReservedCapacity; ++i)
        array.Add(i);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Array_Capacity, ShrinkToFit_Free)
{
    using Item = int32;
    Array<Item> array;

    array.Add(69);
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_FALSE(array.IsEmpty());
    GTEST_ASSERT_EQ(array.Capacity(), ARRAY_DEFAULT_CAPACITY);
    GTEST_ASSERT_EQ(array.Count(), 1);

    array.RemoveAt(0);
    GTEST_ASSERT_EQ(array.Count(), 0);
    GTEST_ASSERT_TRUE(array.IsEmpty());
    GTEST_ASSERT_TRUE(array.IsAllocated());

    array.ShrinkToFit();
    GTEST_ASSERT_TRUE(array.IsEmpty());
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Array_Capacity, ShrinkToFit_Reloc)
{
    constexpr int32 TestCapacity1 = 256;
    constexpr int32 TestCapacity2 = 3;

    Array<int32> array;
    for (int32 i = 0; i < TestCapacity1; ++i)
        array.Add(i);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), TestCapacity1);
    GTEST_ASSERT_EQ(array.Count(),    TestCapacity1);

    while (array.Count() > TestCapacity2)
        array.RemoveAt(0);

    array.ShrinkToFit();
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), TestCapacity2);
    GTEST_ASSERT_EQ(array.Count(),    TestCapacity2);

    GTEST_ASSERT_LE(array.Capacity(), TestCapacity1 / 2);
}


// Element Lifecycle Management

TEST(Array_ElementLifecycle, Add)
{
    LIFECYCLE_TEST_INTO
    {
        Array<LifecycleTracker> array;
        array.Add(LifecycleTracker{ 69 });
        GTEST_ASSERT_EQ(array.Count(), 1);
        GTEST_ASSERT_EQ(array[0].Value, 69);
        array.Reset();
        GTEST_ASSERT_EQ(array.Count(), 0);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2) // Include temporary
}


TEST(Array_ElementLifecycle, Emplace)
{
    LIFECYCLE_TEST_INTO
    {
        Array<LifecycleTracker> array;
        array.Emplace(69);
        GTEST_ASSERT_EQ(array.Count(), 1);
        GTEST_ASSERT_EQ(array[0].Value, 69);
        array.Reset();
        GTEST_ASSERT_EQ(array.Count(), 0);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}
