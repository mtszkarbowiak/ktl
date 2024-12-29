// Created by Mateusz Karbowiak 2024

#pragma once

#include "Allocators/HeapAlloc.h"
#include "Collections/AllocHelper.h"
#include "Language/Templates.h"
#include "Language/TypeInfo.h"
#include "Math/Arithmetic.h"
#include "Math/Growing.h"
#include "Math/HashingFunctions.h"
#include "Math/Probing.h"
#include "Types/IterHint.h"


/// <summary>
/// Default memory allocator. To be used when no specific allocator is required.
/// </summary>
using DefaultAlloc = HeapAlloc;


#ifndef ARRAY_DEFAULT_CAPACITY
    /// <summary> Default capacity for arrays. </summary>
    #define ARRAY_DEFAULT_CAPACITY 4
#endif

#ifndef HASH_MAPS_DEFAULT_CAPACITY
    /// <summary> Default capacity for any hash-based collection. </summary>
    #define HASH_MAPS_DEFAULT_CAPACITY 64
#endif

#ifndef HASH_MAPS_DEFAULT_SLACK_RATIO
    /// <summary> Default capacity for any hash-based collection. </summary>
#define HASH_MAPS_DEFAULT_SLACK_RATIO 3
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
    /// Helper object for searching for a bucket in a hash-based collection.
    /// -1 means the object was not found.
    /// </summary>
    struct BucketSearchResult final
    {
        int32 FoundObject;
        int32 FreeBucket;
    };

    /// <summary>
    /// Signifies stage of life of a bucket in a hash-based collection.
    /// </summary>
    enum class BucketState
    {
        /// <summary>
        /// The bucket is empty and can be used.
        /// </summary>
        Empty,

        /// <summary>
        /// The bucket is occupied and contains a valid key-value pair.
        /// </summary>
        Occupied,

        /// <summary>
        /// The bucket was occupied, but the key-value pair was deleted.
        /// </summary>
        /// <remarks> This value works as a tombstone. </remarks>
        Deleted,
    };

    // /// <summary>
    // /// Calculates the minimal legal capacity for given number of elements for the specified allocator.
    // /// </summary>
    // /// <remarks>
    // /// The allocator accepts only a specific range of capacities.
    // /// </remarks>
    // template<typename Element, typename Alloc, int32 Default>
    // FORCE_INLINE NODISCARD
    // static int32 GetRequiredCapacity(const int32 minCount)
    // {
    //     constexpr static int32 MinElements = Alloc::MinCapacity / sizeof(Element);
    //     constexpr static int32 MaxElements = Alloc::MaxCapacity / sizeof(Element);
    //     constexpr static int32 DefaultCapped = Math::Clamp(Default, MinElements, MaxElements);
    // 
    //     const int32 capacity = Math::Max<int32>(minCount, DefaultCapped);
    // 
    // 
    //     ASSERT(minCount <= capacity);   // The collection capacity must be at least as big as the requested count.
    //     return capacity;
    // }
    // 
    // template<typename Element, typename Alloc>
    // FORCE_INLINE NODISCARD
    // static auto AllocateCapacity(typename Alloc::Data& data, const int32 capacity) -> int32
    // {
    //     constexpr static int32 MinElements = Alloc::MinCapacity / sizeof(Element);
    //     constexpr static int32 MaxElements = Alloc::MaxCapacity / sizeof(Element);
    // 
    //     ASSERT(capacity >= MinElements); // Requested capacity is too low for the allocator.
    //     ASSERT(capacity <= MaxElements); // Requested capacity is too high for the allocator.
    // 
    //     const int32 requestedMemory = Math::NextPow2(capacity) * sizeof(Element);
    //     const int32 allocatedCapacity = data.Allocate(requestedMemory) / sizeof(Element);
    // 
    //     ASSERT(allocatedCapacity >= capacity);
    // 
    //     const Element* elements = DATA_OF(Element, data);
    //     ASSERT_CORRECT_ALIGNMENT(Element, elements);
    // 
    //     return allocatedCapacity;
    // }
};

class BulkOperations
{
private:
    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    DefaultLinearContentImpl(
        Element* elements,
        const int32 count
    )
    {
        for (int32 i = 0; i < count; ++i)
            new (elements + i) Element();
    }
    
    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    DefaultLinearContentImpl(
        Element* elements,
        const int32 count
    )
    {
        memset(elements, 0, count * sizeof(Element));
    }


    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    MoveLinearContentImpl(
        Element*    source,
        Element*    target,
        const int32 count
    )
    {
        for (int32 i = 0; i < count; ++i)
            new (target + i) Element(MOVE(source[i]));
    }

    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    MoveLinearContentImpl(
        Element*    source,
        Element*    target,
        const int32 count
    )
    {
        memcpy(target, source, count * sizeof(Element));

#if ZERO_MEMORY_FOR_CSTYLE
        memset(source, 0, count * sizeof(Element));
#endif
    }

    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    CopyLinearContentImpl(
        const Element* source,
        Element*       target,
        const int32    count
    )
    {
        for (int32 i = 0; i < count; ++i)
            new (target + i) Element(source[i]);
    }

    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    CopyLinearContentImpl(
        const Element* source,
        Element*       target,
        const int32    count
    )
    {
        memcpy(target, source, count * sizeof(Element));
    }


    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<!TIsCStyle<Element>::Value, void>
    DestroyLinearContentImpl(
        Element*    elements,
        const int32 count
    )
    {
        for (int32 i = 0; i < count; ++i)
            elements[i].~Element();
    }

    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    DestroyLinearContentImpl(
        Element*    elements,
        const int32 count
    )
    {
        // Pass.

#if ZERO_MEMORY_FOR_CSTYLE
        memset(elements, 0, count * sizeof(Element));
 #endif
    }


public:
    /// <summary>
    /// Default-construct the content of the target allocation.
    /// If necessary, objects lifetimes are managed. Otherwise, fast memory operations are used.
    /// </summary>
    template<typename Element>
    FORCE_INLINE
    static void DefaultLinearContent(
        Element*    elements,
        const int32 count
    )
    {
        DefaultLinearContentImpl<Element>(elements, count);
    }

    /// <summary>
    /// Moves to construct the content from the source allocation to the target allocation.
    /// If necessary, objects lifetimes are managed. Otherwise, fast memory operations are used.
    /// </summary>
    template<typename Element>
    FORCE_INLINE
    static void MoveLinearContent(
        Element*    source,
        Element*    target,
        const int32 count
    )
    {
        MoveLinearContentImpl<Element>(
            source, target, count
        );
    }

    /// <summary>
    /// Copies to construct the content from the source allocation to the target allocation.
    /// If necessary, objects lifetimes are managed. Otherwise, fast memory operations are used.
    /// </summary>
    /// <remarks>
    /// Elements are copied with respect to index.
    /// </remarks>
    template<typename Element>
    FORCE_INLINE
    static void CopyLinearContent(
        const Element* source,
        Element*       target,
        const int32    count
    )
    {
        CopyLinearContentImpl<Element>(
            source, target, count
        );
    }

    /// <summary>
    /// Destroys the content from the source allocation.
    /// If necessary, objects lifetimes are managed. Otherwise, fast memory operations are used.
    /// </summary>
    template<typename Element>
    FORCE_INLINE
    static void DestroyLinearContent(
        Element*    elements,
        const int32 count
    )
    {
        DestroyLinearContentImpl<Element>(
            elements, count
        );
    }
};
