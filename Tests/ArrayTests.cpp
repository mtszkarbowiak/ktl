// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Debugging/LifecycleTracker.h"
#include "Collections/Array.h"
#include "Collections/StaticArray.h"


// Capacity Management

TEST(ArrayCapacity, ReserveOnCall)
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

TEST(ArrayCapacity, ReserveOnCtor)
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

TEST(ArrayCapacity, ReserveOnAdd)
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

TEST(ArrayCapacity, CompactOnFree)
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

TEST(ArrayCapacity, CompactOnReloc)
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

TEST(ArrayElementLifecycle, Add)
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

TEST(ArrayElementLifecycle, Emplace)
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

TEST(ArrayElementLifecycle, InsertAt)
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

TEST(ArrayElementLifecycle, RemoveAt)
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

TEST(ArrayElementAccess, Index)
{
    constexpr int32 ElementCount = 12;

    Array<int32> array;
    for (int32 i = 0; i < ElementCount; ++i)
        array.Add(i);

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(array[i], i);
}

TEST(ArrayElementAccess, ConstIndex)
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

TEST(ArrayRelocation, Reserve)
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
        array.Reserve(ElementCount * 3);
        // Note: This reservation forces reallocation. Element are obligated to be moved.

        // Init: n constructions (emplace, no temporary)
        for (int32 i = ElementCount; i < ElementCount * 2; ++i)
            array.Emplace(i);

        // Total: 3n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(ArrayRelocation, Compact)
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

TEST(ArrayRelocation, MoveCtorNoDragAlloc)
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
        GTEST_ASSERT_EQ(movedArray.Count(), 0);
        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);

        // Total: 2n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}

TEST(ArrayRelocation, MoveAsgnNoDragAlloc)
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

TEST(ArrayRelocation, MoveCtorDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using DragAlloc = HeapAlloc;

        Array<TestTracker, DragAlloc> movedArray{ ElementCount };

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        // Reloc: 0 constructions
        Array<TestTracker, DragAlloc> targetArray{ MOVE(movedArray) };
        GTEST_ASSERT_EQ(targetArray.Count(), ElementCount);

        // Total: n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1 * ElementCount)
}

TEST(ArrayRelocation, MoveAsgnDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using DragAlloc = HeapAlloc;

        Array<TestTracker, DragAlloc> movedArray{ ElementCount };

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedArray.Emplace(i);

        Array<TestTracker, DragAlloc> targetArray;

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


// Element Copying

TEST(ArrayCopying, CopyCtor)
{
    constexpr int32 ElementCount = 12;
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> arraySrc{ ElementCount }; // Reserve space so we don't reallocate
        for (int32 i = 0; i < ElementCount; ++i)
            arraySrc.Add(TestTracker{ i }); // 2 constructions: 1 for the array, 1 for the temporary

        Array<TestTracker> arrayDst{ arraySrc }; // 1 construction: 1 for the copy

        GTEST_ASSERT_EQ(arrayDst.Count(), arraySrc.Count());
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(ArrayCopying, CopyAsgn)
{
    constexpr int32 ElementCount = 12;
    LIFECYCLE_TEST_INTO
    {
        Array<TestTracker> arraySrc{ ElementCount }; // Reserve space so we don't reallocate
        for (int32 i = 0; i < ElementCount; ++i)
            arraySrc.Add(TestTracker{ i }); // 2 constructions: 1 for the array, 1 for the temporary

        Array<TestTracker> arrayDst = Array<TestTracker>{}; // Explicit init, so IDE doesn't complain.
        arrayDst = arraySrc; // 1 construction: 1 for the copy

        GTEST_ASSERT_EQ(arrayDst.Count(), arraySrc.Count());
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}


// Element Manipulation

TEST(ArrayElementManipulation, Insert)
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

TEST(ArrayElementManipulation, InsertAt)
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

TEST(ArrayElementManipulation, Remove)
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

TEST(ArrayElementManipulation, RemoveStable)
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


TEST(StaticArray, BasicAssignment)
{
    constexpr int32 ElementCount = 12;

    StaticArray<int32, ElementCount> array{};
    for (int32 i = 0; i < ElementCount; ++i)
        array[i] = i;
    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(array[i], i);
}

TEST(StaticArray, ValuesPuller)
{
    constexpr int32 ElementCount = 12;
    StaticArray<int32, ElementCount> array{};
    for (int32 i = 0; i < ElementCount; ++i)
        array[i] = i;

    int32 i = 0;
    for (auto puller = array.Values(); puller; ++puller)
    {
        GTEST_ASSERT_EQ(*puller, i);
        ++i;
    }
}
