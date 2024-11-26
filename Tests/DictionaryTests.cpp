// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Dictionary.h"


// Capacity Management

TEST(Dictionary_Capacity, Reserve_OnInit)
{
    constexpr int32 ExampleDefaultCapacity = 10;
    Dictionary<int32, int32> dict{ ExampleDefaultCapacity };
    GTEST_ASSERT_GE(HASH_MAPS_DEFAULT_CAPACITY, ExampleDefaultCapacity);
    GTEST_ASSERT_EQ(dict.Capacity(), HASH_MAPS_DEFAULT_CAPACITY);
}


TEST(DictionaryTest, Reserve_Add)
{
    Dictionary<int32, int32> dict;
    dict.Add(1, 2);
    dict.Add(3, 4);

    GTEST_ASSERT_EQ(dict.Count(), 2);
}

TEST(DictionaryTest, AddMany)
{
    Dictionary<int32, int32> dict;

    for (int32 i = 0; i < 1000; ++i)
    {
        dict.Add(i, i);
    }

    GTEST_ASSERT_EQ(dict.Count(), 1000);

    for (int32 i = 0; i < 1000; ++i)
    {
        GTEST_ASSERT_TRUE(dict.Contains(i));
        GTEST_ASSERT_EQ(*dict.TryGet(i), i);
    }
}

TEST(DictionaryTest, AddManyRemoveMany)
{
    Dictionary<int32, int32> dict;

    for (int32 i = 0; i < 1000; ++i)
    {
        dict.Add(i, i);
    }

    GTEST_ASSERT_EQ(dict.Count(), 1000);

    for (int32 i = 0; i < 1000; ++i)
    {
        GTEST_ASSERT_TRUE(dict.Contains(i));
        GTEST_ASSERT_EQ(*dict.TryGet(i), i);
    }

    for (int32 i = 0; i < 1000; ++i)
    {
        if (i % 200 == 0) 
        {
            dict.Compact();
        }

        dict.Remove(i);
    }
}

TEST(DictionaryTest, AddAndGet)
{
    Dictionary<int32, int32> dict;
    dict.Add(1, 2);
    dict.Add(3, 4);

    const int32 value1 = *dict.TryGet(1);
    const int32 value2 = *dict.TryGet(3);

    GTEST_ASSERT_EQ(value1, 2);
    GTEST_ASSERT_EQ(value2, 4);
}
