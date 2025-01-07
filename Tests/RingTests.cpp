// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Debugging/LifecycleTracker.h"
#include "Collections/Ring.h"


// Capacity Management

TEST(Ring_Capacity, Reserve_Call)
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

TEST(Ring_Capacity, Reserve_Ctor)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    Ring<int32> ring{ MinReservedCapacity };

    GTEST_ASSERT_TRUE(ring.IsAllocated());
    GTEST_ASSERT_GE(ring.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(ring.Capacity(), MinReservedCapacity * 2);

    ring.Reset();

    GTEST_ASSERT_FALSE(ring.IsAllocated());
}

TEST(Ring_Capacity, Reserve_Add)
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

TEST(Ring_Capacity, Compact_Free)
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

    array.Compact();

    GTEST_ASSERT_TRUE (array.IsEmpty());
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Ring_Capacity, Compact_Reloc)
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

    array.Compact();

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

TEST(Ring_ElementLifecycle, EmplaceBack)
{
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        ring.EmplaceBack(69);
        GTEST_ASSERT_EQ(ring.Count(), 1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1) // No temporary
}

TEST(Ring_ElementLifecycle, EmplaceFront)
{
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        ring.EmplaceFront(69);
        GTEST_ASSERT_EQ(ring.Count(), 1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1) // No temporary
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

TEST(Ring_ElementAccess, Index)
{
    constexpr int32 ElementCount = 12;
    Ring<int32> ring;
    for (int32 i = 0; i < ElementCount; ++i)
        ring.PushBack(i);
    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(ring[i], i);
}

TEST(Ring_ElementAccess, ConstIndex)
{
    constexpr int32 ElementCount = 12;
    const Ring<int32> ring = [&ElementCount]() -> Ring<int32>
    {
        Ring<int32> temp;
        for (int32 i = 0; i < ElementCount; ++i)
            temp.PushBack(i);
        return temp;
    }();
    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(ring[i], i);
}


// Element Relocation

TEST(Ring_Relocation, Reserve)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring{ ElementCount };
        const int32 initCapacity = ring.Capacity();

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushBack(TestTracker{ i });

        // Reloc: n constructions
        ring.Reserve(RING_DEFAULT_CAPACITY * 2);

        const int32 newCapacity = ring.Capacity();
        GTEST_ASSERT_GT(newCapacity, initCapacity); // Relocation must occur

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushBack(TestTracker{ i });

        // Total: 5n constructions
    }

    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(5 * ElementCount)
}

TEST(Ring_Relocation, Compact)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring{ RING_DEFAULT_CAPACITY * 2 };
        const int32 initCapacity = ring.Capacity();

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushBack(TestTracker{ i });

        // Reloc: n constructions
        ring.Compact();

        const int32 newCapacity = ring.Capacity();
        GTEST_ASSERT_LT(newCapacity, initCapacity); // Relocation must occur
        // Total: 3n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Ring_Relocation, MoveCtor_NoDragAlloc)
{
    const int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<sizeof(TestTracker) * 32>;

        Ring<TestTracker, NoDragAlloc> movedRing;

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            movedRing.PushBack(TestTracker{ i });

        // Reloc: n constructions
        Ring<TestTracker, NoDragAlloc> targetRing{ MOVE(movedRing) };
        GTEST_ASSERT_EQ(targetRing.Count(), ElementCount);

        // Total: 3n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Ring_Relocation, MoveAsgn_NoDragAlloc)
{
    constexpr int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using NoDragAlloc = FixedAlloc<sizeof(TestTracker) * 32>;

        Ring<TestTracker, NoDragAlloc> movedRing;

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            movedRing.PushBack(TestTracker{ i });

        Ring<TestTracker, NoDragAlloc> targetRing;

        // Init: 1 construction (1 for the ring)
        targetRing.PushBack(TestTracker{ 69 });// To be overriden

        // Reloc: n constructions
        targetRing = MOVE(movedRing);
        GTEST_ASSERT_EQ(targetRing.Count(), ElementCount);

        // Total: 3n + 2 constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount + 2)
}

TEST(Ring_Relocation, MoveCtor_DragAlloc)
{
    const int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using DragAlloc = HeapAlloc;

        Ring<TestTracker, DragAlloc> movedRing{ ElementCount };

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            movedRing.PushBack(TestTracker{ i });

        // Reloc: 0 constructions
        Ring<TestTracker, DragAlloc> targetRing{ MOVE(movedRing) };
        GTEST_ASSERT_EQ(targetRing.Count(), ElementCount);

        // Total: 2n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount)
}

TEST(Ring_Relocation, MoveAsgn_DragAlloc)
{
    const int32 ElementCount = 12;

    LIFECYCLE_TEST_INTO
    {
        using DragAlloc = HeapAlloc;

        Ring<TestTracker, DragAlloc> movedRing{ ElementCount };

        // Init: 2n constructions (1 for the ring, 1 for the temporary)
        for (int32 i = 0; i < ElementCount; ++i)
            movedRing.PushBack(TestTracker{ i });

        Ring<TestTracker, DragAlloc> targetRing;

        // Init: 2 construction
        targetRing.PushBack(TestTracker{ 69 }); // To be overriden

        // Reloc: 0 constructions
        targetRing = MOVE(movedRing);
        GTEST_ASSERT_EQ(targetRing.Count(), ElementCount);

        // Total: 2n + 2 constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2 * ElementCount + 2)
}


// Element Copying

TEST(Ring_Copying, CopyCtor)
{
    constexpr int32 ElementCount = 12;
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> movedRing{ ElementCount };
        for (int32 i = 0; i < ElementCount; ++i)
            movedRing.PushBack(TestTracker{ i }); // 2 constructions: 1 for the ring, 1 for the temporary

        Ring<TestTracker> targetRing{ movedRing }; // 1 construction: 1 for the copy
        GTEST_ASSERT_EQ(targetRing.Count(), ElementCount);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Ring_Copying, CopyAsgn)
{
    constexpr int32 ElementCount = 12;
    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> movedRing{ ElementCount };
        for (int32 i = 0; i < ElementCount; ++i)
            movedRing.PushBack(TestTracker{ i }); // 2 constructions: 1 for the ring, 1 for the temporary

        Ring<TestTracker> targetRing = Ring<TestTracker>{}; // Explicit init, so IDE doesn't complain.
        targetRing = movedRing; // 1 construction: 1 for the copy

        GTEST_ASSERT_EQ(targetRing.Count(), ElementCount);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount) // Include temporary
}


// Element Manipulation

TEST(Ring_ElementManipulation, PushBack_PopBack)
{
    const int32 Cycles = 3;
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        for (auto cycle = 0; cycle < Cycles; ++cycle)
        {
            // Add n elements
            for (int32 i = 0; i < ElementCount; ++i)
                ring.PushBack(TestTracker{ i });
            GTEST_ASSERT_EQ(ring.Count(), ElementCount);
            GTEST_ASSERT_FALSE(ring.IsEmpty());

            // Remove n elements
            for (int32 i = 0; i < ElementCount; ++i)
                ring.PopBack();
            GTEST_ASSERT_EQ(ring.Count(), 0);
            GTEST_ASSERT_TRUE(ring.IsEmpty());
        }
    }
    LIFECYCLE_TEST_OUT
}

