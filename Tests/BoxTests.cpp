// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Types/Box.h"
#include "Allocators/BumpAlloc.h"

TEST(Box, Init_Empty_ByDefaultConstructor)
{
    Box<int32> box;
    GTEST_ASSERT_TRUE(box.IsEmpty());
}

TEST(Box, Init_Empty_ByFactoryMethod)
{
    auto box = Box<int32>::Empty();
    GTEST_ASSERT_TRUE(box.IsEmpty());
}

TEST(Box, Init_Value_NoContext_ByFactorMethod)
{
    auto box = Box<int32>::Make(69);
    GTEST_ASSERT_FALSE(box.IsEmpty());
    GTEST_ASSERT_TRUE(box.HasValue());
    GTEST_ASSERT_EQ(*box, 69);
}

TEST(Box, Init_Value_WithContext_ByFactoryMethod)
{
    byte buffer[4096];
    BumpAlloc::Context context{ buffer, 4096 };
    auto box = Box<int32, BumpAlloc>::MakeWithContext(context, 69);
    GTEST_ASSERT_FALSE(box.IsEmpty());
    GTEST_ASSERT_TRUE(box.HasValue());
    GTEST_ASSERT_EQ(*box, 69);
}
