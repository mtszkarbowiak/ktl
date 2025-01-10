// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/Dictionary.h"
#include "Debugging/LifecycleTracker.h"

// Capacity Management

TEST(DictionaryCapacity, ReserveOnCall)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, HASH_SETS_DEFAULT_CAPACITY);

    Dictionary<int32, int32> dict;
    dict.ReserveSlots(MinReservedCapacity);

    GTEST_ASSERT_TRUE(dict.IsAllocated());
    GTEST_ASSERT_GE(dict.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(dict.Capacity(), MinReservedCapacity * 2);

    dict.Reset();
    GTEST_ASSERT_FALSE(dict.IsAllocated());
}

TEST(DictionaryCapacity, ReserveOnCtor)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, HASH_SETS_DEFAULT_CAPACITY);

    Dictionary<int32, int32> dict{ MinReservedCapacity };

    GTEST_ASSERT_TRUE(dict.IsAllocated());
    GTEST_ASSERT_GE(dict.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(dict.Capacity(), MinReservedCapacity * 2);

    dict.Reset();
    GTEST_ASSERT_FALSE(dict.IsAllocated());

}

TEST(DictionaryCapacity, ReserveOnAdd)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, HASH_SETS_DEFAULT_CAPACITY);

    Dictionary<int32, int32> dict;
    for (int32 i = 0; i < MinReservedCapacity; ++i)
        dict.Add(i, i);
    

    GTEST_ASSERT_TRUE(dict.IsAllocated());
    GTEST_ASSERT_GE  (dict.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (dict.Capacity(), MinReservedCapacity * 2);

    dict.Reset();

    GTEST_ASSERT_FALSE(dict.IsAllocated());
}

TEST(DictionaryCapacity, FreeOnCompact)
{
    Dictionary<int32, int32> dict;
    dict.Add(69, 69);

    GTEST_ASSERT_TRUE (dict.IsAllocated());
    GTEST_ASSERT_FALSE(dict.IsEmpty());
    GTEST_ASSERT_EQ   (dict.Capacity(), HASH_SETS_DEFAULT_CAPACITY);
    GTEST_ASSERT_EQ   (dict.Count(), 1);

    dict.Remove(69);

    GTEST_ASSERT_EQ(dict.Count(), 0);
    GTEST_ASSERT_TRUE(dict.IsEmpty());
    GTEST_ASSERT_TRUE(dict.IsAllocated());

    dict.Compact();

    GTEST_ASSERT_TRUE(dict.IsEmpty());
    GTEST_ASSERT_FALSE(dict.IsAllocated());
}

TEST(DictionaryCapacity, CompactOnReloc)
{
    constexpr int32 TestCapacity1 = 256;
    constexpr int32 TestCapacity2 = 3;

    Dictionary<int32, int32> dict;
    for (int32 i = 0; i < TestCapacity1; ++i)
        dict.Add(i, i);
    

    GTEST_ASSERT_TRUE(dict.IsAllocated());
    GTEST_ASSERT_EQ  (dict.Count(), TestCapacity1);
    GTEST_ASSERT_GE  (dict.Capacity(), TestCapacity1);

    for (int32 i = TestCapacity2; i < TestCapacity1; ++i)
        dict.Remove(i);

    dict.Compact();

    GTEST_ASSERT_TRUE(dict.IsAllocated());
    GTEST_ASSERT_EQ  (dict.Count(),    TestCapacity2);
    GTEST_ASSERT_GE  (dict.Capacity(), TestCapacity2);
    GTEST_ASSERT_LE  (dict.Capacity(), TestCapacity1 / 2);
}


TEST(DictionaryCapacity, AddMany)
{
    Dictionary<int32, int32> dict;

    for (int32 i = 0; i < 1000; ++i)
    {
        dict.Add(i, i);
    }

    GTEST_ASSERT_EQ(dict.Count(), 1000);

    for (int32 i = 0; i < 1000; ++i)
    {
        GTEST_ASSERT_TRUE(dict.Contains(i));
        GTEST_ASSERT_EQ(*dict.TryGet(i), i);
    }
}

TEST(DictionaryCapacity, AddManyRemoveMany)
{
    Dictionary<int32, int32> dict;

    for (int32 i = 0; i < 1000; ++i)
    {
        dict.Add(i, i);
    }

    GTEST_ASSERT_EQ(dict.Count(), 1000);

    for (int32 i = 0; i < 1000; ++i)
    {
        GTEST_ASSERT_TRUE(dict.Contains(i));
        GTEST_ASSERT_EQ(*dict.TryGet(i), i);
    }

    for (int32 i = 0; i < 1000; ++i)
    {
        if (i % 200 == 0) 
        {
            dict.Compact();
        }

        dict.Remove(i);
    }
}

TEST(DictionaryCapacity, AddAndGet)
{
    Dictionary<int32, int32> dict;
    dict.Add(1, 2);
    dict.Add(3, 4);

    const int32 value1 = *dict.TryGet(1);
    const int32 value2 = *dict.TryGet(3);

    GTEST_ASSERT_EQ(value1, 2);
    GTEST_ASSERT_EQ(value2, 4);
}

TEST(DictionaryRelocation, AddManyAndCompact)
{
    using Alloc = HeapAlloc; // Only dragging alloc makes sense here
    constexpr int32 ElementCount = 1000;
    static_assert(Alloc::MinCapacity < ElementCount, "Test requires more elements than the minimum capacity.");

    // Feed with n elements
    Dictionary<int32, int32> dict;
    for (int32 i = 0; i < ElementCount; ++i)
    {
        dict.Add(i, i);
    }

    GTEST_ASSERT_EQ(dict.Count(), ElementCount);
    for (int32 i = 0; i < ElementCount; ++i)
    {
        GTEST_ASSERT_TRUE(dict.Contains(i));
        GTEST_ASSERT_EQ(*dict.TryGet(i), i);
    }

    const int32 oldCapacity = dict.Capacity();

    // Remove first 3n/4 elements (must be less than half of the capacity)
    for (int32 i = 0; i < (ElementCount / 4 * 3); ++i)
    {
        dict.Remove(i);
    }

    dict.Compact();

    const int32 newCapacity = dict.Capacity();
    GTEST_ASSERT_LT(newCapacity, oldCapacity); // Capacity must be reduced
}


// Element Relocation

TEST(DictionaryRelocation, MoveConstructNoDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<256>;

        Dictionary<Index, TestTracker, NoDragAlloc> movedDict;

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedDict.Add(i, TestTracker(i)); // 2n: 1 for temporary, 1 for slot

        // Reloc: n constructions
        Dictionary<Index, TestTracker, NoDragAlloc> targetDict{ MOVE(movedDict) }; // 1n: 1 for move
        GTEST_ASSERT_EQ(movedDict.Count(), 0);
        GTEST_ASSERT_EQ(targetDict.Count(), ElementCount);

        // Total: 3n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(DictionaryRelocation, MoveConstructDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using DragAlloc = HeapAlloc;

        Dictionary<Index, TestTracker, DragAlloc> movedDict;

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedDict.Add(i, TestTracker(i)); // 2n: 1 for temporary, 1 for slot

        // Reloc: 0 constructions
        Dictionary<Index, TestTracker, DragAlloc> targetDict{ MOVE(movedDict) }; // 0 constructions
        GTEST_ASSERT_EQ(movedDict.Count(), 0);
        GTEST_ASSERT_EQ(targetDict.Count(), ElementCount);

        // Total: n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}


// Element Manipulation
