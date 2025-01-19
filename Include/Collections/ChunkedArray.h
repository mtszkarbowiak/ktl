// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"
#include "Allocators/FixedAlloc.h"

/// <summary>
/// A container for dynamically resizable arrays of elements,
/// stored in multiple contiguous blocks of memory, called chunks.
/// </summary>
/// 
/// <typeparam name="T">
/// The type of elements stored in the array.
/// Must be movable (both constructor and assignment), non-const, and non-reference.
/// </typeparam>
/// <typeparam name="CC">
/// (Optional) The capacity of a single chunk.
/// </typeparam>
/// <typeparam name="TA">
/// (Optional) The type of the allocator to use to allocate elements (chunks).
/// Can be either a dragging or non-dragging allocator.
/// </typeparam>
/// <typeparam name="MA">
/// (Optional) The type of the allocator to use to allocate meta-data,
/// it is data of allocations of chunks.
/// Can be either a dragging or non-dragging allocator.
/// </typeparam>
/// <typeparam name="MG">
/// (Optional) A reference to a function that calculates the next capacity
/// before applying allocator limits.
/// </typeparam>
template<
    typename T,
    int32    CC = 256,
    typename CA = FixedAlloc<CC * sizeof(T)>,
    typename MA = HeapAlloc,
    typename MG = DefaultGrowth
>
class ChunkedArray
{
public:
    static constexpr int32 ChunkCapacity = CC;

    using Element          = T;
    using ChunkAllocData   = typename CA::Data;
    using MetaAllocData    = typename MA::Data;
    using ChunkAllocHelper = AllocHelperOf<Element,        CA, CC, MG>;
    using MetaAllocHelper  = AllocHelperOf<ChunkAllocData, MA, 1,  MG>;

PRIVATE:
    MetaAllocData _metaAllocData{};  // Allocation data for meta-data
    int32         _capacityChunks{}; // Number of allocated (and initialized) chunks
    int32         _countElements{};  // Number of elements in the array (not the number of chunks)


    // Capacity Access

public:
    /// <summary> Checks if the chunked array has an active allocation for chunks metadata. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsAllocated() const -> bool
    {
        return _capacityChunks > 0;
    }
    /// <summary> Number of elements that can be stored without invoking any of the allocators. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Capacity() const -> int32
    {
        return _capacityChunks * ChunkCapacity;
    }


    // Count Access

    /// <summary> Checks if the chunked array has any elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsEmpty() const -> bool
    {
        return _countElements == 0;
    }

    /// <summary> Number of currently stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Count() const -> int32
    {
        return _countElements;
    }

    /// <summary> Number of elements that can be added without invoking any of the allocators. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Slack() const -> int32
    {
        return Capacity() - _countElements;
    }


    // Allocation Manipulation

    //TODO(mtszkarbowiak): Implement the ChunkedArray class


    // Element Access

PRIVATE:
    /// <summary>
    /// Accesses the first chunk metadata of the chunked array.
    /// </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto MetaData() -> ChunkAllocData*
    {
        return DATA_OF(ChunkAllocData, _metaAllocData);
    }

    /// <summary>
    /// Accesses the first chunk metadata of the chunked array.
    /// </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto MetaData() const -> const ChunkAllocData*
    {
        return DATA_OF(const ChunkAllocData, _metaAllocData);
    }


    //TODO(mtszkarbowiak): Implement the ChunkedArray class


    // Constraints

    REQUIRE_TYPE_MOVEABLE_NOEXCEPT(Element);
    REQUIRE_TYPE_NOT_REFERENCE(Element);
    REQUIRE_TYPE_NOT_CONST(Element);

    REQUIRE_TYPE_MOVEABLE_NOEXCEPT(ChunkAllocData);
    REQUIRE_TYPE_NOT_REFERENCE(ChunkAllocData);
    REQUIRE_TYPE_NOT_CONST(ChunkAllocData);
};
