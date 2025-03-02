// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Debugging/LifecycleTracker.h"
#include "Types/Index.h"
#include "Types/Nullable.h"
#include "Types/NullableAsSpan.h"
#include "Types/Numbers.h"
#include "Types/Ref.h"


// static_assert(THasTrivialCtorV<Index>, "");
static_assert(THasTrivialCopyV<Index>, "");
static_assert(THasTrivialDtorV<Index>, "");

// static_assert(THasTrivialCtorV<Nullable<Index>>, "");
// static_assert(THasTrivialCopyV<Nullable<Index>>, "");
// static_assert(THasTrivialDtorV<Nullable<Index>>, "");

//TODO: Add Nullable trivial specializations.


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


TEST(NullableByFlagTests, ValueClearing)
{
    Nullable<int32> nullable;
    nullable.Set(69);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable.Clear();
    GTEST_ASSERT_FALSE(nullable.HasValue());
}

TEST(NullableByTombstone, ValueClearing)
{
    Nullable<Index> nullable;
    nullable.Set(69);
    GTEST_ASSERT_TRUE(nullable.HasValue());
    nullable.Clear();
    GTEST_ASSERT_FALSE(nullable.HasValue());
}


TEST(NullableByFlagTests, ValueCtor)
{
    LIFECYCLE_TEST_INTO
    {
        Nullable<TestTracker> nullable;
    }
    LIFECYCLE_TEST_OUT
}

TEST(NullableByFlagTests, ValueAsgn)
{
    LIFECYCLE_TEST_INTO
    {
        Nullable<TestTracker> nullable;
        GTEST_ASSERT_EQ(LifecycleCountersInstance.Instances, 0); // The type should not have been initialized!
        GTEST_ASSERT_FALSE(nullable.HasValue());
        nullable.Set(TestTracker{ 69 });
        GTEST_ASSERT_TRUE(nullable.HasValue());
    }
    LIFECYCLE_TEST_OUT
}

TEST(NullableByTombstoneTests, RefExample)
{
    LIFECYCLE_TEST_INTO
    {
        TestTracker tracker{ 69 };

        Nullable<Ref<TestTracker>> nullable{};
        GTEST_ASSERT_FALSE(nullable.HasValue());

        nullable.Set(Ref<TestTracker>{ tracker });
        GTEST_ASSERT_TRUE(nullable.HasValue());

        // Unfortunately, for now testing the lifecycle of tombstone is impossible.
    }
    LIFECYCLE_TEST_OUT
}


namespace SentinelNullables
{
    using Nullable0 = Nullable<int>;
    using Nullable1 = Nullable<Nullable0>;
    using Nullable2 = Nullable<Nullable1>;

    static_assert(TMaxTombstoneDepth<int>::Value == 0, "");
    static_assert(TMaxTombstoneDepth<Nullable0>::Value == 64, "");
    static_assert(TMaxTombstoneDepth<Nullable1>::Value == 63, "");
    static_assert(TMaxTombstoneDepth<Nullable2>::Value == 62, "");

    static_assert(sizeof(Nullable1) == sizeof(Nullable0), "");
    static_assert(sizeof(Nullable2) == sizeof(Nullable0), "");
}

