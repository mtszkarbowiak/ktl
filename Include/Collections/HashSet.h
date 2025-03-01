// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"
#include "Collections/LoadFHelper.h"
#include "Types/Index.h"
#include "Types/Nullable.h"

/// <summary>
/// Open-addressing hash set with power-of-two capacity and specified probing strategy.
/// It uses nested <c>Nullable</c> to track empty and deleted slots.
/// </summary>
/// 
/// <typeparam name="T">
/// The type of elements stored in the set.
/// Must be movable (both constructor and assignment), non-const, and non-reference.
/// It must support hashing and equality comparison.
/// If it supports tombstone semantics, it will be used by the collection.
/// The hash of the element must never change to ensure the integrity of the set.
/// </typeparam>
/// <typeparam name="A">
/// (Optional) The type of the allocator to use.
/// Can be either a dragging or non-dragging allocator.
/// </typeparam>
/// <typeparam name="H">
/// (Optional) Class providing hashing function for the stored elements.
/// </typeparam>
/// <typeparam name="P">
/// (Optional) Class providing a function that calculates the next probing index.
/// </typeparam>
template<
    typename T,
    typename A = DefaultAlloc,
    typename H = HashOf<T>,
    typename P = LinearProbing
>
class HashSet
{
public:
    // Cell is a slot which had an element at some point.
    // Slot without a cell means that it was never occupied.

    using Element = T;
    using Cell    = Nullable<Element>;
    using Slot    = Nullable<Cell>;

    using HashWrapper = H;
    using AllocData   = typename A::Data;
    using AllocHelper = AllocHelperOf<Slot, A, HASH_SETS_DEFAULT_CAPACITY, DoubleGrowth>;
    using LoadFHelper = LoadFHelperOf<HASH_SETS_DEFAULT_SLACK_RATIO>;

    constexpr static int32 SlotSize = sizeof(Slot);

PRIVATE:
    AllocData _allocData{ NullOptT{} };
    int32     _capacity{};           // Number of slots
    int32     _elementCountCached{}; // Number of elements
    int32     _cellsCountCached{};   // Number of cells


    // Capacity Access

public:
    /// <summary> Checks if the array has an active allocation. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsAllocated() const -> bool
    {
        return _capacity > 0;
    }

    /// <summary> Number of elements that can be stored without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Capacity() const -> int32
    {
        return _capacity;
    }


    // Count Access

    /// <summary> Checks if the array has any elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsEmpty() const -> bool
    {
        return _elementCountCached == 0;
    }

    /// <summary> Number of currently stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Count() const -> int32
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
    auto Slack() const -> int32
    {
        return _capacity - _cellsCountCached;
    }

    /// <summary> Number of cells - slots that store an element or a marker. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto CellCount() const -> int32
    {
        return _cellsCountCached;
    }

    /// <summary> Number of cells (not slots) that are empty. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto CellSlack() const -> int32
    {
        return _cellsCountCached - _elementCountCached;
    }


    // Hashing

PRIVATE:
    NO_DISCARD FORCE_INLINE
    auto IsValid() const -> bool
    {
        // Nothing to check if the set is empty
        if (_capacity == 0) 
        {
            return
                0 == _elementCountCached && 
                0 == _cellsCountCached;
        }

        // Ensure that the capacity is a power of 2
        if (!Math::IsPow2(_capacity))
            return false;

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

    /// <summary> Searches for a place for existing or incoming elements. </summary>
    /// <param name="data"> The data array to search. </param>
    /// <param name="capacity"> The capacity of the data array. </param>
    /// <param name="key"> The key of the element to find. </param>
    NO_DISCARD
    static auto FindSlot(
        const Slot* data,
        const int32 capacity,
        const Element& key
    ) ->  HashSlotSearchResult
    {
        ASSERT_COLLECTION_INTEGRITY(data);
        ASSERT_COLLECTION_INTEGRITY(Math::IsPow2(capacity)); // Make sure the capacity is a power of 2

        Nullable<Index> keyCell;
        Nullable<Index> firstFreeSlot;

        const int32 capacityBitMask = capacity - 1;
        const int32 initIndex = H::GetHash(key) & capacityBitMask;

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
                return { keyCell, firstFreeSlot };
            }

            const Cell& cell = slot.Value();
            if (cell.HasValue() && cell.Value() == key)
            {
                // Found the key
                firstFreeSlot.Clear();
                return { keyCell, firstFreeSlot };
            }

            // Record the first deleted slot if no empty slot was found yet
            if (cell.IsEmpty() && firstFreeSlot.IsEmpty())
            {
                firstFreeSlot = keyCell;
            }

            // Move to the next slot using the probing strategy
            keyCell.Set((initIndex + P::Next(capacity, checkCount)) % capacityBitMask);
            ++checkCount;
        }

