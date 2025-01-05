// Created by Mateusz Karbowiak 2024

#pragma once

#include "Index.h"
#include "Collections/CollectionsUtils.h"
#include "Types/Nullable.h"

/// <summary>
/// Open-addressing hash set with specified probing and growing strategy.
/// It uses nested <c>Nullable</c> to track empty and deleted slots.
/// </summary>
/// 
/// <typeparam name="T">
/// The type of elements stored in the set.
/// Must be movable (both constructor and assignment), non-const, and non-reference.
/// It must support hashing and equality comparison.
/// If it supports tombstone semantics, it will be used by the collection.
/// </typeparam>
/// <typeparam name="A">
/// (Optional) The type of the allocator to use.
/// Can be either a dragging or non-dragging allocator.
/// </typeparam>
/// <typeparam name="P">
/// (Optional) A reference to a function that calculates the next probing index.
/// </typeparam>
/// <typeparam name="G">
/// (Optional) A reference to a function that calculates the next capacity.
/// </typeparam>
/// <typeparam name="H">
/// (Optional) Class that provides hashing function for the stored elements.
/// </typeparam>
template<
    typename T,
    typename A = DefaultAlloc,
    int32(&P)(int32, int32) = Probing::Linear,
    int32(&G)(int32) = Growing::Default,
    typename H = HashOf<T>
>
class HashSet
{
public:
    // Cell is a slot which had an element at some point.
    // Slot without a cell means that it was never occupied.

    using Element     = T;
    using Cell        = Nullable<Element>;
    using Slot        = Nullable<Cell>;

    using AllocData   = typename A::Data;
    using AllocHelper = AllocHelperOf<Slot, A, HASH_SETS_DEFAULT_CAPACITY, G>;
    using HashHelper  = H;

private:
    AllocData _allocData{};
    int32     _capacity{};
    int32     _elementCountCached{};
    int32     _cellsCountCached{};


    // Capacity Access

public:
    /// <summary> Checks if the array has an active allocation. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    bool IsAllocated() const
    {
        return _capacity > 0;
    }

    /// <summary> Number of elements that can be stored without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    int32 Capacity() const 
    {
        return _capacity;
    }


    // Count Access

    /// <summary> Checks if the array has any elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    bool IsEmpty() const
    {
        return _elementCountCached == 0;
    }

    /// <summary> Number of currently stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    int32 Count() const
    {
        return _elementCountCached;
    }

    /// <summary>
    /// Number of elements that could be added without invoking the allocator,
    /// assuming that the collection would never be rebuilt.
    /// </summary>
    /// <remarks>
    /// This is not the same as the slack in the allocation.
    /// Some slots may not become occupied unless the collection is rebuilt.
    /// </remarks>
    NO_DISCARD FORCE_INLINE constexpr
    int32 Slack() const
    {
        return _capacity - _cellsCountCached;
    }

    /// <summary> Number of cells - slots that store an element or a marker. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    int32 CellCount() const
    {
        return _cellsCountCached;
    }

    /// <summary> Number of cells (not slots) that are empty. </summary>
    int32 CellSlack() const
    {
        return _cellsCountCached - _elementCountCached;
    }


    // Hashing

private:
    NO_DISCARD FORCE_INLINE
    bool IsValid() const
    {
        if (_capacity == 0)
            return true;

        // Iterate over all slots and count cached elements
        int32 actualElements = 0, actualCells = 0;
        for (int32 i = 0; i < _capacity; ++i)
        {
            const Slot& slot = DATA_OF(Slot, _allocData)[i];

            if (slot.IsEmpty())
                continue;

            ++actualCells;

            if (slot.Value().HasValue())
                ++actualElements;
        }

        // Check if the cached counts are correct
        return
            actualElements == _elementCountCached && 
            actualCells    == _cellsCountCached;
    }

    /// <summary>
    /// Calculates how many slots are required to store the specified number of elements.
    /// </summary>
    static FORCE_INLINE constexpr
    int32 GetDesiredCapacity(const int32 elementCount)
    {
        return elementCount + (elementCount) / HASH_SETS_DEFAULT_SLACK_RATIO;
    }

