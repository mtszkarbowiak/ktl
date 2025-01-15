// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Algorithms/Querying.h"
#include "Collections/Dictionary.h"
#include "Collections/HashSet.h"
#include "Debugging/LifecycleTracker.h"
#include "Math/Stastics.h"

TEST(HashSetCursors, Empty)
{
    HashSet<int32> set;
    GTEST_ASSERT_EQ(set.Values().Hint().Min, 0);
    GTEST_ASSERT_EQ(set.Values().Hint().Max.Value(), 0);
}

TEST(HashSetCursors, Count)
{
    HashSet<int32> set;
    set.Add(1);
    set.Add(3);
    set.Add(5);

    GTEST_ASSERT_EQ(Querying::Count(set.Values()), 3);
}

TEST(DictionaryCursors, Empty)
{
    Dictionary<int32, int32> dict;
    GTEST_ASSERT_EQ(dict.Values().Hint().Min, 0);
    GTEST_ASSERT_EQ(dict.Values().Hint().Max.Value(), 0);
    GTEST_ASSERT_EQ(dict.Keys().Hint().Min, 0);
    GTEST_ASSERT_EQ(dict.Keys().Hint().Max.Value(), 0);
    GTEST_ASSERT_EQ(dict.Pairs().Hint().Min, 0);
    GTEST_ASSERT_EQ(dict.Pairs().Hint().Max.Value(), 0);

    GTEST_ASSERT_EQ(Querying::Count(dict.Values()), 0);
    GTEST_ASSERT_EQ(Querying::Count(dict.Keys()), 0);
}

TEST(DictionaryCursors, Hint)
{
    Dictionary<int32, int32> dict;
    dict.Add(3, 0);
    dict.Add(1, 2);
    dict.Add(5, 6);

    dict.Remove(3);
    dict.Add(3, 4);

    GTEST_ASSERT_EQ(dict.Keys().Hint().Min, 3);
    GTEST_ASSERT_EQ(dict.Keys().Hint().Max.Value(), 3);
    GTEST_ASSERT_EQ(dict.Values().Hint().Min, 3);
    GTEST_ASSERT_EQ(dict.Values().Hint().Max.Value(), 3);
    GTEST_ASSERT_EQ(dict.Pairs().Hint().Min, 3);
    GTEST_ASSERT_EQ(dict.Pairs().Hint().Max.Value(), 3);
}

TEST(DictionaryCursors, Count)
{
    using namespace Querying;

    Dictionary<int32, int32> dict;
    dict.Add(3, 0);
    dict.Add(1, 2);
    dict.Add(5, 6);

    dict.Remove(3);
    dict.Add(3, 4);

    const int32 count = Count(dict.Values());
    GTEST_ASSERT_EQ(count, 3);

    const int32 countKeys = Count(dict.Keys());
    GTEST_ASSERT_EQ(countKeys, 3);

    const int32 countPairs = Count(dict.Pairs());
    GTEST_ASSERT_EQ(countPairs, 3);
}

TEST(DictionaryCursors, Sum)
{
    using namespace Querying;
    using namespace Statistics;

    Dictionary<int32, int32> dict;
    dict.Add(3, 0);
    dict.Add(1, 2);
    dict.Add(5, 6);

    dict.Remove(3);
    dict.Add(3, 4);

    // Values
    const int32 sum = Sum(dict.Values());
    GTEST_ASSERT_EQ(sum, 2 + 4 + 6);

    // Keys
    const int32 sumKeys = Sum(dict.Keys());
    GTEST_ASSERT_EQ(sumKeys, 1 + 3 + 5);

    // Pairs
    const auto sumKeyVal = [](const auto& pair) -> int32
    {
        return *pair.Key + *pair.Value;
    };
    const int32 sumPairs = dict.Pairs()
        | Select<decltype(sumKeyVal)>(sumKeyVal)
        | ToSum();
    GTEST_ASSERT_EQ(sumPairs, 1 + 2 + 3 + 4 + 5 + 6);
}
