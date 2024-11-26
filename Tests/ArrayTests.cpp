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
    array.EnsureCapacity(MinReservedCapacity);

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

    array.Compact();

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

    array.Compact();

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
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}

TEST(Array_ElementLifecycle, InsertAt)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        array.InsertAt(0, 69);
        GTEST_ASSERT_EQ(array.Count(), 1);
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


// Element Relocation

TEST(Array_Relocation, Reserve)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array{ ElementCount };
        GTEST_ASSERT_GE(array.Capacity(), ElementCount);

        // Init: n constructions (emplace, no temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            array.Emplace(i);

        // Reloc: n constructions
        array.EnsureCapacity(ElementCount * 3);
        // Note: This reservation forces reallocation. Element are obligated to be moved.

        // Init: n constructions (emplace, no temporary)
        for (int32 i = ElementCount; i < ElementCount * 2; ++i)
            array.Emplace(i);

        // Total: 3n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Array_Relocation, ShrinkToFit)
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

        // Reloc: n constructions
        array.Compact();

        // Total: 2n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}

TEST(Array_Relocation, MoveConstruct_NoDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<sizeof(TestTracker) * 32>;

        Array<TestTracker, NoDragAlloc> movedArray;

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        // Reloc: n constructions
        Array<TestTracker, NoDragAlloc> targetArray{ MOVE(movedArray) };
        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);

        // Total: 2n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}

TEST(Array_Relocation, MoveAssignment_NoDragAlloc)
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

        // Init: 1 construction
        targetArray.Emplace(69); // To be overriden

        // Reloc: n constructions
        targetArray = MOVE(movedArray);
        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);

        // Total: 2n + 1 constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount + 1)
}

TEST(Array_Relocation, MoveConstruct_DragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = HeapAlloc;

        Array<TestTracker, NoDragAlloc> movedArray{ ElementCount };

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        // Reloc: 0 constructions
        Array<TestTracker, NoDragAlloc> targetArray{ MOVE(movedArray) };
        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);

        // Total: n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1 * ElementCount)
}

TEST(Array_Relocation, MoveAssignment_DragAlloc)
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

        // Init: 1 construction
        targetArray.Emplace(69); // To be overriden

        // Reloc: 0 constructions
        targetArray = MOVE(movedArray);
        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);

        // Total: n + 1 constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1 * ElementCount + 1)
}


// Element Manipulation

TEST(Array_ElementManipulation, Insert)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        for (int32 i = 0; i < 6; ++i)
            array.Emplace(i);

        array.InsertAt(2, 69);

        GTEST_ASSERT_EQ(array.Count(), 7);

        GTEST_ASSERT_EQ(array[0].Value, 0);
        GTEST_ASSERT_EQ(array[1].Value, 1);
        GTEST_ASSERT_EQ(array[2].Value, 69);
        GTEST_ASSERT_EQ(array[3].Value, 3);
        GTEST_ASSERT_EQ(array[4].Value, 4);
        GTEST_ASSERT_EQ(array[5].Value, 5);
        GTEST_ASSERT_EQ(array[6].Value, 2);
    }
    LIFECYCLE_TEST_OUT
}

TEST(Array_ElementManipulation, InsertAt)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        for (int32 i = 0; i < 6; ++i)
            array.Emplace(i);

        array.InsertAtStable(2, 69);

        GTEST_ASSERT_EQ(array.Count(), 7);

        GTEST_ASSERT_EQ(array[0].Value, 0);
        GTEST_ASSERT_EQ(array[1].Value, 1);
        GTEST_ASSERT_EQ(array[2].Value, 69);
        GTEST_ASSERT_EQ(array[3].Value, 2);
        GTEST_ASSERT_EQ(array[4].Value, 3);
        GTEST_ASSERT_EQ(array[5].Value, 4);
        GTEST_ASSERT_EQ(array[6].Value, 5);
    }
    LIFECYCLE_TEST_OUT
}

TEST(Array_ElementManipulation, Remove)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        for (int32 i = 0; i < 6; ++i)
            array.Emplace(i);

        array.RemoveAt(2);

        GTEST_ASSERT_EQ(array.Count(), 5);

        GTEST_ASSERT_EQ(array[0].Value, 0);
        GTEST_ASSERT_EQ(array[1].Value, 1);
        GTEST_ASSERT_EQ(array[2].Value, 5);
        GTEST_ASSERT_EQ(array[3].Value, 3);
        GTEST_ASSERT_EQ(array[4].Value, 4);

    }
    LIFECYCLE_TEST_OUT
}

TEST(Array_ElementManipulation, RemoveStable)
{
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> array;
        for (int32 i = 0; i < 6; ++i)
            array.Emplace(i);

        array.RemoveAtStable(2);

        GTEST_ASSERT_EQ(array.Count(), 5);

        GTEST_ASSERT_EQ(array[0].Value, 0);
        GTEST_ASSERT_EQ(array[1].Value, 1);
        GTEST_ASSERT_EQ(array[2].Value, 3);
        GTEST_ASSERT_EQ(array[3].Value, 4);
        GTEST_ASSERT_EQ(array[4].Value, 5);
    }
    LIFECYCLE_TEST_OUT
}