    /// <summary> Searches for a place for existing or incoming elements. </summary>
    /// <param name="key"> The key of the element to find. </param>
    /// <param name="keyCell"> The index of the cell that contains the key. </param>
    /// <param name="firstFreeSlot"> The index of the first empty slot. </param>
    static void FindSlot(
        const Slot* data,
        const int32 capacity,
        const Element& key,
        Nullable<Index>& keyCell,
        Nullable<Index>& firstFreeSlot
    )
    {
        const Index initIndex = HashOf<Element>::GetHash(key) % capacity;

        keyCell.Set(initIndex);
        firstFreeSlot.Clear();

        int32 checkCount = 0;
        while (checkCount < capacity)
        {
            const Slot& slot = data[keyCell.Value()];
            if (slot.IsEmpty())
            {
                // Record the first free slot if not already done
                if (firstFreeSlot.IsEmpty())
                    firstFreeSlot = keyCell;

                // No matching cell found, stop search
                keyCell.Clear();
                return;
            }

            const Cell& cell = slot.Value();
            if (cell.HasValue() && cell.Value() == key)
            {
                // Found the key
                firstFreeSlot.Clear();
                return;
            }

            // Record the first deleted slot if no empty slot was found yet
            if (cell.IsEmpty() && firstFreeSlot.IsEmpty())
            {
                firstFreeSlot = keyCell;
            }

            // Move to the next slot using the probing strategy
            keyCell.Set(P(keyCell.Value() + 1, capacity) % capacity);
            ++checkCount;
        }

        // The set is full, or no suitable slot was found
        keyCell.Clear();
        firstFreeSlot.Clear();
    }

    void RebuildImpl(const int32 minCapacity)
    {
        ASSERT_COLLECTION_INTEGRITY(IsValid()); // Ensure the integrity of the collection
        ASSERT_COLLECTION_SAFE_MOD(!IsEmpty()); // Rebuilding an empty collection is not allowed.

        // 1. Set up the new content.
        AllocData newData{ _allocData };
        const int32 requestedCapacity = AllocHelper::NextCapacity(_capacity, minCapacity);
        const int32 allocatedCapacity = AllocHelper::Allocate(newData, requestedCapacity);

        BulkOperations::DefaultLinearContent<Slot>(
            DATA_OF(Slot, newData),
            allocatedCapacity
        );

        // 2. Rebuild the set into the new allocation.

        // Iterate over old slots and find new places for the encountered elements.
        // Then, move the elements to the new slots.
        for (int32 i = 0; i < _capacity; ++i)
        {
            const Slot& oldSlot = DATA_OF(Slot, _allocData)[i];
            if (oldSlot.IsEmpty()) // If there is no cell, skip the slot.
                continue;

            const Cell& oldCell = oldSlot.Value(); // Unwrap the cell from the slot.
            if (oldCell.IsEmpty()) // Get rid of the deleted cell.
                continue;

            // Find a place for the element in the new set.
            Nullable<Index> keyCell, firstFreeSlot;
            FindSlot(
                DATA_OF(Slot, newData), 
                allocatedCapacity, 
                oldCell.Value(), 
                keyCell, 
                firstFreeSlot
            );

            // If the key was found, move the element to the new slot.
            if (keyCell.HasValue())
            {
                DATA_OF(Slot, newData)[keyCell.Value()].Set(Cell{ MOVE(oldCell) });
            }
            else if (firstFreeSlot.HasValue())
            {
                // If the key was not found, but there is a free slot, move the element there.
                DATA_OF(Slot, newData)[firstFreeSlot.Value()].Set(Cell{ MOVE(oldCell) });
            }
            else
            {
                // The set is full, or no suitable slot was found.
                ASSERT_COLLECTION_SAFE_MOD(false); // This should never happen.
            }
        }

        // 3. Destroy the old slots and replace the allocation.
        BulkOperations::DestroyLinearContent<Slot>(
            DATA_OF(Slot, _allocData),
            _capacity
        );
        _allocData.Free();

        // 4. Replace the old allocation and reset the capacity.
        if (newData.MovesItems())
        {
            _allocData = MOVE(newData);
            _capacity  = allocatedCapacity;
        }
        else
        {
            BulkOperations::MoveLinearContent<Slot>(
                DATA_OF(Slot, _allocData),
                DATA_OF(Slot, newData),
                _capacity
            );
            BulkOperations::DestroyLinearContent<Slot>(
                DATA_OF(Slot, _allocData),
                _capacity
            );

            _allocData = MOVE(newData);
            _capacity = allocatedCapacity;
        }

        // 5. Update the cached counts.
        _cellsCountCached = _elementCountCached;
    }

public:
    /// <summary> Forces the set to rebuild itself. </summary>
    void Rebuild()
    {
        const int32 desiredCapacity = GetDesiredCapacity(_elementCountCached);
        RebuildImpl(desiredCapacity);
    }


