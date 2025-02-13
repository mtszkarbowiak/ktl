// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Language/Templates.h"
#include "Math/Hashing.h"
#include "Types/Numbers.h"
#include "Types/EnumPuller.h"
#include "Types/RcBox.h"
#include "Types/RangePuller.h"
#include "Types/EnumSet.h"


TEST(TypeUtils, SwapByMoves)
{
    struct NoSwap
    {
        int32 _value;

        explicit NoSwap(const int32 value)
            : _value{ value }
        {}

        NoSwap(NoSwap&& other) noexcept
            : _value{ other._value }
        {}

        NoSwap& operator=(NoSwap&& other) noexcept
        {
            _value = other._value;
            return *this;
        }
    };

    NoSwap a{ 1 };
    NoSwap b{ 2 };

    ::Swap(a, b);

    GTEST_ASSERT_EQ(a._value, 2);
    GTEST_ASSERT_EQ(b._value, 1);
}

TEST(TypeUtils, SwapByMember)
{
    struct HasSwap
    {
        int32 _value;

        explicit HasSwap(const int32 value)
            : _value{ value }
        {}

        void Swap(HasSwap& other) noexcept
        {
            // Here we can swap the members directly.
            ::Swap(_value, other._value);
        }

        HasSwap(HasSwap&& other) = delete;

        HasSwap& operator=(HasSwap&& other) = delete;
    };

    HasSwap a{ 1 };
    HasSwap b{ 2 };

    ::Swap(a, b);

    GTEST_ASSERT_EQ(a._value, 2);
    GTEST_ASSERT_EQ(b._value, 1);
}


TEST(Hashing, PodHash)
{
    struct SomeStruct
    {
        int X, Y;
    };

    constexpr SomeStruct A{ 1, 2 };
    constexpr SomeStruct B{ 3, 4 };

    const uint32 hashA = PodHashOf<SomeStruct>::GetHash(A);
    const uint32 hashB = PodHashOf<SomeStruct>::GetHash(B);

    GTEST_ASSERT_NE(hashA, hashB);
}


TEST(RefCounted, Rc)
{
    Rc rc{};
    int32 resource{ 1 };

    {
        RcWrite<int32> write{ rc, resource };
        GTEST_ASSERT_TRUE(write.HasValue());
        *write = 3;

        write.Clear();
        GTEST_ASSERT_FALSE(write.HasValue());
    }

    {
        RcRead<int32> read{ rc, resource };
        GTEST_ASSERT_TRUE(read.HasValue());
        GTEST_ASSERT_TRUE(read.IsUnique());
        GTEST_ASSERT_EQ(*read, 3);

        RcRead<int32> read2{ read };
        GTEST_ASSERT_FALSE(read.IsUnique());
        GTEST_ASSERT_FALSE(read2.IsUnique());
    }
}

TEST(RefCounted, RcBox)
{
    RcBox<int32> box;
    box.Emplace(7);

    {
        RcWrite<int32> write = box.TryWrite();
        GTEST_ASSERT_TRUE(static_cast<bool>(write));
        *write = 3;
    }
    {
        RcRead<int32> read = box.TryRead();
        GTEST_ASSERT_TRUE(static_cast<bool>(read));
        GTEST_ASSERT_EQ(*read, 3);

        RcWrite<int32> write = box.TryWrite();
        GTEST_ASSERT_FALSE(static_cast<bool>(write)); // Read lock is active.
    }
    {
        RcRead<int32> read = box.TryRead();
        GTEST_ASSERT_TRUE(static_cast<bool>(read));
        GTEST_ASSERT_EQ(*read, 3);
    }
}

TEST(EnumPuller, Simple)
{
    enum class ExampleEnum : int32
    {
        A = 4, // Start at arbitrary value.
        B,
        C, // Increment by one.
    };

    using ExampleEnumPuller = EnumPuller<ExampleEnum, ExampleEnum::C, ExampleEnum::A>;

    int32 count = 0, sum = 0;
    for (ExampleEnumPuller it; it; ++it)
    {
        ++count;
        sum += static_cast<int32>(*it);
    }

    GTEST_ASSERT_EQ(count, 3);
    GTEST_ASSERT_EQ(sum, 4 + 5 + 6);
}


TEST(CompilerIntrinsics, PopCount32)
{
    struct Case
    {
        uint32 Value;
        int32 Expected;
    };

    const Case cases[] = {
        { 0x00000000, 0 },  // All bits zero
        { 0xFFFFFFFF, 32 }, // All bits set
        { 0x00000001, 1 },  // Single bit set at LSB
        { 0x80000000, 1 },  // Single bit set at MSB
        { 0xAAAAAAAA, 16 }, // Alternating bits (1010...)
        { 0x55555555, 16 }, // Alternating bits (0101...)
        { 0x0F0F0F0F, 16 }, // Nibble pattern
        { 0xF0F0F0F0, 16 }, // Inverse nibble pattern
        { 0x12345678, 13 }, // Random pattern
        { 0x7FFFFFFF, 31 }, // All bits set except MSB
    };

    for (const Case case_ : cases) 
    {
        const auto popCount = POP_COUNT32(case_.Value);
        EXPECT_EQ(popCount, case_.Expected);
    }
}

