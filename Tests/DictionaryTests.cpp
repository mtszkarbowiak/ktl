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

TEST(Dictionary_Capacity, Reserve_OnInit)
{
    constexpr int32 ExampleDefaultCapacity = 10;
    Dictionary<int32, int32> dict{ ExampleDefaultCapacity };
    GTEST_ASSERT_GE(HASH_SETS_DEFAULT_CAPACITY, ExampleDefaultCapacity);
    GTEST_ASSERT_EQ(dict.Capacity(), HASH_SETS_DEFAULT_CAPACITY);
}

TEST(Dictionary_Capacity, Reserve_Add)
{
    Dictionary<int32, int32> dict;
    dict.Add(1, 2);
    dict.Add(3, 4);

    GTEST_ASSERT_EQ(dict.Count(), 2);
}

TEST(Dictionary_Capacity, AddMany)
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

TEST(Dictionary_Capacity, AddManyRemoveMany)
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

TEST(Dictionary_Capacity, AddAndGet)
{
    Dictionary<int32, int32> dict;
    dict.Add(1, 2);
    dict.Add(3, 4);

    const int32 value1 = *dict.TryGet(1);
    const int32 value2 = *dict.TryGet(3);

    GTEST_ASSERT_EQ(value1, 2);
    GTEST_ASSERT_EQ(value2, 4);
}

TEST(Dictionary_Relocation, AddManyAndCompact)
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

TEST(Dictionary_Relocation, MoveConstruct_NoDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<256>;
        
        Dictionary<Index, TestTracker, NoDragAlloc> movedDict;
        // Note: Here use index, as index allows for predictable stack alloc, due to no sentinels.

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedDict.Add(i, TestTracker(i)); // 2n: 1 for temporary, 1 for slot

        // Reloc: n constructions
        Dictionary<Index, TestTracker, NoDragAlloc> targetDict{ MOVE(movedDict) }; // 1n: 1 for move
        GTEST_ASSERT_EQ(movedDict.Count(), 0);
        GTEST_ASSERT_EQ(targetDict.Count(), ElementCount);

        // Total: 2n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Dictionary_Relocation, MoveConstruct_DragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using DragAlloc = HeapAlloc;

        Dictionary<int32, TestTracker, DragAlloc> movedDict;

        // Init: n constructions
        for (int32 i = 0; i < ElementCount; ++i)
            movedDict.Add(i, TestTracker(i));

        // Reloc: 0 constructions
        //Dictionary<int32, TestTracker, DragAlloc> targetDict{ MOVE(movedDict) };
        //GTEST_ASSERT_EQ(movedDict.Count(), 0);
        //GTEST_ASSERT_EQ(targetDict.Count(), ElementCount);

        // Total: n constructions
    }
    LIFECYCLE_TEST_OUT
    // LIFECYCLE_TEST_DIFF(1 * ElementCount)
}