TEST(Ring_ElementManipulation, PushBack_PopFront)
{
    const int32 Cycles = 3;
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        for (auto cycle = 0; cycle < Cycles; ++cycle)
        {
            for (int32 i = 0; i < ElementCount; ++i)
                ring.PushBack(TestTracker{ i });
            GTEST_ASSERT_EQ(ring.Count(), ElementCount);

            for (int32 i = 0; i < ElementCount; ++i)
                ring.PopFront();
            GTEST_ASSERT_EQ(ring.Count(), 0);
            GTEST_ASSERT_TRUE(ring.IsEmpty());
        }
    }
    LIFECYCLE_TEST_OUT
}

TEST(Ring_ElementManipulation, PushFront_PopBack)
{
    const int32 Cycles = 3;
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        for (auto cycle = 0; cycle < Cycles; ++cycle)
        {
            for (int32 i = 0; i < ElementCount; ++i)
                ring.PushFront(TestTracker{ i });
            GTEST_ASSERT_EQ(ring.Count(), ElementCount);

            for (int32 i = 0; i < ElementCount; ++i)
                ring.PopBack();
            GTEST_ASSERT_EQ(ring.Count(), 0);
            GTEST_ASSERT_TRUE(ring.IsEmpty());
        }
    }
    LIFECYCLE_TEST_OUT
}

TEST(Ring_ElementManipulation, PushFront_PopFront)
{
    const int32 Cycles = 3;
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        for (auto cycle = 0; cycle < Cycles; ++cycle)
        {
            for (int32 i = 0; i < ElementCount; ++i)
                ring.PushFront(TestTracker{ i });
            GTEST_ASSERT_EQ(ring.Count(), ElementCount);

            for (int32 i = 0; i < ElementCount; ++i)
                ring.PopFront();
            GTEST_ASSERT_EQ(ring.Count(), 0);
            GTEST_ASSERT_TRUE(ring.IsEmpty());
        }
    }
    LIFECYCLE_TEST_OUT
}


TEST(Ring_ElementManipulation, EmplaceBack_PopBack)
{
    const int32 Cycles = 3;
    const int32 ElementCount = 1000;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        for (auto cycle = 0; cycle < Cycles; ++cycle)
        {
            for (int32 i = 0; i < ElementCount; ++i)
                ring.EmplaceBack(i);
            GTEST_ASSERT_EQ(ring.Count(), ElementCount);

            for (int32 i = 0; i < ElementCount; ++i)
                ring.PopBack();
            GTEST_ASSERT_EQ(ring.Count(), 0);
            GTEST_ASSERT_TRUE(ring.IsEmpty());
        }
    }
    LIFECYCLE_TEST_OUT
}

TEST(Ring_ElementManipulation, EmplaceFront_PopFront)
{
    const int32 Cycles = 3;
    const int32 ElementCount = 1000;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;
        for (auto cycle = 0; cycle < Cycles; ++cycle)
        {
            for (int32 i = 0; i < ElementCount; ++i)
                ring.EmplaceFront(i);
            GTEST_ASSERT_EQ(ring.Count(), ElementCount);

            for (int32 i = 0; i < ElementCount; ++i)
                ring.PopFront();
            GTEST_ASSERT_EQ(ring.Count(), 0);
            GTEST_ASSERT_TRUE(ring.IsEmpty());
        }
    }
    LIFECYCLE_TEST_OUT
}
