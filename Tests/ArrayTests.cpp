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
    GTEST_ASSERT_GE  (array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (array.Capacity(), MinReservedCapacity * 2);

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
    GTEST_ASSERT_GE  (array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (array.Capacity(), MinReservedCapacity * 2);

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
    GTEST_ASSERT_GE  (array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (array.Capacity(), MinReservedCapacity * 2);

    array.Reset();

    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Array_Capacity, ShrinkToFit_Free)
{
    using Item = int32;
    Array<Item> array;

    array.Add(69);

    GTEST_ASSERT_TRUE (array.IsAllocated());
    GTEST_ASSERT_FALSE(array.IsEmpty());
    GTEST_ASSERT_EQ   (array.Capacity(), ARRAY_DEFAULT_CAPACITY);
    GTEST_ASSERT_EQ   (array.Count(), 1);

    array.RemoveAt(0);

    GTEST_ASSERT_EQ  (array.Count(), 0);
    GTEST_ASSERT_TRUE(array.IsEmpty());
    GTEST_ASSERT_TRUE(array.IsAllocated());

    array.ShrinkToFit();

    GTEST_ASSERT_TRUE (array.IsEmpty());
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
    GTEST_ASSERT_EQ  (array.Count(),    TestCapacity1);
    GTEST_ASSERT_GE  (array.Capacity(), TestCapacity1);

    while (array.Count() > TestCapacity2)
        array.RemoveAt(0);

    array.ShrinkToFit();

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_EQ  (array.Count(),    TestCapacity2);
    GTEST_ASSERT_GE  (array.Capacity(), TestCapacity2);
    GTEST_ASSERT_LE  (array.Capacity(), TestCapacity1 / 2);
}


// Element Lifecycle Management

TEST(Array_ElementLifecycle, Add)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        array.Add(TestTracker{ 69 });
        GTEST_ASSERT_EQ(array.Count(), 1);
        GTEST_ASSERT_EQ(array[0].Value, 69);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2) // Include temporary
}


TEST(Array_ElementLifecycle, Emplace)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        array.Emplace(69);
        GTEST_ASSERT_EQ(array.Count(), 1);
        GTEST_ASSERT_EQ(array[0].Value, 69);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}

TEST(Array_ElementLifecycle, RemoveAt)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        array.Emplace(69);
        array.RemoveAt(0);
        GTEST_ASSERT_EQ(array.Count(), 0);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}


// Element Relocation

TEST(Array_ElementLifecycle, Reserve)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array{ ElementCount };
        GTEST_ASSERT_GE(array.Capacity(), ElementCount);

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            array.Emplace(i);

        // Relocation: n + n = 2n constructions
        array.Reserve(ElementCount * 3);
        // Note: This reservation forces reallocation. Element are obligated to be moved.

        // 2n Init: 2n + n = 3n constructions
        for (int32 i = ElementCount; i < ElementCount * 2; ++i)
            array.Emplace(i);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Array_ElementLifecycle, ShrinkToFit)
{
    constexpr int32 InitCapacity = 128;
    constexpr int32 ElementCount = 12;
    GTEST_ASSERT_GT(InitCapacity, ElementCount * 2);

    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array{ InitCapacity };

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            array.Emplace(i);

        GTEST_ASSERT_GE(array.Capacity(), InitCapacity);
        GTEST_ASSERT_EQ(array.Count(),    ElementCount);

        // Shrink: n + n = 2n constructions
        array.ShrinkToFit();
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}

TEST(Array_ElementLifecycle, MoveConstruct_NoDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<sizeof(TestTracker) * 32>;

        Array<TestTracker, NoDragAlloc> movedArray;

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        // Move Construct (no-drag alloc): n + n = 2n constructions
        Array<TestTracker, NoDragAlloc> targetArray{ MOVE(movedArray) };

        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}

TEST(Array_ElementLifecycle, MoveAssignment_NoDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<sizeof(TestTracker) * 32>;

        Array<TestTracker, NoDragAlloc> movedArray;

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        Array<TestTracker, NoDragAlloc> targetArray;
        // Dummy init: n + 1 = (n + 1) constructions
        targetArray.Add(69);

        // Move Assignment (no-drag alloc): (n + 1) + n = 2n + 1 constructions
        targetArray = MOVE(movedArray);

        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount + 1)
}

TEST(Array_ElementLifecycle, MoveConstruct_DragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = HeapAlloc;

        Array<TestTracker, NoDragAlloc> movedArray{ ElementCount };

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        // Move Construct (drag alloc): n + 0 = n constructions
        Array<TestTracker, NoDragAlloc> targetArray{ MOVE(movedArray) };

        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1 * ElementCount)
}

TEST(Array_ElementLifecycle, MoveAssignment_DragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = HeapAlloc;

        Array<TestTracker, NoDragAlloc> movedArray{ ElementCount };

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        Array<TestTracker, NoDragAlloc> targetArray;
        // Dummy init: n + 1 = (n + 1) constructions
        targetArray.Add(69);

        // Move Assignment (no-drag alloc): n + 1 = n + 1 constructions
        targetArray = MOVE(movedArray);

        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1 * ElementCount + 1)
}


// Element Access

TEST(Array_ElementAccess, Index)
{
    constexpr int32 ElementCount = 12;
    Array<int32> array;
    for (int32 i = 0; i < ElementCount; ++i)
        array.Add(i);
    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(array[i], i);
}

TEST(Array_ElementAccess, ConstIndex)
{
    constexpr int32 ElementCount = 12;

    const Array<int32> array = [&ElementCount]() -> Array<int32>
    {
        Array<int32> temp;
        for (int32 i = 0; i < ElementCount; ++i)
            temp.Add(i);
        return temp;
    }();

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(array[i], i);
}


// Element Manipulation

