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
#include "Types/RcBox.h"


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
