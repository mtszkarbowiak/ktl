// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Math/Arithmetic.h"


/// <summary> Returns the pointer to the data of the specified type. </summary>
/// <remarks> In the future, this macro may also be used to add additional checks or operations. </remarks>
#define DATA_OF(element_type, alloc) reinterpret_cast<element_type*>((alloc).Get())

/// <summary> Makes sure that the pointer is correctly aligned for the specified type. </summary>
#define ASSERT_CORRECT_ALIGNMENT(type, ptr) ASSERT(reinterpret_cast<uintptr_t>(ptr) % alignof(type) == 0)


/// <summary>
/// Utility class to help manage the state of allocation in a collection.
/// This classes uses the type of the element, the allocator, and the growth policy
/// to manage the allocation of the memory for the collection.
/// </summary>
/// <remarks>
/// Collection takes care of tracking the capacity of the allocated memory.
/// Collections have shared logic for managing the allocation, but the allocator
/// is responsible for the actual allocation and deallocation. Thus this class
/// helps to keep the collection and allocator in sync.
/// </remarks>
template<
    typename Element, 
    typename Alloc, 
    int32 DefaultUncapped,
    int32(&Grow)(int32)
>
class AllocHelperOf final
{
public:
    /// <summary> Binding between the allocator and the collection's context. </summary>
    using AllocData = typename Alloc::Data;

    /// <summary>
    /// Minimal capacity for the allocator. A collection can have a lower capacity,
    /// but the allocator can never yield smaller number of elements then this value.
    /// (Zero is not a valid capacity, but can be used as a sentinel value 
    /// to indicate that no allocation is active.)
    /// </summary>
    static constexpr int32 MinElements = Alloc::MinCapacity / sizeof(Element);

    /// <summary>
    /// Maximal capacity for the allocator. A collection can never have a higher capacity.
    /// Notably, collection may violate the growth policy if it exceeds this value.
    /// </summary>
    static constexpr int32 MaxElements = Alloc::MaxCapacity / sizeof(Element);

    /// <summary>
    /// Maximal capacity for the collection. A collection may never have a lower capacity.
    /// (Zero is not a valid capacity, but can be used as a sentinel value
    /// to indicate that no allocation is active.)
    /// </summary>
    static constexpr int32 DefaultElements = Math::Clamp(DefaultUncapped, MinElements, MaxElements);


    /// <summary>
    /// Calculates the next capacity for empty collection.
    /// If there is no more capacity assertion will be triggered.
    /// </summary>
    NO_DISCARD static
    auto InitCapacity(const int32 minCapacity) -> int32
    {
        const int32 requiredCapacity = Math::Max(minCapacity, DefaultElements);
        ASSERT_ALLOCATOR_SAFETY(requiredCapacity <= MaxElements);
        return requiredCapacity;
    }

    /// <summary>
    /// Calculates the next capacity for non-empty collection.
    /// If there is no more capacity assertion will be triggered.
    /// </summary>
    /// <param name="oldCapacity"> Current capacity of the collection. </param>
    /// <param name="minCapacity"> Minimal capacity that the collection should have. </param>
    /// <returns> The new capacity, which the collection should request from the allocator. </returns>
    NO_DISCARD static
    auto NextCapacity(const int32 oldCapacity, const int32 minCapacity) -> int32
    {
        // This method returns next capacity and assumes there already is an active allocation.
        // Values lower than the default capacity mean that the collection is empty (or corrupted).
        ASSERT_ALLOCATOR_SAFETY(oldCapacity >= DefaultElements);
        // If the collection is empty, just use the default capacity.
        // You will use only one branch (in the collection) to allocate the memory, instead of two.

        // If the requested capacity is lower than the current capacity, just return the current capacity.
        if (minCapacity <= oldCapacity)
            return oldCapacity;

        int32 newCapacity = oldCapacity;

        // Keep applying the growth policy until the capacity is sufficient.
        while (newCapacity < minCapacity)
            newCapacity = Grow(newCapacity);

        // Ultimately, cap the capacity to the maximum allowed by the allocator.
        newCapacity = Math::Min(newCapacity, MaxElements);

        // Make sure the capacity is at least as big as the requested minimal capacity.
        // This is a safety check to ensure that the collection can store the requested number of elements.
        ASSERT_ALLOCATOR_SAFETY(newCapacity >= minCapacity);
        // If the capacity is not at least as big as the requested minimal capacity,
        // the collection will not be able to store the requested number of elements.

        return newCapacity;
    }

    /// <summary>
    /// Allocates the memory for the collection.
    /// </summary>
    /// <param name="alloc"> The allocation data to be used. Warning: It must not have active allocation prior to this call! </param>
    /// <param name="capacity"> The capacity to allocate. </param>
    /// <returns>
    /// The number of elements that can be stored in the allocated memory.
    /// It is guaranteed that the allocated capacity is at least as big as the requested capacity.
    /// </returns>
    NO_DISCARD static
    auto Allocate(AllocData& alloc, const int32 capacity) -> int32
    {
        // Calculate the required memory size.
        const int32 requestedMemory = capacity * sizeof(Element);
        // Allocate the memory.
        const int32 allocatedMemory = alloc.Allocate(requestedMemory);
        // (Allocator itself should assert that the requested memory is in the valid range.)

        // Calculate the number of elements that can be stored in the allocated memory.
        const int32 allocatedCapacity = allocatedMemory / sizeof(Element);

        // Make sure the allocated capacity is at least as big as the requested capacity.
        // This is a safety check to ensure that the collection can store the requested number of elements.
        // (Allocator could return 0 if the allocation failed.)
        ASSERT_ALLOCATOR_SAFETY(allocatedCapacity >= capacity);

        // If the allocated capacity is not at least as big as the requested capacity,
        // the collection will not be able to store the requested number of elements.
        // Get the pointer to the allocated memory.
        const Element* elements = DATA_OF(Element, alloc);
        ASSERT_ALLOCATOR_SAFETY(elements);
        // Make sure the pointer is correctly aligned.
        ASSERT_CORRECT_ALIGNMENT(Element, elements);

        return allocatedCapacity;
    }


    /// <summary>
    /// Some collections have very strict requirement of the capacity being always a power of 2.
    /// To keep the functions above valid, the allocator must use only powers of 2, including the limits.
    /// This ensures that capacities undergoing the clamp operation will not violate the collection's constraints.
    /// </summary>
    NO_DISCARD static constexpr
    auto HasBinaryMaskingSupport() -> bool
    {
        constexpr bool correctMinimum = Math::IsPow2(MinElements) || (Alloc::MinCapacity < 2);
        constexpr bool correctMaximum = Math::IsPow2(MaxElements) || (Alloc::MaxCapacity == INT32_MAX);
        return correctMinimum && correctMaximum;

        // The underlying problem is that after applying the growth function, we should ceil the result
        // to the power of two. This is to keep the constraint of pow-2 capacity. Unfortunately,
        // the capacity may be capped by the allocator. Therefore, we must guarantee that the limits
        // of the allocator do also comply with the pow-2 constraint.
    }
};

//TODO Consider implementing `contexpr` manipulation for allocators.
// To do that, the allocator should use template `GetData` method.
// Thus, it could match its underlying type with the collection's element type,
// allowing for constexpr operations on the allocator's data.
// Ngl, if it works it's gonna be a game changer.