        // The set is full, or no suitable slot was found
        keyCell.Clear();
        firstFreeSlot.Clear();

        return { keyCell, firstFreeSlot };
    }

    void RebuildImpl(const int32 minCapacity)
    {
        ASSERT_COLLECTION_INTEGRITY(IsValid()); // Ensure the integrity of the collection
        ASSERT_COLLECTION_SAFE_MOD(!IsEmpty()); // Rebuilding an empty collection is not allowed.

        // 1. Set up the new content.

        AllocData newData{ _allocData };
        const int32 requiredCapacity  = Math::NextPow2(minCapacity);
        const int32 requestedCapacity = AllocHelper::InitCapacity(requiredCapacity);
        const int32 allocatedCapacity = AllocHelper::Allocate(newData, requestedCapacity);

        ASSERT_COLLECTION_INTEGRITY(Math::IsPow2(allocatedCapacity)); // Better safe than sorry

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
            const HashSlotSearchResult searchResult = FindSlot(
                DATA_OF(Slot, newData), 
                allocatedCapacity, 
                oldCell.Value()
            );

            const Nullable<::Index>& keyCell = searchResult.FoundObject;
            const Nullable<::Index>& firstFreeSlot = searchResult.FreeBucket;

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
            _capacity  = AllocHelper::Allocate(_allocData, requestedCapacity);

            BulkOperations::MoveLinearContent<Slot>(
                DATA_OF(Slot, newData),
                DATA_OF(Slot, _allocData),
                _capacity
            );
            BulkOperations::DestroyLinearContent<Slot>(
                DATA_OF(Slot, newData),
                _capacity
            );

            newData.Free();
        }

        // 5. Update the cached counts.
        _cellsCountCached = _elementCountCached;
    }

public:
    /// <summary> Forces the set to rebuild itself. </summary>
    void Rebuild()
    {
        const int32 desiredCapacity = LoadFHelper::SlotsForElements(_elementCountCached);
        RebuildImpl(desiredCapacity);
    }


    // Allocation Manipulation