TEST(NullableNested, NestedSentinel_Double)
{
    using namespace SentinelNullables;
    
    const Nullable1 nullableC{ MakeNullable<Nullable0>(NullOptT{}) };
    const Nullable1 nullableB{ MakeNullable<Nullable0>(MakeNullable<int>(NullOptT{})) };
    const Nullable1 nullableA{ MakeNullable<Nullable0>(MakeNullable<int>(69)) };

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

    const Nullable2 nullableA{ MakeNullable<Nullable1>(MakeNullable<Nullable0>(MakeNullable<int>(69))) };
    const Nullable2 nullableB{ MakeNullable<Nullable1>(MakeNullable<Nullable0>(MakeNullable<int>(NullOptT{}))) };
    const Nullable2 nullableC{ MakeNullable<Nullable1>(MakeNullable<Nullable0>(NullOptT{})) };
    const Nullable2 nullableD{ MakeNullable<Nullable1>(NullOptT{}) };

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

    const Nullable2 nullableC{ MakeNullable<Nullable1>(NullOptT{}) };
    const Nullable2 nullableB{ MakeNullable<Nullable1>(MakeNullable<Index>(NullOptT{})) };
    const Nullable2 nullableA{ MakeNullable<Nullable1>(MakeNullable<Index>({ 69 })) };

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


TEST(NullableUtilities, BoolConversion)
{
    Nullable<int32> nullableA;
    Nullable<Index> nullableB;
    GTEST_ASSERT_FALSE(static_cast<bool>(nullableA));
    GTEST_ASSERT_FALSE(static_cast<bool>(nullableB));
    nullableA.Set(69);
    nullableB.Set({ 69 });
    GTEST_ASSERT_TRUE(static_cast<bool>(nullableA));
    GTEST_ASSERT_TRUE(static_cast<bool>(nullableB));
    nullableA.Clear(); // BUG! Non-tombstone nullable here does not clear the value!
    nullableB.Clear();
    GTEST_ASSERT_FALSE(static_cast<bool>(nullableA));
    GTEST_ASSERT_FALSE(static_cast<bool>(nullableB));
}

TEST(NullableUtilities, ValueEmplacement)
{
    LIFECYCLE_TEST_INTO
    {
        Nullable<TestTracker> nullable;
        nullable.Emplace(69);
        GTEST_ASSERT_TRUE(nullable.HasValue());
        GTEST_ASSERT_EQ(nullable.Value().Value, 69);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1); // Only one instance should be created. Without temporary.
}

//TEST(NullableUtilities, AsSpan)
//{
//    Nullable<int32> nullableA;
//    Nullable<Index> nullableB;
//    GTEST_ASSERT_EQ(nullableA.AsSpan().Count(), 0);
//    GTEST_ASSERT_EQ(nullableB.AsSpan().Count(), 0);
//    nullableA.Set(69);
//    nullableB.Set({ 69 });
//    GTEST_ASSERT_EQ(nullableA.AsSpan().Count(), 1);
//    GTEST_ASSERT_EQ(nullableB.AsSpan().Count(), 1);
//    GTEST_ASSERT_EQ(nullableA.AsSpan()[0], 69);
//    GTEST_ASSERT_EQ(nullableB.AsSpan()[0], 69);
//}


namespace TombstonePropagation
{
    /// <summary>
    /// Custom value type used to test if Nullable propagates null correctly.
    /// The main idea is that GetTombstoneLevel() returns the depth of the tombstone,
    /// EVEN IF the value is not a tombstone. Nullable must not use the level value,
    /// before checking if the value is a tombstone in the first place.
    /// </summary>
    struct CustomType
    {
        int8 Value;
        bool MIsTombstone;

        auto IsTombstone() const -> bool
        {
            return MIsTombstone;
        }

        auto GetTombstoneLevel() const -> int8
        {
            return Value;
        }

        explicit CustomType(const int8 value)
            : Value{ value }
            , MIsTombstone{ false }
        {
        }

        explicit CustomType(const TombstoneDepth depth)
            : Value{ depth.Value }
            , MIsTombstone{ true }
        {
        }
    };
}

template<>
struct TMaxTombstoneDepth<TombstonePropagation::CustomType>
{
    enum { Value = 64 }; // More than enough for any collection.
};

TEST(NullableNested, Propagation)
{
    using namespace TombstonePropagation;

    using Nullable0 = Nullable<CustomType>;
    using Nullable1 = Nullable<Nullable0>;
    using Nullable2 = Nullable<Nullable1>;

    static_assert(sizeof(Nullable2) == sizeof(CustomType), "");

    Nullable2 nullableA{};
    nullableA.Set(MakeNullable<Nullable0>(MakeNullable<CustomType>(CustomType{ 69 })));

    GTEST_ASSERT_TRUE(nullableA.HasValue());
}
