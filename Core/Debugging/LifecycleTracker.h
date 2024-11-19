// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

class LifecycleTracker
{
public:
    static int32 Instances;
    static int32 Constructions;
    static int32 Destructions;

    static int32 Moves;
    static int32 Copies;

    int32 Value;

    explicit LifecycleTracker(const int32 value = 0) noexcept
    {
        Value = value;

        Instances     += 1;
        Constructions += 1;
    }

    LifecycleTracker(const LifecycleTracker& other) noexcept
    {
        Value = other.Value;

        Instances     += 1;
        Constructions += 1;
        Copies        += 1;
    }

    LifecycleTracker(LifecycleTracker&& other) noexcept
    {
        Value = other.Value;

        Instances     += 1;
        Constructions += 1;
        Moves         += 1;
    }

    ~LifecycleTracker()
    {
        Instances -= 1;
        Destructions += 1;
    }


    auto operator=(const LifecycleTracker&) noexcept -> LifecycleTracker&
    {
        Copies += 1;
        return *this;
    }

    auto operator=(LifecycleTracker&&) noexcept -> LifecycleTracker&
    {
        Moves += 1;
        return *this;
    }


    auto operator==(const LifecycleTracker& other) const noexcept -> bool
    {
        return Value == other.Value;
    }

    auto operator!=(const LifecycleTracker& other) const noexcept -> bool
    {
        return Value != other.Value;
    }
};

#define LIFECYCLE_TEST_TRACK \
    int32 LifecycleTracker::Instances     = 0; \
    int32 LifecycleTracker::Constructions = 0; \
    int32 LifecycleTracker::Destructions  = 0; \
    int32 LifecycleTracker::Moves         = 0; \
    int32 LifecycleTracker::Copies        = 0;

#define LIFECYCLE_TEST_INTO \
    const int32 instancesBefore     = LifecycleTracker::Instances; \
    const int32 constructionsBefore = LifecycleTracker::Constructions;

#define LIFECYCLE_TEST_OUT \
    GTEST_ASSERT_EQ(LifecycleTracker::Instances,     instancesBefore); \
    GTEST_ASSERT_EQ(LifecycleTracker::Constructions, LifecycleTracker::Destructions);

#define LIFECYCLE_TEST_DIFF(instancesDiff) \
    GTEST_ASSERT_EQ(LifecycleTracker::Constructions, constructionsBefore + instancesDiff); \
    GTEST_ASSERT_EQ(LifecycleTracker::Destructions,  constructionsBefore + instancesDiff);
