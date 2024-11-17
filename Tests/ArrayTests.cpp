// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "../Collections/Array.h"

// Test Array's ability to set capacity during initialization.
TEST(Array, Reserving_Init)
{
    using Item = int32;
    constexpr int32 MinReservedCapacity = 128;

    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);
    Array<Item> array{ MinReservedCapacity };
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

// Test Array's ability to set capacity after initialization,
// without a need to relocate any elements.
TEST(Array, Reserving_Request_NoReloc)
{
    using Item = int32;
    constexpr int32 MinReservedCapacity = 128;

    GTEST_ASSERT_GE(MinReservedCapacity, ARRAY_DEFAULT_CAPACITY);
    Array<Item> array;
    array.Reserve(MinReservedCapacity);
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE(array.Capacity(), MinReservedCapacity * 2);

    array.Reset();
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

// Test Array's ability to set capacity after initialization,
// with a need to relocate C-Style elements.
TEST(Array, Reserving_Request_Reloc_Fast)
{
    using Item = int32;

    constexpr int32 TestCapacity = 64;

    Array<Item> array{ 3 };
    GTEST_ASSERT_GE(array.Capacity(), 3);
    GTEST_ASSERT_LT(array.Capacity(), TestCapacity);

    array.Add(1);
    array.Add(2);
    array.Add(3);

    GTEST_ASSERT_EQ(array.Count(), 3);
    GTEST_ASSERT_EQ(array[0], 1);
    GTEST_ASSERT_EQ(array[1], 2);
    GTEST_ASSERT_EQ(array[2], 3);

    array.Reserve(TestCapacity);
    GTEST_ASSERT_GE(array.Capacity(), TestCapacity);

    // Is payload preserved?
    GTEST_ASSERT_EQ(array.Count(), 3);
    GTEST_ASSERT_EQ(array[0], 1);
    GTEST_ASSERT_EQ(array[1], 2);
    GTEST_ASSERT_EQ(array[2], 3);
}

// Test Array's ability to set capacity after initialization,
// with a need to relocate C++-Style elements.
TEST(Array, Reserving_Request_Reloc_Full)
{
    class Item final
    {
    public:
        int32 Value;

        explicit Item(const int32 value = int32{})
            : Value{ value } {}

        Item(Item&&)                 = default;
        Item& operator=(Item&&)      = default;
        ~Item()                      = default;

        Item(const Item&)            = delete;
        Item& operator=(const Item&) = delete;
    };

    constexpr int32 TestCapacity = 64;

    Array<Item> array{ 3 };
    GTEST_ASSERT_GE(array.Capacity(), 3);
    GTEST_ASSERT_LT(array.Capacity(), TestCapacity);

    array.Add(Item{ 1 });
    array.Add(Item{ 2 });
    array.Add(Item{ 3 });

    GTEST_ASSERT_EQ(array.Count(), 3);
    GTEST_ASSERT_EQ(array[0].Value, 1);
    GTEST_ASSERT_EQ(array[1].Value, 2);
    GTEST_ASSERT_EQ(array[2].Value, 3);

    array.Reserve(TestCapacity);
    GTEST_ASSERT_GE(array.Capacity(), TestCapacity);

    // Is payload preserved?
    GTEST_ASSERT_EQ(array.Count(), 3);
    GTEST_ASSERT_EQ(array[0].Value, 1);
    GTEST_ASSERT_EQ(array[1].Value, 2);
    GTEST_ASSERT_EQ(array[2].Value, 3);
}

// Test Array's ability to free allocation for empty arrays.
TEST(Array, ShrinkToFit_Free)
{
    using Item = int32;
    Array<Item> array;

    array.Add(69);
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_FALSE(array.IsEmpty());
    GTEST_ASSERT_EQ(array.Capacity(), ARRAY_DEFAULT_CAPACITY);
    GTEST_ASSERT_EQ(array.Count(), 1);

    array.RemoveAt(0);
    GTEST_ASSERT_EQ(array.Count(), 0);
    GTEST_ASSERT_TRUE(array.IsEmpty());
    GTEST_ASSERT_TRUE(array.IsAllocated());

    array.ShrinkToFit();
    GTEST_ASSERT_TRUE(array.IsEmpty());
    GTEST_ASSERT_FALSE(array.IsAllocated());
}

TEST(Array, ShrinkToFit_Reloc_Any)
{
    constexpr int32 TestCapacity1 = 256;
    constexpr int32 TestCapacity2 = 3;

    Array<int32> array;
    for (int32 i = 0; i < TestCapacity1; ++i)
        array.Add(i);

    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), TestCapacity1);
    GTEST_ASSERT_EQ(array.Count(),    TestCapacity1);

    while (array.Count() > TestCapacity2)
        array.RemoveAt(0);

    array.ShrinkToFit();
    GTEST_ASSERT_TRUE(array.IsAllocated());
    GTEST_ASSERT_GE(array.Capacity(), TestCapacity2);
    GTEST_ASSERT_EQ(array.Count(),    TestCapacity2);

    GTEST_ASSERT_LE(array.Capacity(), TestCapacity1 / 2);
}
