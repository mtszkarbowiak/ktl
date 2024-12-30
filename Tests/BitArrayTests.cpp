// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/BitArray.h"


// Capacity Management

TEST(BitArray, Reserving_Init)
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

TEST(BitArray, Reserving_Request)
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


// Element Access

TEST(BitArray, BitReferences)
{
    BitArray<> array;

    array.Add(true);
    array.Add(false);

    array[0] = false;
    array[1] = true;

    GTEST_ASSERT_EQ(false, array[0]);
    GTEST_ASSERT_EQ(true, array[1]);
}

TEST(BitArray, ConstEnumerator)
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

//TODO(mtszkarbowiak) Add more tests.