TEST(CompilerIntrinsics, PopCount64)
{
    struct Case
    {
        uint64 Value;
        int32 Expected;
    };

    const Case cases[] = {
        { 0x0000000000000000ULL, 0 },  // All bits zero
        { 0xFFFFFFFFFFFFFFFFULL, 64 }, // All bits set
        { 0x0000000000000001ULL, 1 },  // Single bit set at LSB
        { 0x8000000000000000ULL, 1 },  // Single bit set at MSB
        { 0xAAAAAAAAAAAAAAAAULL, 32 }, // Alternating bits (1010...)
        { 0x5555555555555555ULL, 32 }, // Alternating bits (0101...)
        { 0x0F0F0F0F0F0F0F0FULL, 32 }, // Nibble pattern
        { 0xF0F0F0F0F0F0F0F0ULL, 32 }, // Inverse nibble pattern
        { 0x123456789ABCDEF0ULL, 32 }, // Random pattern
        { 0x7FFFFFFFFFFFFFFFULL, 63 }, // All bits set except MSB
    };

    for (const Case case_ : cases)
    {
        const auto popCount = POP_COUNT64(case_.Value);
        EXPECT_EQ(popCount, case_.Expected);
    }
}


TEST(RangePuller, EndOnly)
{
    RangePuller<int32> puller{ 10 };

    int32 count = 0, sum = 0;
    for (; puller; ++puller)
    {
        ++count;
        sum += *puller;
    }

    GTEST_ASSERT_EQ(count, 10);
    GTEST_ASSERT_EQ(sum, 45);
}

TEST(RangePuller, StartEnd)
{
    RangePuller<int32> puller{ 5, 10 };
    int32 count = 0, sum = 0;
    for (; puller; ++puller)
    {
        ++count;
        sum += *puller;
    }

    GTEST_ASSERT_EQ(count, 5);
    GTEST_ASSERT_EQ(sum, (5 + 6 + 7 + 8 + 9));
}


namespace
{
    enum class ExampleEnum { A, B, C, };
}

TEST(EnumSet, BasicOperations)
{
    EnumSet<ExampleEnum> set;

    set.Add(ExampleEnum::A);
    EXPECT_TRUE (set.Contains(ExampleEnum::A));
    EXPECT_FALSE(set.Contains(ExampleEnum::B));

    set.Remove(ExampleEnum::A);
    EXPECT_FALSE(set.Contains(ExampleEnum::A));
    EXPECT_FALSE(set.Contains(ExampleEnum::B));

    EXPECT_TRUE(set.IsEmpty());
    set.Add(ExampleEnum::B);
    EXPECT_FALSE(set.IsEmpty());

    set.Clear();
    EXPECT_TRUE(set.IsEmpty());
}

TEST(EnumSet, CountElements)
{
    EnumSet<ExampleEnum> set;
    EXPECT_EQ(set.Count(), 0);

    set.Add(ExampleEnum::A).Add(ExampleEnum::B);
    EXPECT_EQ(set.Count(), 2);

    set.Remove(ExampleEnum::A);
    EXPECT_EQ(set.Count(), 1);

    set.Clear();
    EXPECT_EQ(set.Count(), 0);
}

TEST(EnumSet, BitwiseOperations)
{
    EnumSet<ExampleEnum> setA;
    EnumSet<ExampleEnum> setB;

    setA.Add(ExampleEnum::A).Add(ExampleEnum::B);
    setB.Add(ExampleEnum::B).Add(ExampleEnum::C);

    EnumSet<ExampleEnum> setOr = setA | setB;
    EXPECT_TRUE(setOr.Contains(ExampleEnum::A));
    EXPECT_TRUE(setOr.Contains(ExampleEnum::B));
    EXPECT_TRUE(setOr.Contains(ExampleEnum::C));

    EnumSet<ExampleEnum> setAnd = setA & setB;
    EXPECT_FALSE(setAnd.Contains(ExampleEnum::A));
    EXPECT_TRUE (setAnd.Contains(ExampleEnum::B));
    EXPECT_FALSE(setAnd.Contains(ExampleEnum::C));

    EnumSet<ExampleEnum> setXor = setA ^ setB;
    EXPECT_TRUE (setXor.Contains(ExampleEnum::A));
    EXPECT_FALSE(setXor.Contains(ExampleEnum::B));
    EXPECT_TRUE (setXor.Contains(ExampleEnum::C));
}

TEST(EnumSet, ComparisonOperations)
{
    EnumSet<ExampleEnum> setA;
    EnumSet<ExampleEnum> setB;

    setA.Add(ExampleEnum::A).Add(ExampleEnum::B);
    setB.Add(ExampleEnum::A).Add(ExampleEnum::B);

    EXPECT_TRUE(setA == setB);

    setB.Add(ExampleEnum::C);
    EXPECT_FALSE(setA == setB);
    EXPECT_TRUE (setA != setB);
    EXPECT_TRUE (setA <= setB);
    EXPECT_FALSE(setB <= setA);
}
