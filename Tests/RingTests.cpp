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

TEST(Ring_Relocation, ShrinkToFit)
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
        ring.ShrinkToFit();

        const int32 newCapacity = ring.Capacity();
        GTEST_ASSERT_LT(newCapacity, initCapacity); // Relocation must occur
        // Total: 3n constructions
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(3 * ElementCount)
}

TEST(Ring_Relocation, MoveConstruct_NoDragAlloc)
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

TEST(Ring_Relocation, MoveAssignment_NoDragAlloc)
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

TEST(Ring_Relocation, MoveConstruct_DragAlloc)
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

TEST(Ring_Relocation, MoveAssignment_DragAlloc)
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


// Element Manipulation

TEST(Array_ElementManipulation, PushBack_PopBack)
{
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushBack(TestTracker{ i });
        GTEST_ASSERT_EQ(ring.Count(), ElementCount);

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PopBack();
        GTEST_ASSERT_EQ(ring.IsEmpty(), 0);
    }
    LIFECYCLE_TEST_OUT
}

TEST(Array_ElementManipulation, PushBack_PopFront)
{
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushBack(TestTracker{ i });
        GTEST_ASSERT_EQ(ring.Count(), ElementCount);

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PopFront();
        GTEST_ASSERT_EQ(ring.IsEmpty(), 0);
    }
    LIFECYCLE_TEST_OUT
}

TEST(Array_ElementManipulation, PushFront_PopBack)
{
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushFront(TestTracker{ i });
        GTEST_ASSERT_EQ(ring.Count(), ElementCount);

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PopBack();
        GTEST_ASSERT_EQ(ring.IsEmpty(), 0);
    }
    LIFECYCLE_TEST_OUT
}

TEST(Array_ElementManipulation, PushFront_PopFront)
{
    const int32 ElementCount = 100;

    LIFECYCLE_TEST_INTO
    {
        Ring<TestTracker> ring;

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PushFront(TestTracker{ i });
        GTEST_ASSERT_EQ(ring.Count(), ElementCount);

        for (int32 i = 0; i < ElementCount; ++i)
            ring.PopFront();
        GTEST_ASSERT_EQ(ring.IsEmpty(), 0);
    }
    LIFECYCLE_TEST_OUT
}

