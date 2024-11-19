// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

struct LifecycleCounters
{
    int32_t Instances = 0;
    int32_t Constructions = 0;
    int32_t Destructions = 0;
    int32_t Moves = 0;
    int32_t Copies = 0;
};

template<LifecycleCounters& Counters>
class LifecycleTracker
{
public:
    int32 Value;

    explicit LifecycleTracker(const int32 value = 0) noexcept
    {
        Value = value;

        Counters.Instances     += 1;
        Counters.Constructions += 1;
    }

    LifecycleTracker(const LifecycleTracker& other) noexcept
    {
        Value = other.Value;

        Counters.Instances     += 1;
        Counters.Constructions += 1;
        Counters.Copies        += 1;
    }

    LifecycleTracker(LifecycleTracker&& other) noexcept
    {
        Value = other.Value;

        Counters.Instances     += 1;
        Counters.Constructions += 1;
        Counters.Moves         += 1;
    }

    ~LifecycleTracker()
    {
        Counters.Instances -= 1;
        Counters.Destructions += 1;
    }


    auto operator=(const LifecycleTracker&) noexcept -> LifecycleTracker&
    {
        Counters.Copies += 1;
        return *this;
    }

    auto operator=(LifecycleTracker&&) noexcept -> LifecycleTracker&
    {
        Counters.Moves += 1;
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

#define LIFECYCLE_TEST_INTO \
    static LifecycleCounters LifecycleCountersInstance; \
    using LifecycleTracker = LifecycleTracker<LifecycleCountersInstance>; \
    const int32 instancesBefore     = LifecycleCountersInstance.Instances; \
    const int32 constructionsBefore = LifecycleCountersInstance.Constructions;

#define LIFECYCLE_TEST_OUT \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Instances,     instancesBefore); \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Constructions, LifecycleCountersInstance.Destructions);

#define LIFECYCLE_TEST_DIFF(instancesDiff) \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Constructions, constructionsBefore + (instancesDiff)); \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Destructions,  constructionsBefore + (instancesDiff));
