// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Debugging/LifecycleTracker.h"
#include "Collections/HashSet.h"


// Fundamentals

TEST(HashSet, BasicOperations)
{
	HashSet<int32> set;

	const bool added1 = set.Add(1);
	const bool added2 = set.Add(2);
	const bool added3 = set.Add(3);

    GTEST_ASSERT_TRUE(added1 && added2 && added3);
	GTEST_ASSERT_EQ(set.Count(), 3);

    GTEST_ASSERT_TRUE(set.Contains(1));
    GTEST_ASSERT_TRUE(set.Contains(2));
    GTEST_ASSERT_TRUE(set.Contains(3));

    const bool readded2 = set.Add(2);
    GTEST_ASSERT_FALSE(readded2);

    const bool removed2 = set.Remove(2);
    GTEST_ASSERT_TRUE(removed2);

    const bool readded2AfterRemoval = set.Add(2);
    GTEST_ASSERT_TRUE(readded2AfterRemoval);
}
