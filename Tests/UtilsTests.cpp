// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Language/Templates.h"
#include "Types/Numbers.h"


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

TEST(TypeUtils, CopyAbleDispatch)
{
    int32 result;

    struct
    {
        int32* _theResult;

        void Check(IsCopyableTag)
        {
            *_theResult = 1;
        }

        void Check(NonCopyableTag)
        {
            *_theResult = 2;
        }
    }
    tester{ &result };

    struct CopyableExample
    {
        CopyableExample(const CopyableExample&) = default;
    };

    struct NotCopyableExample
    {
        NotCopyableExample(const NotCopyableExample&) = delete;
    };

    using Tag1 = GetCopyableTag<CopyableExample>;
    using Tag2 = GetCopyableTag<NotCopyableExample>;

    tester.Check(Tag1{});
    GTEST_ASSERT_EQ(result, 1);

    tester.Check(Tag2{});
    GTEST_ASSERT_EQ(result, 2);
}
