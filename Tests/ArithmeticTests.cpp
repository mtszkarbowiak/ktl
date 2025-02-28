// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Math/Arithmetic.h"
#include "Types/Numbers.h"

TEST(Arithmetic, Min)
{
    EXPECT_EQ(Math::Min<int32>(1, 2), 1);
    EXPECT_EQ(Math::Min<int32>(2, 1), 1);
    EXPECT_EQ(Math::Min<int32>(1, 1), 1);
}

TEST(Arithmetic, Max)
{
    EXPECT_EQ(Math::Max<int32>(1, 2), 2);
    EXPECT_EQ(Math::Max<int32>(2, 1), 2);
    EXPECT_EQ(Math::Max<int32>(1, 1), 1);
}

TEST(Arithmetic, Clamp)
{
    EXPECT_EQ(Math::Clamp<int32>(1, 2, 3), 2);
    EXPECT_EQ(Math::Clamp<int32>(2, 2, 3), 2);
    EXPECT_EQ(Math::Clamp<int32>(3, 2, 3), 3);
    EXPECT_EQ(Math::Clamp<int32>(1, 1, 3), 1);
    EXPECT_EQ(Math::Clamp<int32>(2, 1, 3), 2);
    EXPECT_EQ(Math::Clamp<int32>(3, 1, 3), 3);
    EXPECT_EQ(Math::Clamp<int32>(1, 1, 1), 1);
}

TEST(Arithmetic, Abs)
{
    EXPECT_EQ(Math::Abs<int32>(1), 1);
    EXPECT_EQ(Math::Abs<int32>(-1), 1);
    EXPECT_EQ(Math::Abs<int32>(0), 0);
}
