// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/BitArray.h"


// Capacity Management

TEST(BitArrayCapacity, ReservingOnCall)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    BitArray<> array;
    array.Reserve(MinReservedCapacity);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();

    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(BitArrayCapacity, ReservingOnCtor)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    BitArray<> array{ MinReservedCapacity };

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();

    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(BitArrayCapacity, ReservingOnAdd)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    BitArray<> array;
    for (int32 i = 0; i < MinReservedCapacity; ++i)
        array.Add(i % 2 == 0);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE  (array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (array.Capacity(), MinReservedCapacity * 2);

    array.Reset();

    GTEST_ASSERT_FALSE(array.IsAllocated());
}


// Element Access

TEST(BitArrayElementAccess, BitReferences)
{
    BitArray<> array;

    array.Add(true);
    array.Add(false);

    array[0] = false;
    array[1] = true;

    GTEST_ASSERT_EQ(false, array[0]);
    GTEST_ASSERT_EQ(true, array[1]);
}

TEST(BitArrayElementAccess, ConstEnumerator)
{
    BitArray<> array;
    array.Add(true);
    array.Add(false);

    auto enumerator = array.Values();
    GTEST_ASSERT_EQ(true, *enumerator);
    GTEST_ASSERT_TRUE(static_cast<bool>(enumerator));
    ++enumerator;
    GTEST_ASSERT_EQ(false, *enumerator);
    GTEST_ASSERT_TRUE(static_cast<bool>(enumerator));
    ++enumerator;
    GTEST_ASSERT_FALSE(static_cast<bool>(enumerator));
}


// Element Relocation

TEST(BitArrayRelocation, MoveCtorNoDragAlloc)
{
    constexpr int32 ElementCount = 128;
    using NoDragAlloc = FixedAlloc<sizeof(uint64) * 2>;

    BitArray<NoDragAlloc> movedArray;

    for (int32 i = 0; i < ElementCount; ++i)
        movedArray.Add(i % 2 == 0);

    BitArray<NoDragAlloc> targetArray{ MOVE(movedArray) };

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, targetArray[i]);
}

TEST(BitArrayRelocation, MoveAsgnNoDragAlloc)
{
    constexpr int32 ElementCount = 128;
    using NoDragAlloc = FixedAlloc<sizeof(uint64) * 2>;

    BitArray<NoDragAlloc> movedArray;

    for (int32 i = 0; i < ElementCount; ++i)
        movedArray.Add(i % 2 == 0);

    BitArray<NoDragAlloc> targetArray = BitArray<NoDragAlloc>{}; // Suppress IDE warning.
    targetArray = MOVE(movedArray);

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, targetArray[i]);
}

TEST(BitArrayRelocation, MoveCtorDragAlloc)
{
    constexpr int32 ElementCount = 128;
    using DragAlloc = HeapAlloc;

    BitArray<DragAlloc> movedArray;

    for (int32 i = 0; i < ElementCount; ++i)
        movedArray.Add(i % 2 == 0);

    BitArray<DragAlloc> targetArray{ MOVE(movedArray) };

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, targetArray[i]);
}

TEST(BitArrayRelocation, MoveAsgnDragAlloc)
{
    constexpr int32 ElementCount = 128;
    using DragAlloc = HeapAlloc;

    BitArray<DragAlloc> movedArray;

    for (int32 i = 0; i < ElementCount; ++i)
        movedArray.Add(i % 2 == 0);

    BitArray<DragAlloc> targetArray = BitArray<DragAlloc>{}; // Suppress IDE warning.
    targetArray = MOVE(movedArray);

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, targetArray[i]);
}


// Element Copying

TEST(BitArrayCopying, CopyCtor)
{
    constexpr int32 ElementCount = 128;

    BitArray<> arraySrc;
    for (int32 i = 0; i < ElementCount; ++i)
        arraySrc.Add(i % 2 == 0);

    BitArray<> arrayDst{ arraySrc };
    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, arrayDst[i]);
}

TEST(BitArrayCopying, CopyAsgn)
{
    constexpr int32 ElementCount = 128;

    BitArray<> arraySrc;
    for (int32 i = 0; i < ElementCount; ++i)
        arraySrc.Add(i % 2 == 0);

    BitArray<> arrayDst = BitArray<>{};
    arrayDst = arraySrc;

    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, arrayDst[i]);
}


