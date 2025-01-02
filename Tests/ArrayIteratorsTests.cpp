// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Array.h"

TEST(ArrayIterators, ConstEnumerator)
{
    constexpr int32 TestElements = 10;

    Array<int32> array;
    int32 expectedSum = 0;

    for (int32 i = 0; i < TestElements; ++i) 
    {
        array.Add(i);
        expectedSum += i;
    }

    int32 actualSum = 0;
    int32 index = 0;
    int32 elementsLeft = array.Count(); // Hints are inclusive!

    for (auto enumerator = array.Values(); enumerator; ++enumerator) 
    {
        actualSum += *enumerator;

        GTEST_ASSERT_EQ(enumerator.Hint().Min, elementsLeft);
        GTEST_ASSERT_EQ(enumerator.Hint().Max, elementsLeft);

        ++index;
        --elementsLeft;
    }
    
    GTEST_ASSERT_EQ(expectedSum, actualSum);
}

TEST(ArrayIterators, MutableEnumerator)
{
    constexpr int32 TestElements = 10;

    Array<int32> array;
    int32 expectedSum = 0;

    for (int32 i = 0; i < TestElements; ++i)
    {
        array.Add(i);
        expectedSum += i;
    }

    int32 index = 0;
    int32 elementsLeft = array.Count(); // Hints are inclusive!
    for (auto enumerator = array.Values(); enumerator; ++enumerator) 
    {
        *enumerator = TestElements - *enumerator;

        GTEST_ASSERT_EQ(enumerator.Hint().Min, elementsLeft);
        GTEST_ASSERT_EQ(enumerator.Hint().Max, elementsLeft);

        ++index;
        --elementsLeft;
    }
    
    int32 actualSum = 0;
    for (auto enumerator = array.Values(); enumerator; ++enumerator)
        actualSum += *enumerator;
}

TEST(ArrayIterators, EnumeratorEquality)
{
    constexpr int32 TestElements = 3;

    Array<int32> array;
    for (int32 i = 0; i < TestElements; ++i)
        array.Add(i);

    auto enumerator1 = array.Values();
    auto enumerator2 = array.Values();

    GTEST_ASSERT_EQ(enumerator1, enumerator2);
    ++enumerator1;
    GTEST_ASSERT_NE(enumerator1, enumerator2);
    ++enumerator2;
    GTEST_ASSERT_EQ(enumerator1, enumerator2);
}
