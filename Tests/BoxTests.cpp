// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/BumpAlloc.h"
#include "Allocators/FixedAlloc.h"
#include "Debugging/LifecycleTracker.h"
#include "Types/Box.h"


TEST(Box, Init_Empty_ByDefaultConstructor)
{
    LIFECYCLE_TEST_INTO
    {
        Box<TestTracker> box;

        GTEST_ASSERT_TRUE  (box.IsEmpty());
        GTEST_ASSERT_FALSE (box.HasValue());

        // box.Get() // Illegal!
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(0)
}

TEST(Box, Init_Empty_ByFactoryMethod)
{
    LIFECYCLE_TEST_INTO
    {
        auto box = Box<TestTracker>::Empty();

        GTEST_ASSERT_TRUE  (box.IsEmpty());
        GTEST_ASSERT_FALSE (box.HasValue());

        // box.Get() // Illegal!
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(0)
}


TEST(Box, Init_Value_NoContext_ByFactorMethod)
{
    LIFECYCLE_TEST_INTO
    {
        auto box = Box<TestTracker>::Make(69);

        GTEST_ASSERT_FALSE (box.IsEmpty());
        GTEST_ASSERT_TRUE  (box.HasValue());
        GTEST_ASSERT_EQ    (box->Value, 69);
        GTEST_ASSERT_EQ    (box.Get()->Value, 69);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}

TEST(Box, Init_Value_WithContext_ByFactoryMethod)
{
    LIFECYCLE_TEST_INTO
    {
        constexpr int32 bufferSize = 4096;
        byte buffer[bufferSize];
        BumpAlloc::Context context{ buffer, bufferSize };

        GTEST_ASSERT_EQ    (context.FreeSpace(), bufferSize);

        auto box = Box<TestTracker, BumpAlloc>::MakeWithContext(context, 69);

        GTEST_ASSERT_FALSE (box.IsEmpty());
        GTEST_ASSERT_TRUE  (box.HasValue());
        GTEST_ASSERT_EQ    (box->Value, 69);
        GTEST_ASSERT_LE    (context.FreeSpace() - sizeof(TestTracker), bufferSize);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}


TEST(Box, Equality_Empty)
{
    LIFECYCLE_TEST_INTO
    {
        auto box1 = Box<TestTracker>::Empty();
        auto box2 = Box<TestTracker>::Empty();
        GTEST_ASSERT_EQ(box1, box2);
        GTEST_ASSERT_EQ(box2, box1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(0)
}

TEST(Box, Equality_Value)
{
    LIFECYCLE_TEST_INTO
    {
        auto box1 = Box<TestTracker>::Make(69);
        auto box2 = Box<TestTracker>::Make(69);
        GTEST_ASSERT_EQ(box1, box2);
        GTEST_ASSERT_EQ(box2, box1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2)
}


TEST(Box, Move)
{
    LIFECYCLE_TEST_INTO
    {
        auto box1 = Box<TestTracker>::Make(69);
        auto box2 = MOVE(box1);
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1) // Only one construction - Box is supposed to drag the value along.
}

TEST(Box, Assignment_NonEmpty)
{
    LIFECYCLE_TEST_INTO
    {
        auto box1 = Box<TestTracker>::Make(69);
        auto box2 = Box<TestTracker>::Make(96);
        box2 = MOVE(box1);

        GTEST_ASSERT_TRUE(box2.HasValue());
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(2)
}

TEST(Box, Assignment_Empty)
{
    LIFECYCLE_TEST_INTO
    {
        auto box1 = Box<TestTracker>::Make(69);
        auto box2 = Box<TestTracker>::Empty();
        box2 = MOVE(box1);

        GTEST_ASSERT_TRUE(box2.HasValue());
    }
    LIFECYCLE_TEST_OUT
    LIFECYCLE_TEST_DIFF(1)
}
