// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Array.h"
#include "Algorithms/Querying.h"

TEST(Querying, Count)
{
    Array<int32> array;
    array.Add(1);
    array.Add(2);
    array.Add(3);
    auto count = Querying::Count(array.Enumerate());
    ASSERT_EQ(count, 3);
}

TEST(Querying, Sum)
{
    Array<int32> array;
    array.Add(1);
    array.Add(2);
    array.Add(3);
    auto sum = Querying::Sum(array.Enumerate());
    ASSERT_EQ(sum, 6);
}

TEST(Querying, Average)
{
    Array<int32> array;
    array.Add(1);
    array.Add(2);
    array.Add(3);
    auto average = Querying::Average(array.Enumerate());
    ASSERT_EQ(average, 2);
}