public:
    /// <summary>
    /// Ensures that the set can store at least the specified number of slots (not elements).
    /// May require rebuilding (rehashing) the set.
    /// </summary>
    /// <param name="minCapacitySlots"> The minimal number of <b>slots</b> that the set should be able to store. </param>
    void ReserveSlots(const int32 minCapacitySlots)
    {
        if (minCapacitySlots < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        if (minCapacitySlots <= _capacity)
            return; // Reserving the same capacity would not increase the capacity.

        if (_capacity == 0)
        {
            // Allocate the initial capacity and initialize the slots
            const int32 requiredCapacity  = Math::NextPow2(minCapacitySlots);
            const int32 requestedCapacity = AllocHelper::InitCapacity(requiredCapacity);
            _capacity = AllocHelper::Allocate(_allocData, requestedCapacity);
            BulkOperations::DefaultLinearContent<Slot>(DATA_OF(Slot, _allocData), _capacity);
        }
        else
        {
            // Rebuild the dictionary
            RebuildImpl(minCapacitySlots);
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

    /// <summary> Removes all elements from the set and frees the allocation. </summary>
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
    auto Contains(const Element& key) const -> bool
    {
        if (_capacity == 0)
            return false;

        // Find the cell that contains the key
        const HashSlotSearchResult searchResult = FindSlot(
            DATA_OF(const Slot, _allocData),
            _capacity,
            key
        );

        const Nullable<::Index>& keyCell = searchResult.FoundObject;

        // If the key was found, return true
        return keyCell.HasValue();
    }

    /// <summary> Adds the specified element to the set. </summary>
    /// <returns> True if the element was added, false if the element was already in the set. </returns>
    MAY_DISCARD
    auto Add(Element&& element) -> bool
    {
        if (_capacity == 0)
        {
            // Allocate the initial capacity and initialize the slots
            _capacity = AllocHelper::Allocate(_allocData, HASH_SETS_DEFAULT_CAPACITY);
            BulkOperations::DefaultLinearContent<Slot>(DATA_OF(Slot, _allocData), _capacity);
        }

        // Find the cell that contains the key
        HashSlotSearchResult searchResult = FindSlot(
            DATA_OF(const Slot, _allocData),
            _capacity, 
            element
        );

        // If the key was found, return
        if (searchResult.FoundObject.HasValue())
            return false;

        // If there is no free slot, rebuild the set
        if (searchResult.FreeBucket.IsEmpty())
        {
            RebuildImpl(_elementCountCached + 1);
            searchResult = FindSlot(
                DATA_OF(const Slot, _allocData),
                _capacity, 
                element
            );

            // Note: This implementation is different than the original.
            // Instead of checking the slack ratio, it rebuilds the set when there is no free slot.
        }

        ASSERT(searchResult.FreeBucket.HasValue()); // There must be a free slot

        // Add the element to the set
        Slot* slot = DATA_OF(Slot, _allocData) + searchResult.FreeBucket.Value();
        slot->Set(Cell{ MOVE(element) });

        ++_elementCountCached;
        ++_cellsCountCached;

        return true;
    }

    /// <summary> Adds the specified element to the set. </summary>
    /// <returns> True if the element was added, false if the element was already in the set. </returns>
    MAY_DISCARD FORCE_INLINE
    auto Add(const Element& element) -> bool
    {
        return Add(MOVE(Element{ element })); // Maybe a forwarding reference in the future? Or emplacement?
    }

    /// <summary> Removes the specified element from the set. </summary>
    /// <returns> True if the element was removed, false if the element was not in the set. </returns>
    MAY_DISCARD
    auto Remove(const Element& key) -> bool
    {
        if (_capacity == 0)
            return false;

        // Find the cell that contains the key
        const HashSlotSearchResult searchResult = FindSlot(
            DATA_OF(const Slot, _allocData),
            _capacity,
            key
        );

        const Nullable<::Index>& keyCell = searchResult.FoundObject;

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


    // Utility

    /// <summary>
    /// Adds one-by-one copies of the specified elements to the end of the array.
    /// Max one allocation is performed.
    /// </summary>
    void AddElements(const Element* source, const int32 count)
    {
        const int32 totalElements = _elementCountCached + count;
        const int32 requiredSlotsCount = LoadFHelper::SlotsForElements(totalElements);

        ReserveSlots(requiredSlotsCount);

        for (int32 i = 0; i < count; ++i)
            Add(source[i]);
    }


protected:
    void MoteToEmpty(HashSet&& other) noexcept
    {
        ASSERT_COLLECTION_SAFE_MOD(_capacity == 0 && _elementCountCached == 0); // The set must be empty, but the collection must be initialized!
        ASSERT_COLLECTION_INTEGRITY(other.IsValid()); // Ensure the integrity of the collection


        if (other._capacity == 0 || other._elementCountCached == 0)
            return;

        if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);
            _capacity = other._capacity;
            _elementCountCached = other._elementCountCached;
            _cellsCountCached = other._cellsCountCached;

            // The items have been moved with the allocator.
            // The capacity must be reset manually.

            other._capacity = 0;
            other._elementCountCached = 0;
            other._cellsCountCached = 0;
        }
        else
        {
            _capacity = AllocHelper::Allocate(_allocData, other._capacity);
            ASSERT(_capacity == other._capacity);

            //TODO(mtszkarbowiak): Add rebuild on move.

            BulkOperations::MoveLinearContent<Slot>(
                DATA_OF(Slot, other._allocData),
                DATA_OF(Slot, _allocData),
                other._capacity
            );
            BulkOperations::DestroyLinearContent<Slot>(
                DATA_OF(Slot, other._allocData),
                other._capacity
            );

            _capacity = other._capacity;
            _elementCountCached = other._elementCountCached;
            _cellsCountCached = other._cellsCountCached;
            other._allocData.Free();
            other._capacity = 0;
            other._elementCountCached = 0;
            other._cellsCountCached = 0;
        }
    }

    void CopyToEmpty(const Slot* source, const int32 count)
    {
        ASSERT_COLLECTION_SAFE_MOD(_capacity == 0 && _elementCountCached == 0); // The set must be empty, but the collection must be initialized!

        if (count == 0)
            return;

        // Allocate the initial capacity and initialize the slots
        _capacity = AllocHelper::Allocate(_allocData, HASH_SETS_DEFAULT_CAPACITY);
        BulkOperations::DefaultLinearContent<Slot>(DATA_OF(Slot, _allocData), _capacity);

        // Add the elements to the set
        for (int32 i = 0; i < count; ++i)
        {
            const Slot& slot = source[i];
            if (slot.IsEmpty())
                continue;
            const Cell& cell = slot.Value();
            if (cell.HasValue())
                Add(cell.Value());
        }
    }



    // Lifecycle

public:
    /// <summary> Initializes an empty set with no active allocation. </summary>
    HashSet() = default;

    /// <summary> Initializes a set by moving the allocation from another set. </summary>
    FORCE_INLINE
    HashSet(HashSet&& other) noexcept
    {
        MoteToEmpty(MOVE(other));
    }

    /// <summary> Initializes a set by copying another set. </summary>
    FORCE_INLINE
    HashSet(const HashSet& other)
    {
        if (other._capacity == 0)
            return;

        CopyToEmpty(DATA_OF(const Slot, other._allocData), other._capacity);
    }

    /// <summary> Initializes an empty hash set with an active allocation of the specified capacity and context. </summary>
    template<typename C_ = NullOptT>
    FORCE_INLINE explicit
    HashSet(const int32 capacity, C_&& context = NullOptT{})
        : _allocData{ FORWARD(C_, context) }
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);

        BulkOperations::DefaultLinearContent<Slot>(DATA_OF(Slot, _allocData), _capacity);
    }


    MAY_DISCARD FORCE_INLINE
    auto operator=(HashSet&& other) noexcept -> HashSet&
    {
        if (this != &other)
        {
            Reset();
            MoveFrom(MOVE(other));
        }
        return *this;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator=(const HashSet& other) -> HashSet&
    {
        if (this != &other)
        {
            Reset();
            CopyToEmpty(DATA_OF(const Slot, other._allocData), other._capacity);
        }
        return *this;
    }


    /// <summary> Destructor. </summary>
    ~HashSet()
    {
        Reset();
    }


    // Factorization

    /// <summary> Creates a has set with the specified elements. </summary>
    NO_DISCARD static constexpr
    auto Of(std::initializer_list<Element> list) -> HashSet
    {
        const int32 capacity = static_cast<int32>(list.size());
        HashSet set{ capacity };

        for (const Element& element : list)
            set.Add(element);

        return set;
    }


    // Pullers

PRIVATE:
    /// <summary>
    /// Moves the iterator index to the next occupied slot.
    /// If the end of the collection is reached, the capacity is returned.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto SkipToOccupied(int32 index) const -> int32
    {
        for (; index < _capacity; ++index)
        {
            const Slot& slot = DATA_OF(const Slot, _allocData)[index];

            // If there is no cell, skip the slot.
            if (slot.IsEmpty())
                continue;

            // If the cell is empty, skip the slot.
            if (slot.Value().HasValue())
                return index;
        }
        return _capacity;
    }


public:
    /// <summary> Puller for iterating over the elements of the set. </summary>
    /// <remarks> <c>HashSet</c> does not have a read-write puller. </remarks>
    class ConstValuePuller
    {
        const HashSet* _set;
        int32          _index;


    public:
        FORCE_INLINE explicit
        ConstValuePuller(const HashSet& set)
            : _set{ &set }
            , _index{ set.SkipToOccupied(0) }
        {
        }


        // Access

        NO_DISCARD FORCE_INLINE
        auto operator*() const -> const Element&
        {
            return DATA_OF(const Slot, _set->_allocData)[_index].Value().Value();
        }

        NO_DISCARD FORCE_INLINE
        auto operator->() const -> const Element*
        {
            return &(DATA_OF(const Slot, _set->_allocData)[_index].Value().Value());
        }


        // Iteration

        NO_DISCARD FORCE_INLINE
        auto Hint() const -> SizeHint
        {
            // In the future, this could be optimized to return the actual number of elements.
            // It will be done, once generalized hash collection utilities are introduced.
            return { 0, Nullable<::Index>{ _set->_elementCountCached } };
        }

        NO_DISCARD FORCE_INLINE explicit
        operator bool() const
        {
            return _index < _set->_capacity;
        }

        MAY_DISCARD FORCE_INLINE
            auto operator++() -> ConstValuePuller&
        {
            ++_index;
            _index = _set->SkipToOccupied(_index);
            return *this;
        }

        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> ConstValuePuller
        {
            auto copy = *this;
            ++*this;
            return copy;
        }


        // Identity

        NO_DISCARD FORCE_INLINE
        auto operator==(const ConstValuePuller& other) const -> bool
        {
            ASSERT_ITERATOR_SAFETY(&(other._set) == &_set);
            return _index == other._index;
        }

        NO_DISCARD FORCE_INLINE
        auto operator!=(const ConstValuePuller& other) const -> bool
        {
            return !(*this == other);
        }

        NO_DISCARD FORCE_INLINE
        auto operator<(const ConstValuePuller& other) const -> bool
        {
            ASSERT_ITERATOR_SAFETY(&(other._set) == &_set);
            return _index < other._index;
        }
    };


    NO_DISCARD FORCE_INLINE
    auto Values() const -> ConstValuePuller
    {
        return ConstValuePuller{ *this };
    }


    // Constraints

    static_assert(!TIsRefV<Element>,     INFO_TYPE_NOT_REF);
    static_assert(!TIsConstV<Element>,   INFO_TYPE_NOT_CONST);
    static_assert(TIsMoveableV<Element>, INFO_TYPE_MOVEABLE);

    static_assert(
        AllocHelper::HasBinaryMaskingSupport() == AllocHelper::BinaryMaskingSupportStatus::Supported,
        "The allocator must support binary masking."
    );
};
