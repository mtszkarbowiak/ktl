// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include <gtest/gtest.h>

#include "Allocators/FixedAlloc.h"
#include "Allocators/HeapAlloc.h"
#include "Allocators/BumpAlloc.h"
#include "Allocators/PolymorphicAlloc.h"
#include "Collections/Array.h"
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

TEST(PolymorphicAlloc, AllocationCycle)
{
    using TestInt                  = int64;
    constexpr int32 BufferCapacity = 32;
    constexpr int32 BufferMemory   = BufferCapacity * sizeof(TestInt);
    using TestPolymorphicAllocData = PolymorphicAlloc<FixedAlloc<BufferMemory>, HeapAlloc>::Data;

    // 0. Declare the polymorphic allocator.
    TestPolymorphicAllocData alloc;
    GTEST_ASSERT_TRUE(alloc.MovesItems()); // Empty polymorphic allocator should be able to move items, it's empty.

    // 1. Test the polymorphic allocator, using the main allocator - FixedAlloc.
    const int32 allocated = alloc.Allocate(BufferMemory);
    GTEST_ASSERT_GE(allocated, BufferMemory);
    GTEST_ASSERT_FALSE(alloc.MovesItems()); // If the main allocator is engaged, the polymorphic allocator should not be able to move items.
    alloc.Free();

    // 2. Test the polymorphic allocator, using the backup allocator - HeapAlloc.
    const int32 allocated2 = alloc.Allocate(BufferMemory * 2); // Ask for twice the memory to force the backup allocator.
    GTEST_ASSERT_GE(allocated2, BufferMemory * 2);
    GTEST_ASSERT_TRUE(alloc.MovesItems()); // If the backup allocator is engaged, the polymorphic allocator should be able to move items.
    alloc.Free();

    //TODO(mtszkarbowiak): Moving the polymorphic allocator.
}

TEST(PolymorphicAlloc, Array)
{
    using TestInt                  = int32;
    constexpr int32 BufferCapacity = 32;
    constexpr int32 BufferMemory   = BufferCapacity * sizeof(TestInt);
    using TestPolymorphicAlloc     = PolymorphicAlloc<FixedAlloc<BufferMemory>, HeapAlloc>;

    Array<TestInt, TestPolymorphicAlloc> array;
    for (TestInt i = 0; i < BufferCapacity; ++i)
    {
        array.Add(i);
    }
    GTEST_ASSERT_EQ(array.Count(), BufferCapacity);

    for (TestInt i = 0; i < BufferCapacity; ++i)
    {
        array.Add(i);
    }
    GTEST_ASSERT_EQ(array.Count(), BufferCapacity * 2);
}
