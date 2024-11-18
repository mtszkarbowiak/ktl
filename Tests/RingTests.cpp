// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Ring.h"

// Test Ring's ability to set capacity during initialization.
TEST(Ring, Reserving_Init)
{
    using Item = int32;
    constexpr int32 MinReservedCapacity = 128;

    GTEST_ASSERT_GE(MinReservedCapacity, RING_DEFAULT_CAPACITY);
    Ring<Item> ring{ MinReservedCapacity };
    GTEST_ASSERT_TRUE(ring.IsAllocated());
    GTEST_ASSERT_GE(ring.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(ring.Capacity(), MinReservedCapacity * 2);

    ring.Reset();
    GTEST_ASSERT_FALSE(ring.IsAllocated());
}
