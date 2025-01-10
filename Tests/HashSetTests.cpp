// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/CollectionsUtils.h"
#include "Debugging/LifecycleTracker.h"
#include "Collections/HashSet.h"

// Capacity Management

TEST(HashSetCapacity, ReserveOnCall)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, HASH_SETS_DEFAULT_CAPACITY);

    HashSet<int32> set;
    set.ReserveSlots(MinReservedCapacity);

    GTEST_ASSERT_TRUE(set.IsAllocated());
    GTEST_ASSERT_GE  (set.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (set.Capacity(), MinReservedCapacity * 2);

    set.Reset();
    GTEST_ASSERT_FALSE(set.IsAllocated());
}

TEST(HashSetCapacity, ReserveOnCtor)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, HASH_SETS_DEFAULT_CAPACITY);

    HashSet<int32> set{ MinReservedCapacity };

    GTEST_ASSERT_TRUE(set.IsAllocated());
    GTEST_ASSERT_GE  (set.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (set.Capacity(), MinReservedCapacity * 2);

    set.Reset();
    GTEST_ASSERT_FALSE(set.IsAllocated());
}

TEST(HashSetCapacity, ReserveOnAdd)
{
    constexpr int32 MinReservedCapacity = 128;
    GTEST_ASSERT_GE(MinReservedCapacity, HASH_SETS_DEFAULT_CAPACITY);

    HashSet<int32> set;
    for (int32 i = 0; i < MinReservedCapacity; ++i)
        set.Add(i);

    GTEST_ASSERT_TRUE(set.IsAllocated());
    GTEST_ASSERT_GE  (set.Capacity(), MinReservedCapacity);
    GTEST_ASSERT_LE  (set.Capacity(), MinReservedCapacity * 2);

    set.Reset();

    GTEST_ASSERT_FALSE(set.IsAllocated());
}

TEST(HashSetCapacity, FreeOnCompact)
{
    HashSet<int32> set;
    set.Add(69);

    GTEST_ASSERT_TRUE (set.IsAllocated());
    GTEST_ASSERT_FALSE(set.IsEmpty());
    GTEST_ASSERT_EQ   (set.Capacity(), HASH_SETS_DEFAULT_CAPACITY);
    GTEST_ASSERT_EQ   (set.Count(), 1);

    set.Remove(69);

    GTEST_ASSERT_EQ  (set.Count(), 0);
    GTEST_ASSERT_TRUE(set.IsEmpty());
    GTEST_ASSERT_TRUE(set.IsAllocated());

    set.Compact();

    GTEST_ASSERT_TRUE (set.IsEmpty());
    GTEST_ASSERT_FALSE(set.IsAllocated());
}

TEST(HashSetCapacity, CompactOnReloc)
{
    constexpr int32 TestCapacity1 = 256;
    constexpr int32 TestCapacity2 = 3;

    HashSet<int32> set;
    for (int32 i = 0; i < TestCapacity1; ++i)
        set.Add(i);

    GTEST_ASSERT_TRUE(set.IsAllocated());
    GTEST_ASSERT_EQ  (set.Count(), TestCapacity1);
    GTEST_ASSERT_GE  (set.Capacity(), TestCapacity1);

    for (int32 i = TestCapacity2; i < TestCapacity1; ++i)
        set.Remove(i);

    set.Compact();

    GTEST_ASSERT_TRUE(set.IsAllocated());
    GTEST_ASSERT_EQ  (set.Count(),    TestCapacity2);
    GTEST_ASSERT_GE  (set.Capacity(), TestCapacity2);
    GTEST_ASSERT_LE  (set.Capacity(), TestCapacity1 / 2);
}



// Elements Manipulation

template<typename Element, typename Allocator>
struct HashSetTestParam
{
    using ElementType   = Element;
    using AllocatorType = Allocator;
    using HashSetType   = HashSet<ElementType, AllocatorType>;
};

using HashSetTestParams = ::testing::Types<
    HashSetTestParam<int32, DefaultAlloc>
    , HashSetTestParam<int32, FixedAlloc<512>>
    , HashSetTestParam<Index, DefaultAlloc>
    , HashSetTestParam<Index, FixedAlloc<256>>
>;

template<typename Param>
struct HashSetFixture : public ::testing::Test
{
protected:
    using ElementType = typename Param::ElementType;
    const ElementType element1 = ElementType{ 1 };
    const ElementType element2 = ElementType{ 2 };
    const ElementType element3 = ElementType{ 3 };
};

TYPED_TEST_SUITE(HashSetFixture, HashSetTestParams);

TYPED_TEST(HashSetFixture, ElementsManipulation)
{
    typename TypeParam::HashSetType set;


    // Initial state

    EXPECT_EQ(set.Count(), 0);
    EXPECT_FALSE(set.Contains(this->element1));
    EXPECT_FALSE(set.Contains(this->element2));
    EXPECT_FALSE(set.Contains(this->element3));


    // Add elements

    GTEST_ASSERT_TRUE(set.Add(this->element1));
    GTEST_ASSERT_TRUE(set.Add(this->element2));
    GTEST_ASSERT_TRUE(set.Add(this->element3));

    EXPECT_FALSE(set.Add(this->element1));
    EXPECT_FALSE(set.Add(this->element2));
    EXPECT_FALSE(set.Add(this->element3));

    EXPECT_EQ(set.Count(), 3);
    EXPECT_EQ(set.CellCount(), 3);


    // Remove elements

    EXPECT_TRUE(set.Remove(this->element1));
    EXPECT_TRUE(set.Remove(this->element2));
    EXPECT_TRUE(set.Remove(this->element3));

    EXPECT_FALSE(set.Remove(this->element1));
    EXPECT_FALSE(set.Remove(this->element2));
    EXPECT_FALSE(set.Remove(this->element3));

    EXPECT_EQ(set.Count(), 0);
    EXPECT_EQ(set.CellCount(), 3);


    // Add elements again

    EXPECT_FALSE(set.Contains(this->element2));
    EXPECT_TRUE (set.Add(this->element2));
    EXPECT_FALSE(set.Add(this->element2));
    EXPECT_TRUE (set.Contains(this->element2));

    GTEST_ASSERT_EQ(set.Count(), 1);
    GTEST_ASSERT_EQ(set.CellCount(), 4);


    // Compact

    set.Compact();
    GTEST_ASSERT_EQ(set.Count(), 1);
    GTEST_ASSERT_EQ(set.CellCount(), 1);
}
