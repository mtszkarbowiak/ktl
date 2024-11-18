// Created by Mateusz Karbowiak 2024

#pragma once

#include "../Allocators/HeapAlloc.h"
#include "../Language/Templates.h"
#include "../Math/Arithmetic.h"


#ifndef ARRAY_DEFAULT_CAPACITY
    /// <summary> Default capacity for arrays. </summary>
    #define ARRAY_DEFAULT_CAPACITY 4
#endif

#ifndef HASH_MAPS_DEFAULT_CAPACITY
    /// <summary> Default capacity for any hash-based collection. </summary>
    #define HASH_MAPS_DEFAULT_CAPACITY 64
#endif

#ifndef RING_DEFAULT_CAPACITY
    /// <summary> Default capacity for any ring collection. </summary>
    #define RING_DEFAULT_CAPACITY 16
#endif


#ifndef ZERO_MEMORY_FOR_CSTYLE
    /// <summary> Enables setting the memory to zero after a move with raw memory operation. </summary>
    /// <remarks> This is useful for security reasons, but creates a performance overhead. </remarks>
    #define ZERO_MEMORY_FOR_CSTYLE 0
#endif


class CollectionsUtils
{
public:
    /// <summary>
    /// Calculates allocation size for the given number of elements.
    /// The operation takes into account the size of the element and the limits of the allocator.
    /// </summary>
    /// <remarks>
    /// Remember that this function calculates number of elements, not bytes!
    /// </remarks>
    template<typename Alloc, int32 Default>
    FORCE_INLINE NODISCARD
    static auto GetAllocCapacity(const int32 minCapacity) -> int32
    {
        ASSERT_INDEX(minCapacity > 0); // Allocating 0 elements is not allowed.
        ASSERT_INDEX(minCapacity <= Alloc::MaxCapacity); // Requested capacity is too high for the allocator.

        // First, calculate default capacity but with respect to the allocator limits.
        constexpr static int32 CappedDefault = Math::Clamp<int32>(
            Default, Alloc::MinCapacity, Alloc::MaxCapacity
        );

        return Math::Max<int32>(minCapacity, CappedDefault);
    }

private:
    template<typename Element, typename SourceAllocation, typename TargetAllocation>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    MoveLinearContentImpl(
        typename SourceAllocation::Data& sourceData,
        typename TargetAllocation::Data& targetData,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        Element* source = static_cast<Element*>(sourceData.Get());
        Element* target = static_cast<Element*>(targetData.Get());

        for (int32 i = beginIndex; i < endIndex; ++i)
        {
            new (target + i) Element(MOVE(source[i]));
            source[i].~Element();
        }
    }

    template<typename Element, typename SourceAllocation, typename TargetAllocation>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    MoveLinearContentImpl(
        typename SourceAllocation::Data& sourceData,
        typename TargetAllocation::Data& targetData,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        Element* source = static_cast<Element*>(sourceData.Get());
        Element* target = static_cast<Element*>(targetData.Get());

        const int32 mem = (endIndex - beginIndex) * sizeof(Element);

        memcpy(
            target + beginIndex, 
            source + beginIndex, 
            mem
        );

#if ZERO_MEMORY_FOR_CSTYLE
        memset(source + beginIndex, 0, mem);
#endif
    }

    template<typename Element, typename SourceAllocation, typename TargetAllocation>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    CopyLinearContentImpl(
        const typename SourceAllocation::Data& sourceData,
        typename TargetAllocation::Data& targetData,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        const Element* source = reinterpret_cast<const Element*>(sourceData.Get());
        Element*       target = reinterpret_cast<      Element*>(targetData.Get());

        for (int32 i = beginIndex; i < endIndex; ++i)
        {
            new (target + i) Element(source[i]);
        }
    }

    template<typename Element, typename SourceAllocation, typename TargetAllocation>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    CopyLinearConentImpl(
        const typename SourceAllocation::Data& sourceData,
        typename TargetAllocation::Data& targetData,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        const Element* source = static_cast<const Element*>(sourceData.Get());
        Element*       target = static_cast<      Element*>(targetData.Get());

        const int32 mem = (endIndex - beginIndex) * sizeof(Element);

        memcpy(
            target + beginIndex,
            source + beginIndex,
            mem
        );
    }


    template<typename Element, typename Allocation>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    DestroyLinearContentImpl(
        typename Allocation::Data& data,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        Element* elements = static_cast<Element*>(data.Get());
        for (int32 i = beginIndex; i < endIndex; ++i)
        {
            elements[i].~Element();
        }
    }

    template<typename Element, typename Allocation>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    DestroyLinearContentImpl(
        typename Allocation::Data& data,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        // Pass.

#if ZERO_MEMORY_FOR_CSTYLE
        Element* elements = static_cast<Element*>(data.Get());
        const int32 mem = (endIndex - beginIndex) * sizeof(Element);
        memset(elements + beginIndex, 0, mem);
 #endif
    }


public:
    /// <summary>
    /// Moves the content from the source allocation to the target allocation.
    /// If necessary, objects lifetimes are managed. Otherwise, fast memory operations are used.
    /// </summary>
    template<typename Element, typename SourceAllocation, typename TargetAllocation>
    FORCE_INLINE
    static void MoveLinearContent(
        typename SourceAllocation::Data& sourceData,
        typename TargetAllocation::Data& targetData,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        static_assert(std::is_class<SourceAllocation>::value, "Allocation must be a class!");
        static_assert(std::is_class<TargetAllocation>::value, "Allocation must be a class!");
        static_assert(std::is_class<typename SourceAllocation::Data>::value, "Allocation::Data must be a class!");
        static_assert(std::is_class<typename TargetAllocation::Data>::value, "Allocation::Data must be a class!");

        MoveLinearContentImpl<Element, SourceAllocation, TargetAllocation>(
            sourceData, targetData, endIndex, beginIndex
        );
    }

    /// <summary>
    /// Copies the content from the source allocation to the target allocation.
    /// If necessary, objects lifetimes are managed. Otherwise, fast memory operations are used.
    /// </summary>
    /// <remarks>
    /// Elements are copied with respect to index.
    /// </remarks>
    template<typename Element, typename SourceAllocation, typename TargetAllocation>
    FORCE_INLINE
    static void CopyLinearContent(
        const typename SourceAllocation::Data& sourceData,
        typename TargetAllocation::Data& targetData,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        static_assert(std::is_class<SourceAllocation>::value, "Allocation must be a class!");
        static_assert(std::is_class<TargetAllocation>::value, "Allocation must be a class!");
        static_assert(std::is_class<typename SourceAllocation::Data>::value, "Allocation::Data must be a class!");
        static_assert(std::is_class<typename TargetAllocation::Data>::value, "Allocation::Data must be a class!");

        CopyLinearContentImpl<Element, SourceAllocation, TargetAllocation>(
            sourceData, targetData, endIndex, beginIndex
        );
    }

    template<typename Element, typename Allocation>
    FORCE_INLINE
    static void DestroyLinearContent(
        typename Allocation::Data& data,
        const int32 endIndex,
        const int32 beginIndex = 0
    )
    {
        static_assert(std::is_class<Allocation>::value, "Allocation must be a class!");
        static_assert(std::is_class<typename Allocation::Data>::value, "Allocation::Data must be a class!");

        DestroyLinearContentImpl<Element, Allocation>(data, endIndex, beginIndex);
    }
};
