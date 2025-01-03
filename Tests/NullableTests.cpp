// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Collections/Index.h"
#include "Debugging/LifecycleTracker.h"
#include "Types/Nullable.h"
#include "Types/Numbers.h"
#include "Types/Ref.h"


TEST(NullableByFlagTests, EmptyCtor)
{
    Nullable<int32> nullable;
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
    Nullable<int32> nullable;
    nullable.Set(69);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable = Nullable<int32>{};
    GTEST_ASSERT_FALSE(nullable.HasValue());
}

TEST(NullableByTombstoneTests, ValueCtor_EmptyAsgn)
{
    Nullable<Index> nullable;
    nullable.Set(69);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable = Nullable<Index>{};
    GTEST_ASSERT_FALSE(nullable.HasValue());
}


TEST(NullableByFlagTests, ValueCtor_ValueAsgn)
{
    Nullable<int32> nullable;
    nullable.Set(69);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable.Set(42);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    GTEST_ASSERT_EQ(nullable.Value(), 42);
}

TEST(NullableByTombstoneTests, ValueCtor_ValueAsgn)
{
    Nullable<Index> nullable;
    nullable.Set(69);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable.Set(42);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    GTEST_ASSERT_EQ(nullable.Value(), 42);
}


//TEST(NullableByFlagTests, ValueCtor)
//{
//    LIFECYCLE_TEST_INTO
//    {
//        Nullable<TestTracker> nullable;
//    }
//    LIFECYCLE_TEST_OUT
//}

//TEST(NullableByFlagTests, ValueAsgn)
//{
//    LIFECYCLE_TEST_INTO
//    {
//        Nullable<TestTracker> nullable;
//        GTEST_ASSERT_FALSE(nullable.HasValue());
//        nullable = TestTracker{ 69 };
//        GTEST_ASSERT_TRUE(nullable.HasValue());
//    }
//    LIFECYCLE_TEST_OUT
//}
//
//TEST(NullableByTombstoneTests, RefExample)
//{
//    LIFECYCLE_TEST_INTO
//    {
//        TestTracker tracker{ 69 };
//
//        Nullable<Ref<TestTracker>> nullable{};
//        GTEST_ASSERT_FALSE(nullable.HasValue());
//
//        nullable = Ref<TestTracker>{ tracker };
//        GTEST_ASSERT_TRUE(nullable.HasValue());
//
//        // Unfortunately, for now testing the lifecycle of tombstone is impossible.
//    }
//    LIFECYCLE_TEST_OUT
//}


namespace SentinelNullables
{
    using Nullable0 = Nullable<int>;
    using Nullable1 = Nullable<Nullable0>;
    using Nullable2 = Nullable<Nullable1>;

    static_assert(GetMaxTombstoneDepth<int>::Value == 0, "");
    static_assert(GetMaxTombstoneDepth<Nullable0>::Value == 64, "");
    static_assert(GetMaxTombstoneDepth<Nullable1>::Value == 63, "");
    static_assert(GetMaxTombstoneDepth<Nullable2>::Value == 62, "");

    static_assert(sizeof(Nullable1) == sizeof(Nullable0), "");
    static_assert(sizeof(Nullable2) == sizeof(Nullable0), "");
}


TEST(NullableNested, NestedSentinel_Double)
{
    using namespace SentinelNullables;

    const Nullable1 nullableC{};
    const Nullable1 nullableB{ Nullable0{} };
    const Nullable1 nullableA{ Nullable0{ 69 } };

    //
    GTEST_ASSERT_EQ(nullableC.HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableB.HasValue(), true);
    GTEST_ASSERT_EQ(nullableB.Value().HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableA.HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().Value(), 69);
}


TEST(NullableNested, NestedSentinel_Triple)
{
    using namespace SentinelNullables;

    const Nullable2 nullableA{ Nullable1{ Nullable0{ 69 } } };
    const Nullable2 nullableB{ Nullable1{ Nullable0{} } };
    const Nullable2 nullableC{ Nullable1{} };
    const Nullable2 nullableD{};

    //
    GTEST_ASSERT_EQ(nullableD.HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableC.HasValue(), true);
    GTEST_ASSERT_EQ(nullableC.Value().HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableB.HasValue(), true);
    GTEST_ASSERT_EQ(nullableB.Value().HasValue(), true);
    GTEST_ASSERT_EQ(nullableB.Value().Value().HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableA.HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().Value().HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().Value().Value(), 69);
}

namespace TombstoneNullables
{
    // using Nullable0 = Index; // Index has a tombstone.
    using Nullable1 = Nullable<Index>;
    using Nullable2 = Nullable<Nullable1>;

    static_assert(sizeof(Nullable1) == sizeof(Index), "");
    static_assert(sizeof(Nullable2) == sizeof(Index), "");
}


TEST(NullableNested, NestedTombstone_Double)
{
    using namespace TombstoneNullables;

    const Nullable2 nullableC{};
    const Nullable2 nullableB{ Nullable1{} };
    const Nullable2 nullableA{ Nullable1{ Index{ 69 } } };

    //
    GTEST_ASSERT_EQ(nullableC.HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableB.HasValue(), true);
    GTEST_ASSERT_EQ(nullableB.Value().HasValue(), false);
    //
    GTEST_ASSERT_EQ(nullableA.HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().HasValue(), true);
    GTEST_ASSERT_EQ(nullableA.Value().Value(), Index{ 69 });
}
