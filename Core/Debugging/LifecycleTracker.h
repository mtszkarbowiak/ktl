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

template<typename Tag>
struct LifecycleCountersHolder
{
    static LifecycleCounters Counters;
};

template <typename Tag>
LifecycleCounters LifecycleCountersHolder<Tag>::Counters;

template <class TheTag>
class LifecycleTracker
{
public:
    int32 Value;

    explicit LifecycleTracker(const int32 value = 0) noexcept
    {
        Value = value;

        auto& counters = LifecycleCountersHolder<TheTag>::Counters;
        counters.Instances     += 1;
        counters.Constructions += 1;
    }

    LifecycleTracker(const LifecycleTracker& other) noexcept
    {
        Value = other.Value;

        auto& counters = LifecycleCountersHolder<TheTag>::Counters;
        counters.Instances     += 1;
        counters.Constructions += 1;
        counters.Copies        += 1;
    }

    LifecycleTracker(LifecycleTracker&& other) noexcept
    {
        Value = other.Value;

        auto& counters = LifecycleCountersHolder<TheTag>::Counters;
        counters.Instances     += 1;
        counters.Constructions += 1;
        counters.Moves         += 1;
    }

    ~LifecycleTracker()
    {
        auto& counters = LifecycleCountersHolder<TheTag>::Counters;
        counters.Instances -= 1;
        counters.Destructions += 1;
    }


    auto operator=(const LifecycleTracker&) noexcept -> LifecycleTracker&
    {
        auto& counters = LifecycleCountersHolder<TheTag>::Counters;
        counters.Copies += 1;
        return *this;
    }

    auto operator=(LifecycleTracker&& other) noexcept -> LifecycleTracker&
    {
        Value = other.Value;

        auto& counters = LifecycleCountersHolder<TheTag>::Counters;
        counters.Moves += 1;
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


// /!\ Very hacky macros ahead! /!\
//
// Their underlying objective is to create a lifecycle tracking scope for a given test case.
// This is difficult, because each tracker requires a shared state between other trackers.
// They fight with each other for the access to that shared state. One of the solutions would
// be to dispatch the tests synchronously, but that would be a huge performance hit.
// Instead, completely different approach is taken:
//
// In Google's 'gtest' each test is its own class. This means that each class creates a scope
// we can assign shared state to. This is exactly what the macros do - they create a class
// per test case and use tag dispatching to assign the shared state to the tracker class.
// This way, each test case has its own shared state and the trackers don't fight with each other.
// Unfortunately, this approach uses one very strong assumption: That the test cases are not run
// in parallel. This is a very strong assumption, but it is the only way to make this work.
//
// Note: Using a reference to the shared state would be a better solution, but it would require:
// a) external linkage b) would suffer the same lack of thread safety.


#define LIFECYCLE_TEST_INTO \
    using ThisType    = std::remove_reference_t<decltype(*this)>; \
    using TestTracker = LifecycleTracker<ThisType>; \
    auto& LifecycleCountersInstance = LifecycleCountersHolder<ThisType>::Counters; \
    const int32 instancesBefore     = LifecycleCountersInstance.Instances; \
    const int32 constructionsBefore = LifecycleCountersInstance.Constructions;

#define LIFECYCLE_TEST_OUT \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Instances,     instancesBefore); \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Constructions, LifecycleCountersInstance.Destructions);

#define LIFECYCLE_TEST_DIFF(instancesDiff) \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Constructions, constructionsBefore + (instancesDiff)); \
    GTEST_ASSERT_EQ(LifecycleCountersInstance.Destructions,  constructionsBefore + (instancesDiff));
