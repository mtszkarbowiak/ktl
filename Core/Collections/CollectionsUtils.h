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

/// <summary> Returns the pointer to the data of the specified type. </summary>
#define DATA_OF(element_type, alloc) static_cast<element_type*>((alloc).Get())

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
        {
            new (target + i) Element(MOVE(source[i]));
            source[i].~Element();
        }
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
        {
            new (target + i) Element(source[i]);
        }
    }

    template<typename Element>
    FORCE_INLINE
    static std::enable_if_t<TIsCStyle<Element>::Value, void>
    CopyLinearConentImpl(
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
    /// Moves the content from the source allocation to the target allocation.
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
    /// Copies the content from the source allocation to the target allocation.
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