// Element Manipulation

TEST(BitArrayElementManipulation, StableInsertOneBlock)
{
    BitArray<> array;
    for (int32 i = 0; i < 6; ++i)
        array.Add(i % 2 == 0);

    // 0 1 2 3 4 5
    // T F T F T F

    array.InsertAtStable(2, true);

    // 0 1 2 3 4 5 6
    // T F T T F T F

    GTEST_ASSERT_EQ(7, array.Count());
    GTEST_ASSERT_EQ(true,  array.GetBit(0));
    GTEST_ASSERT_EQ(false, array.GetBit(1));
    GTEST_ASSERT_EQ(true,  array.GetBit(2));
    GTEST_ASSERT_EQ(true,  array.GetBit(3));
    GTEST_ASSERT_EQ(false, array.GetBit(4));
    GTEST_ASSERT_EQ(true,  array.GetBit(5));
    GTEST_ASSERT_EQ(false, array.GetBit(6));
}

TEST(BitArrayElementManipulation, StableInsertMultipleBlocks)
{
    constexpr int32 ElementCount = 1024;
    constexpr int32 MidPoint = ElementCount / 2;

    BitArray<> arrayOfEvens, arrayOfOdds;

    for (int32 i = 0; i < ElementCount; ++i)
    {
        arrayOfEvens.Add(i % 2 == 0);
        arrayOfOdds .Add(i % 2 == 1);
    }

    arrayOfEvens.InsertAtStable(MidPoint, true);
    arrayOfOdds .InsertAtStable(MidPoint, true);

    for (int32 i = 0; i < MidPoint; ++i)
    {
        GTEST_ASSERT_EQ(i % 2 == 0, arrayOfEvens.GetBit(i));
        GTEST_ASSERT_EQ(i % 2 == 1, arrayOfOdds .GetBit(i));
    }

    GTEST_ASSERT_EQ(true, arrayOfEvens.GetBit(MidPoint));
    GTEST_ASSERT_EQ(true, arrayOfOdds .GetBit(MidPoint));

    for (int32 i = MidPoint + 1; i < ElementCount; ++i)
    {
        GTEST_ASSERT_EQ((i - 1) % 2 == 0, arrayOfEvens.GetBit(i));
        GTEST_ASSERT_EQ((i - 1) % 2 == 1, arrayOfOdds .GetBit(i));
    }
}

TEST(BitArrayElementManipulation, StableRemoveOneBlock)
{
    BitArray<> array;
    for (int32 i = 0; i < 6; ++i)
        array.Add(i % 2 == 0);

    // 0 1 2 3 4 5
    // T F T F T F

    array.RemoveAtStable(2);

    // 0 1 2 3 4
    // T F F T F

    GTEST_ASSERT_EQ(5, array.Count());
    GTEST_ASSERT_EQ(true,  array.GetBit(0));
    GTEST_ASSERT_EQ(false, array.GetBit(1));
    GTEST_ASSERT_EQ(false, array.GetBit(2));
    GTEST_ASSERT_EQ(true,  array.GetBit(3));
    GTEST_ASSERT_EQ(false, array.GetBit(4));
}

TEST(BitArrayElementManipulation, StableRemoveMultipleBlocks)
{
    constexpr int32 ElementCount = 1024;
    constexpr int32 MidPoint = ElementCount / 2;

    BitArray<> arrayOfEvens, arrayOfOdds;

    for (int32 i = 0; i < ElementCount; ++i)
    {
        arrayOfEvens.Add(i % 2 == 0);
        arrayOfOdds.Add(i % 2 == 1);
    }

    arrayOfEvens.RemoveAtStable(MidPoint);
    arrayOfOdds.RemoveAtStable(MidPoint);

    for (int32 i = 0; i < MidPoint; ++i)
    {
        GTEST_ASSERT_EQ(i % 2 == 0, arrayOfEvens.GetBit(i));
        GTEST_ASSERT_EQ(i % 2 == 1, arrayOfOdds.GetBit(i));
    }

    for (int32 i = MidPoint + 1; i < ElementCount - 1; ++i)
    {
        GTEST_ASSERT_EQ(i % 2 == 1, arrayOfEvens.GetBit(i));
        GTEST_ASSERT_EQ(i % 2 == 0, arrayOfOdds.GetBit(i));
    }
}
