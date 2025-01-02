// Created by Mateusz Karbowiak 2024

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Allocators/HeapAlloc.h"
#include "Allocators/BumpAlloc.h"
#include "Math/Arithmetic.h"

TEST(CapacityMath, Pow2)
{
    using namespace Math;

    GTEST_ASSERT_EQ(NextPow2(1), 1);
    GTEST_ASSERT_EQ(NextPow2(2137), 4096);
    GTEST_ASSERT_EQ(NextPow2(4096), 4096);

    GTEST_ASSERT_EQ(PrevPow2(1), 1);
    GTEST_ASSERT_EQ(PrevPow2(2137), 2048);
    GTEST_ASSERT_EQ(PrevPow2(4096), 4096);

    GTEST_ASSERT_TRUE(IsPow2(1));
    GTEST_ASSERT_TRUE(IsPow2(2));
    GTEST_ASSERT_TRUE(IsPow2(1024));

    GTEST_ASSERT_FALSE(IsPow2(3));
    GTEST_ASSERT_FALSE(IsPow2(2137));
}

TEST(FixedAlloc, AllocationCycle)
{
    using TestInt                  = int64;
    constexpr int32 BufferCapacity = 128;
    constexpr int32 BufferMemory   = BufferCapacity * sizeof(TestInt);
    using FixedAllocData = FixedAlloc<BufferMemory>::Data;

    // FixedAlloc isn't very interesting, but it's a good example of basic allocators usage.

    FixedAllocData alloc;
    const int32 allocated = alloc.Allocate(BufferMemory); // Allocation must not have different size than the buffer for fixed alloc.
    GTEST_ASSERT_GE(allocated, BufferMemory);
    GTEST_ASSERT_FALSE(alloc.MovesItems());     
    void* ptr = alloc.Get();
    alloc.Free();
}

TEST(HeapAlloc, AllocationCycle)
{
    using TestInt       = int64;
    using HeapAllocData = HeapAlloc::Data;

    // HeapAlloc is a bit more interesting, but still not very useful to learn.

    HeapAllocData alloc;
    GTEST_ASSERT_EQ(alloc.Get(), nullptr);
    constexpr int32 requested = 3 * sizeof(TestInt);
    const int32 allocated = alloc.Allocate(requested);
    GTEST_ASSERT_GE(allocated, requested);
    GTEST_ASSERT_NE(alloc.Get(), nullptr);    
    void* ptr = alloc.Get();
    GTEST_ASSERT_TRUE(alloc.MovesItems());
    alloc.Free();
    GTEST_ASSERT_EQ(alloc.Get(), nullptr);
}

TEST(BumpAlloc, AllocationCycle)
{
    using TestInt          = int64;
    using BumpAllocData    = BumpAlloc::Data;
    using BumpAllocContext = BumpAlloc::Context;

    // BumpAlloc has more complexity, but it's still a simple allocator.
    // It is the simplest context-based allocator.

    byte buffer[4096];
    BumpAllocContext context{ buffer, sizeof(buffer) };
    {
        BumpAllocData alloc{ context };
        GTEST_ASSERT_EQ(alloc.Get(), nullptr);
        constexpr int32 requested = 3 * sizeof(TestInt);
        const int32 allocated = alloc.Allocate(requested);
        GTEST_ASSERT_GE(allocated, requested);
        GTEST_ASSERT_NE(alloc.Get(), nullptr);   
        void* ptr = alloc.Get();
        GTEST_ASSERT_TRUE(alloc.MovesItems());   
        alloc.Free();                            
        GTEST_ASSERT_EQ(alloc.Get(), nullptr);
    }
    context.Reset();
}
