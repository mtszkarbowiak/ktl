// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/CollectionsUtils.h"
#include "Debugging/LifecycleTracker.h"
#include "Collections/HashSet.h"


// Fundamentals

using AllocatorTypes = ::testing::Types<DefaultAlloc, FixedAlloc<256>>;
using ElementType    = ::testing::Types<int32, Index>;

template<typename Element, typename Allocator>
struct HashSetTestParam
{
    using ElementType   = Element;
    using AllocatorType = Allocator;
};

using HashSetTestParams = ::testing::Types<
    HashSetTestParam<int32, DefaultAlloc>,
    HashSetTestParam<int32, FixedAlloc<512>>,
    HashSetTestParam<Index, DefaultAlloc>,
    HashSetTestParam<Index, FixedAlloc<256>>
>;

template<typename Param>
struct HashSetFixture : public ::testing::Test
{
protected:
    using ElementType   = typename Param::ElementType;
    using AllocatorType = typename Param::AllocatorType;
    using HashSetType   = HashSet<ElementType, AllocatorType>;

    HashSetType set;
    const ElementType element1 = ElementType{ 1 };
    const ElementType element2 = ElementType{ 2 };
    const ElementType element3 = ElementType{ 3 };
};

TYPED_TEST_SUITE(HashSetFixture, HashSetTestParams);

TYPED_TEST(HashSetFixture, EmptyAfterCtor)
{
    EXPECT_TRUE(this->set.IsEmpty());
    EXPECT_EQ  (this->set.Count(), 0);
    EXPECT_EQ  (this->set.CellCount(), 0);

    EXPECT_FALSE(this->set.Contains(this->element1));
    EXPECT_FALSE(this->set.Contains(this->element2));
    EXPECT_FALSE(this->set.Contains(this->element3));
}

TYPED_TEST(HashSetFixture, ElementsManipulation)
{
    // Add elements

    GTEST_ASSERT_TRUE(this->set.Add(this->element1));
    GTEST_ASSERT_TRUE(this->set.Add(this->element2));
    GTEST_ASSERT_TRUE(this->set.Add(this->element3));

    EXPECT_FALSE(this->set.Add(this->element1));
    EXPECT_FALSE(this->set.Add(this->element2));
    EXPECT_FALSE(this->set.Add(this->element3));

    EXPECT_EQ(this->set.Count(), 3);
    EXPECT_EQ(this->set.CellCount(), 3);


    // Remove elements

    EXPECT_TRUE(this->set.Remove(this->element1));
    EXPECT_TRUE(this->set.Remove(this->element2));
    EXPECT_TRUE(this->set.Remove(this->element3));

    EXPECT_FALSE(this->set.Remove(this->element1));
    EXPECT_FALSE(this->set.Remove(this->element2));
    EXPECT_FALSE(this->set.Remove(this->element3));

    EXPECT_EQ(this->set.Count(), 0);
    EXPECT_EQ(this->set.CellCount(), 3);


    // Add elements again

    EXPECT_FALSE(this->set.Contains(this->element2));
    EXPECT_TRUE (this->set.Add(this->element2));
    EXPECT_FALSE(this->set.Add(this->element2));
    EXPECT_TRUE (this->set.Contains(this->element2));

    GTEST_ASSERT_EQ(this->set.Count(), 1);
    GTEST_ASSERT_EQ(this->set.CellCount(), 4);


    // Compact

    this->set.Compact();
    GTEST_ASSERT_EQ(this->set.Count(), 1);
    GTEST_ASSERT_EQ(this->set.CellCount(), 1);
}
