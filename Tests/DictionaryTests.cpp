// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Dictionary.h"

TEST(DictionaryTests, Construct)
{
    Dictionary<int32, int32, HeapAlloc, Probing::Linear> dict;
    GTEST_ASSERT_TRUE(dict.IsEmpty());
}

TEST(DictionaryTest, Add)
{
    Dictionary<int32, int32> dict;
    dict.Add(1, 2);
    dict.Add(3, 4);

    GTEST_ASSERT_EQ(dict.Count(), 2);
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
