// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Collections/Array.h"
#include "Debugging/LifecycleTracker.h"
#include "Math/Stastics.h"

TEST(Stastics, Rss)
{
    const auto set1 = Array<float>::Of({ 1.0f, 2.0f, 3.0f, 4.0f, 5.0f });
    const auto set2 = Array<float>::Of({ 1.0f, 2.0f, 3.0f, 4.0f, 6.0f });

    const auto rss = Statistics::Rss(
        set1.Enumerate(),
        set2.Enumerate()
    );

    EXPECT_FLOAT_EQ(rss, 1.0f);
}
