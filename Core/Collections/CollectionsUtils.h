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
#include "Types/Span.h"


/// <summary>
/// Default memory allocator. To be used when no specific allocator is required.
/// </summary>
using DefaultAlloc = HeapAlloc;


#ifndef ARRAY_DEFAULT_CAPACITY
    /// <summary> Default capacity for arrays. </summary>
    #define ARRAY_DEFAULT_CAPACITY 4
#endif

#ifndef HASH_SETS_DEFAULT_CAPACITY
    /// <summary> Default capacity for any hash-based collection. </summary>
    #define HASH_SETS_DEFAULT_CAPACITY 64
#endif

#ifndef HASH_SETS_DEFAULT_SLACK_RATIO
    /// <summary> Default capacity for any hash-based collection. </summary>
    #define HASH_SETS_DEFAULT_SLACK_RATIO 3
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



namespace Bucketing
{
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
        MEMSET(elements, 0, count * sizeof(Element));
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
        MEMCPY(target, source, count * sizeof(Element));

#if ZERO_MEMORY_FOR_CSTYLE
        MEMSET(source, 0, count * sizeof(Element));
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
        MEMCPY(target, source, count * sizeof(Element));
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
        MEMSET(elements, 0, count * sizeof(Element));
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
        ASSERT(count >= 0);
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
        ASSERT(count >= 0);
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
        ASSERT(count >= 0);
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
        ASSERT(count >= 0);
        DestroyLinearContentImpl<Element>(
            elements, count
        );
    }
};
