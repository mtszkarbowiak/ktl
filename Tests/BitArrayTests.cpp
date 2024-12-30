// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/BitArray.h"


// Capacity Management

TEST(BitArray_Capacity, Reserving_Init)
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

TEST(BitArray_Capacity, Reserving_Request)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);

    BitArray<> array;
    array.Reserve(MinReservedCapacity);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE  (array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (array.Capacity(), MinReservedCapacity * 2);

    array.Reset();

    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(BitArray_Capacity, Reserving_Add)
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

TEST(BitArray_ElementAccess, BitReferences)
{
    BitArray<> array;

    array.Add(true);
    array.Add(false);

    array[0] = false;
    array[1] = true;

    GTEST_ASSERT_EQ(false, array[0]);
    GTEST_ASSERT_EQ(true, array[1]);
}

TEST(BitArray_ElementAccess, ConstEnumerator)
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

TEST(BitArray_Relocation, MoveConstruct_NoDragAlloc)
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

TEST(BitArray_Relocation, MoveAssignment_NoDragAlloc)
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

TEST(BitArray_Relocation, MoveConstruct_DragAlloc)
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

TEST(BitArray_Relocation, MoveAssignment_DragAlloc)
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

TEST(BitArray_Copying, CopyCtr)
{
    constexpr int32 ElementCount = 128;

    BitArray<> arraySrc;
    for (int32 i = 0; i < ElementCount; ++i)
        arraySrc.Add(i % 2 == 0);

    BitArray<> arrayDst{ arraySrc };
    for (int32 i = 0; i < ElementCount; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, arrayDst[i]);
}

TEST(BitArray_Copying, CopyAsg)
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

TEST(BitArray_ElementManipulation, StableInsert_OneBlock)
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

TEST(BitArray_ElementManipulation, StableInsert_MultipleBlocks)
{
    constexpr int32 ElementCount = 1024;

    BitArray<> array;
    for (int32 i = 0; i < ElementCount; ++i)
        array.Add(i % 2 == 0);

    array.InsertAtStable(ElementCount / 2, true);

    for (int32 i = 0; i < ElementCount / 2; ++i)
        GTEST_ASSERT_EQ(i % 2 == 0, array.GetBit(i));

    GTEST_ASSERT_EQ(true, array.GetBit(ElementCount / 2));

    for (int32 i = ElementCount / 2 + 1; i < ElementCount; ++i)
        GTEST_ASSERT_EQ((i - 1) % 2 == 0, array.GetBit(i));
}

//TEST(BitArray_ElementManipulation, StableRemove)
//{
//    BitArray<> array;
//    for (int32 i = 0; i < 6; ++i)
//        array.Add(i % 2 == 0);
//
//    // 0 1 2 3 4 5
//    // T F T F T F
//
//    array.RemoveAtStable(2);
//
//    // 0 1 2 3 4
//    // T F F T F
//
//    GTEST_ASSERT_EQ(5, array.Count());
//    GTEST_ASSERT_EQ(true,  array.GetBit(0));
//    GTEST_ASSERT_EQ(false, array.GetBit(1));
//    GTEST_ASSERT_EQ(false, array.GetBit(2));
//    GTEST_ASSERT_EQ(true,  array.GetBit(3));
//    GTEST_ASSERT_EQ(false, array.GetBit(4));
//}