    // Allocation Manipulation

public:
    /// <summary>
    /// Ensures that the set can store at least the specified number of slots (not elements).
    /// May require rebuilding (rehashing) the set.
    /// </summary>
    void Reserve(const int32 minCapacity)
    {
        if (minCapacity < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        if (minCapacity <= _capacity)
            return; // Reserving the same capacity would not increase the capacity.

        if (_capacity == 0)
        {
            // Allocate the initial capacity and initialize the slots
            const int32 requestedCapacity = AllocHelper::NextCapacity(_capacity, minCapacity);
            _capacity = AllocHelper::Allocate(_allocData, requestedCapacity);
            BulkOperations::DefaultLinearContent<Slot>(DATA_OF(Slot, _allocData), _capacity);
        }
        else
        {
            // Rebuild the dictionary
            Rebuild(minCapacity);
        }
    }

    /// <summary>
    /// Attempts to reduce the capacity, without losing any elements.
    /// If the set is empty, the allocation will be freed.
    /// </summary>
    void Compact()
    {
        if (_elementCountCached == 0 && _capacity > 0)
        {
            _allocData.Free();
            _capacity = 0;
            return;
        }

        Rebuild();
    }

    void Reset()
    {
        if (_capacity == 0)
            return;

        BulkOperations::DestroyLinearContent<Slot>(
            DATA_OF(Slot, _allocData),
            _capacity
        );

        _allocData.Free();
        _capacity           = 0;
        _cellsCountCached   = 0;
        _elementCountCached = 0;
    }


    // Elements Access

    /// <summary> Checks if the set contains the specified key. </summary>
    NO_DISCARD
    bool Contains(const Element& key) const
    {
        if (_capacity == 0)
            return false;

        // Find the cell that contains the key
        Nullable<Index> keyCell;
        Nullable<Index> firstFreeSlot;
        FindSlot(
            DATA_OF(const Slot, _allocData),
            _capacity,
            key, 
            keyCell, 
            firstFreeSlot
        );

        // If the key was found, return true
        return keyCell.HasValue();
    }

    /// <summary> Adds the specified element to the set. </summary>
    /// <returns> True if the element was added, false if the element was already in the set. </returns>
    MAY_DISCARD
    bool Add(Element&& element)
    {
        if (_capacity == 0)
        {
            // Allocate the initial capacity and initialize the slots
            _capacity = AllocHelper::Allocate(_allocData, HASH_SETS_DEFAULT_CAPACITY);
            BulkOperations::DefaultLinearContent<Slot>(DATA_OF(Slot, _allocData), _capacity);
        }

        // Find the cell that contains the key
        Nullable<Index> keyCell;
        Nullable<Index> firstFreeSlot;
        FindSlot(
            DATA_OF(const Slot, _allocData),
            _capacity, 
            element, 
            keyCell, 
            firstFreeSlot
        );

        // If the key was found, return
        if (keyCell.HasValue())
            return false;

        // If there is no free slot, rebuild the set
        if (firstFreeSlot.IsEmpty())
        {
            RebuildImpl(_elementCountCached + 1);
            FindSlot(
                DATA_OF(const Slot, _allocData),
                _capacity, 
                element, 
                keyCell, 
                firstFreeSlot
            );

            // Note: This implementation is different than the original.
            // Instead of checking the slack ratio, it rebuilds the set when there is no free slot.
            //TODO Review the implementation
        }

        ASSERT(firstFreeSlot.HasValue()); // There must be a free slot

        // Add the element to the set
        Slot* slot = DATA_OF(Slot, _allocData) + firstFreeSlot.Value();
        slot->Set(Cell{ MOVE(element) });

        ++_elementCountCached;
        ++_cellsCountCached;

        return true;
    }

    MAY_DISCARD
    bool Remove(const Element& key)
    {
        if (_capacity == 0)
            return false;

        // Find the cell that contains the key
        Nullable<Index> keyCell;
        Nullable<Index> firstFreeSlot;
        FindSlot(
            DATA_OF(const Slot, _allocData),
            _capacity,
            key, 
            keyCell, 
            firstFreeSlot
        );

        // If the key was not found, return
        if (!keyCell.HasValue())
            return false;

        // Remove the element from the set
        Slot* slot = DATA_OF(Slot, _allocData) + keyCell.Value();
        slot->Set(Cell{}); // Use empty cell to mark the slot as deleted.

        --_elementCountCached;

        // What about compacting here?

        return true;
    }


    // Lifecycle

    //TODO

    ~HashSet()
    {
        Reset();
    }
};
