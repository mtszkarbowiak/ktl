// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Index.h"
#include "Types/Nullable.h"
#include "Types/Numbers.h"

TEST(NullableByFlagTests, EmptyCtor)
{
    const Nullable<int32> nullable{};
    GTEST_ASSERT_FALSE(nullable.HasValue());
}

TEST(NullableByTombstoneTests, EmptyCtor)
{
    const Nullable<Index> nullable{};
    GTEST_ASSERT_FALSE(nullable.HasValue());
    GTEST_ASSERT_TRUE(sizeof(nullable) == sizeof(Index));
    GTEST_ASSERT_TRUE(sizeof(nullable) == sizeof(int32));
}


TEST(NullableByFlagTests, ValueCtor_EmptyAsgn)
{
    Nullable<int32> nullable{ 69 };
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable = Nullable<int32>{};
    GTEST_ASSERT_FALSE(nullable.HasValue());
}

TEST(NullableByTombstoneTests, ValueCtor_EmptyAsgn)
{
    Nullable<Index> nullable{ 69 };
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable = Nullable<Index>{};
    GTEST_ASSERT_FALSE(nullable.HasValue());
}


TEST(NullableByFlagTests, ValueCtor_ValueAsgn)
{
    Nullable<int32> nullable{ 69 };
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable = 42;
    GTEST_ASSERT_TRUE(nullable.HasValue());
    GTEST_ASSERT_EQ(nullable.Value(), 42);
}

TEST(NullableByTombstoneTests, ValueCtor_ValueAsgn)
{
    Nullable<Index> nullable{ 69 };
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable = 42;
    GTEST_ASSERT_TRUE(nullable.HasValue());
    GTEST_ASSERT_EQ(nullable.Value(), 42);
}

//TODO Lifecycle tests
