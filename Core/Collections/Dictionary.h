// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Index.h"
#include "LoadFHelper.h"
#include "Collections/CollectionsUtils.h"
#include "Types/Nullable.h"

/// <summary>
/// Open-addressing hash map with power-of-two capacity and specified probing strategy.
/// It uses nested <c>Nullable</c> to track empty and deleted slots.
/// </summary>
/// 
/// <typeparam name="K">
/// The type of the keys identifying the stored values.
/// Must be movable (both constructor and assignment), non-const, and non-reference.
/// It must support hashing and equality comparison.
/// If it supports tombstone semantics, it will be used by the collection.
/// The hash of the key must never change to ensure the integrity of the map.
/// </typeparam>
/// <typeparam name="V">
/// The type of the values stored in the set.
/// Must be movable (both constructor and assignment), non-const, and non-reference.
/// It does not need to support hashing or equality comparison, nor tombstone semantics.
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
    typename K,
    typename V,
    typename A = DefaultAlloc,
    typename H = HashOf<K>,
    typename P = LinearProbing
>
class Dictionary
{
public:
    using Key   = K;
    using Value = V;

    /// <summary>
    /// Wrapper over a key-value pair that tracks the state of the slot, as element of hash map.
    /// </summary>
    class Slot
    {
        Value                   _value;
        Nullable<Nullable<Key>> _key;


        // Access
        
    public:
        /// <summary>
        /// Checks if the slot is ready to accept a new key-value pair.
        /// It defines the first stage of the slot lifecycle.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto IsEmpty() const -> bool
        {
            return _key.IsEmpty();
        }

        /// <summary>
        /// Checks if the slot is occupied by a key-value pair.
        /// It defines the second stage of the slot lifecycle.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto IsOccupied() const -> bool
        {
            if (_key.IsEmpty())
                return false;

            return _key.Value().HasValue();
        }

        /// <summary>
        /// Check if the slot was occupied by a key-value pair, but the key was removed.
        /// The slot is still marked as not able to accept a new pair.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto IsDeleted() const -> bool
        {
            return _key.HasValue() && _key.Value().IsEmpty();
        }

        /// <summary>
        /// Returns a reference to the stored pair value.
        /// This method can be called only when the slot is occupied.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto GetValue() -> Value&
        {
            ASSERT_COLLECTION_SAFE_ACCESS(IsOccupied());
            return _value;
        }
        
        /// <summary>
        /// Returns a reference to the stored pair value.
        /// This method can be called only when the slot is occupied.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto GetValue() const -> const Value&
        {
            ASSERT_COLLECTION_SAFE_ACCESS(IsOccupied());
            return _value;
        }

        /// <summary>
        /// Returns a reference to the stored pair key.
        /// This method can be called only when the slot is occupied.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto GetKey() -> Key&
        {
            ASSERT_COLLECTION_SAFE_ACCESS(IsOccupied());
            return _key.Value().Value();
        }

        /// <summary>
        /// Returns a reference to the stored pair key.
        /// This method can be called only when the slot is occupied.
        /// </summary>
        NO_DISCARD FORCE_INLINE
        auto GetKey() const -> const Key&
        {
            ASSERT_COLLECTION_SAFE_ACCESS(IsOccupied());
            return _key.Value().Value();
        }


        // Manipulation

        /// <summary>
        /// Removes the key-value pair from the slot.
        /// This method can be called only when the slot is occupied.
        /// This is because it leave a mark that the slot was occupied prior to the removal.
        /// </summary>
        void Delete()
        {
            ASSERT_COLLECTION_SAFE_MOD(IsOccupied());
            _key.Set(Nullable<Key>{});
        }

        /// <summary>
        /// Removes the key-value pair from the slot or the mark that it was removed.
        /// Puts the slot in the initial state, capable of accepting a new pair.
        /// This function should be called only during the rebuilding process.
        /// </summary>
        void Reset()
        {
            _key.Clear();
        }

        /// <summary>
        /// Sets the key-value pair in the slot.
        /// This method can be called only when the slot is empty.
        /// </summary>
        template<typename K_, typename V_>
        FORCE_INLINE
        void Set(K_&& key, V_&& value) // Universal references
        {
            ASSERT_COLLECTION_SAFE_MOD(IsEmpty());

            // Value is manipulated manually
            new (&_value) Value{ FORWARD(V_, value) };

            // Set the key
            Key theKey{ FORWARD(K_, key) };
            Nullable<Key> nullableKey{ MOVE(theKey) };
            _key.Set(MOVE(nullableKey));
        }


        // Lifecycle

        /// <summary> Initializes an empty slot. </summary>
        Slot()
            // Do not initialize the key, it will be set by the user!
            : _key{}
        {
        }

        //TODO Other lifecycle methods
    };

    using Element     = Slot;
    using AllocData   = typename A::Data;
    using AllocHelper = AllocHelperOf<Element, A, HASH_SETS_DEFAULT_CAPACITY, Growing::Double>;
    using LoadFHelper = LoadFHelperOf<HASH_SETS_DEFAULT_SLACK_RATIO>;

private:
    AllocData _allocData{};
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

    /// <summary> Number of key-value pairs that can be stored without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Capacity() const -> int32
    {
        return _capacity;
    }


    // Count Access

    /// <summary> Checks if the array has any key-value pairs. </summary>
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
    auto CellSlack() const -> int32
    {
        return _cellsCountCached - _elementCountCached;
    }


private:
    NO_DISCARD FORCE_INLINE
    auto IsValid() const -> bool
    {
        // Nothing to check if the set is empty
        if (_capacity) 
        {
            return
                0 == _elementCountCached &&
                0 == _cellsCountCached;
        }

        // Ensure that the capacity is a power of two
        if (!Math::IsPow2(_capacity))
            return false;

        // Iterate over all slots and recount the elements
        int32 actualValidElements = 0, actualCells = 0; //TODO Naming ambiguity of `Element`.
        for (int32 i = 0; i < _capacity; ++i)
        {
            const Slot& slot = DATA_OF(Slot, _allocData)[i];
            if (slot.IsEmpty())
                continue;

            ++actualCells;

            if (slot.IsOccupied())
                ++actualValidElements;
        }

        // Compare the actual values with the cached ones
        return
            actualValidElements == _elementCountCached &&
            actualCells         == _cellsCountCached;
    }

    /// <summary>
    /// Attempts to find a slot with the specified key, or the first one to be selected for insertion.
    /// </summary>
    /// <param name="slots"> Array of slots to search. </param>
    /// <param name="capacity"> Number of slots in the array. </param>
    /// <param name="key"> Key to search for. </param>
    NO_DISCARD
    static auto FindSlot(
        const Slot* slots,
        const int32 capacity,
        const Key& key
    ) -> Bucketing::SearchResult
    {
        ASSERT_COLLECTION_INTEGRITY(slots);

        const int32 initIndex = H::GetHash(key) % capacity;

        int32 currentIndex = initIndex;
        Nullable<Index> firstFree{};

        for (int32 numChecks = 0; numChecks < capacity; ++numChecks)
        {
            const Slot& slot = slots[currentIndex];

            // If a free slot has been found before a slot with the key, we have the result.
            if (slot.IsEmpty())
            {
                // If the previous slot was occupied we must use the next free slot.
                // Otherwise, we can re-use the last deleted slot.
                firstFree.SetIfNull(currentIndex);

                return { {},  firstFree };
            }

            if (slot.IsDeleted() && firstFree.IsEmpty())
            {
                // Keep changing the first free index until the end of the search.
                // Thus it will remember the last deleted slot, the one which can be re-used.
                firstFree.Set(currentIndex);
            }
            else if (slot.GetKey() == key)
            {
                // If the current slot has the key, just return the index :)
                return { Nullable<Index>{ currentIndex }, {} };
            }

            currentIndex = (initIndex + P::Next(capacity, numChecks)) % capacity;
        }

        // If everything failed, return double null to indicate that the search was unsuccessful.
        return {};

        //TODO Maybe this method is generalizable?
    }

    void RebuildImpl(const int32 miCapacitySlots)
    {
        ASSERT_COLLECTION_INTEGRITY(IsValid()); // Ensure the integrity of the collection
        ASSERT_COLLECTION_SAFE_MOD(!IsEmpty()); // Rebuilding an empty collection is not allowed.

        // 1. Set up the new content.
        AllocData newData{ _allocData };
        const int32 requiredCapacity  = Math::NextPow2(miCapacitySlots);
        const int32 requestedCapacity = AllocHelper::InitCapacity(requiredCapacity);
        const int32 allocatedCapacity = AllocHelper::Allocate(newData, requestedCapacity);

        ASSERT_COLLECTION_INTEGRITY(Math::IsPow2(allocatedCapacity));

        BulkOperations::DefaultLinearContent<Slot>(
            DATA_OF(Slot, newData),
            allocatedCapacity
        );
        
        // 2. Rebuild the set into the new allocation.
        for (int32 i = 0; i < _capacity; ++i) 
        {
            const Slot& oldSlot = DATA_OF(Slot, _allocData)[i];
            if (!oldSlot.IsOccupied()) // If there's no element, skip the slot.
                continue;

            const Bucketing::SearchResult result = FindSlot(
                DATA_OF(Slot, newData),
                allocatedCapacity,
                oldSlot.GetKey()
            );

            // If a free slot was not found, it means that the allocation is too small, or the probing strategy failed.
            ASSERT_COLLECTION_INTEGRITY(result.FreeBucket.HasValue());
            // If the key was found, it means it had already been there, aka something went very (very) wrong.
            ASSERT_COLLECTION_INTEGRITY(result.FoundObject.IsEmpty());

            // Move the element to the new slot.
            DATA_OF(Slot, newData)[result.FreeBucket.Value()] = MOVE(oldSlot);
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
            _capacity  = allocatedCapacity;
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
        _capacity = 0;
        _cellsCountCached = 0;
        _elementCountCached = 0;
    }


    // Elements Access

    /// <summary> Checks if the dictionary contains entry of specified key. </summary>
    NO_DISCARD FORCE_INLINE
    auto Contains(const Key& key) const -> bool
    {
        if (_capacity == 0)
            return false;

        return FindSlot(DATA_OF(const Slot, _allocData), _capacity, key).FoundObject.HasValue();
    }


    /// <summary>
    /// Adds the specified key-value pair to the dictionary.
    /// </summary>
    /// <returns>
    /// If a new pair added, <c>true</c> is returned.
    /// If the key was already in the set, and the value was just updated, <c>false</c> is returned.
    /// </returns>
    template<typename K_, typename V_> 
    MAY_DISCARD FORCE_INLINE
    auto Add(K_&& key, V_&& value) -> bool // Universal references
    {
        // Ensure that the dictionary has enough capacity.
        {
            const int32 requiredCapacity = LoadFHelper::SlotsForElements(_elementCountCached + 1);
            ReserveSlots(requiredCapacity);
        }

        const K& keyConstRef = key; // If your code fails here, check if `K_` is `const K&` or `K&&`.
        const Bucketing::SearchResult result = FindSlot(
            DATA_OF(Slot, _allocData),
            _capacity,
            keyConstRef
        );

        // If the entry is present, then just update the value.
        if (result.FoundObject.HasValue()) 
        {
            DATA_OF(Slot, _allocData)[result.FoundObject.Value()]
                .Set(FORWARD(K_, key), FORWARD(V_, value)); //TODO Leave the key, as it was?
            return false;
        }
        else 
        {
            // If the entry is not present, then add a new one.
            DATA_OF(Slot, _allocData)[result.FreeBucket.Value()]
                .Set(FORWARD(K_, key), FORWARD(V_, value));
            ++_elementCountCached;
            ++_cellsCountCached;
            return true;
        }
    }

    /// <summary> Removes the entry with the specified key from the dictionary. </summary>
    /// <returns> <c>true</c> if the entry was removed, <c>false</c> if the entry was not in the dictionary. </returns>
    MAY_DISCARD FORCE_INLINE
    auto Remove(const Key& key) -> bool
    {
        if (_capacity == 0)
            return false;

        const Bucketing::SearchResult result = FindSlot(
            DATA_OF(Slot, _allocData),
            _capacity,
            key
        );

        if (result.FoundObject.IsEmpty())
            return false;

        // Use `Delete` specifically to mark the slot as deleted.
        DATA_OF(Slot, _allocData)[result.FoundObject.Value()].Delete();
        --_elementCountCached;

        return true;
    }


    // Element Access

    /// <summary>
    /// Checks if the dictionary contains entry of specified key.
    /// If true, a pointer to the value is returned. Otherwise, <c>nullptr</c>.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto TryGet(const Key& key) -> Value*
    {
        if (_capacity == 0)
            return nullptr;

        const Bucketing::SearchResult result = FindSlot(
            DATA_OF(Slot, _allocData),
            _capacity,
            key
        );

        if (result.FoundObject.IsEmpty())
            return nullptr;

        return &(DATA_OF(Slot, _allocData)[result.FoundObject.Value()].GetValue());
    }

    /// <summary>
    /// Checks if the dictionary contains entry of specified key.
    /// If true, a pointer to the value is returned. Otherwise, <c>nullptr</c>.
    /// </summary>
    NO_DISCARD FORCE_INLINE
    auto TryGet(const Key& key) const -> const Value*
    {
        if (_capacity == 0)
            return nullptr;

        const Bucketing::SearchResult result = FindSlot(
            DATA_OF(Slot, _allocData),
            _capacity,
            key
        );

        if (result.FoundObject.IsEmpty())
            return nullptr;

        return &(DATA_OF(Slot, _allocData)[result.FoundObject.Value()].GetValue());
    }

    // Utility

protected:
    /// <summary>
    /// Moves the contents of the other set to this one.
    /// Make sure that this set is empty before calling this method.
    /// </summary>
    void MoteToEmpty(Dictionary&& other) noexcept
    {
        ASSERT_COLLECTION_SAFE_MOD(_capacity == 0 && _elementCountCached == 0); // The set must be empty, but the collection must be initialized!
        ASSERT_COLLECTION_INTEGRITY(other.IsValid()); // Ensure the integrity of the collection

        if (other._capacity == 0 || other._elementCountCached == 0)
            return;

        if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);
            _capacity  = other._capacity;
            _elementCountCached = other._elementCountCached;
            _cellsCountCached   = other._cellsCountCached;

            // The items have been moved with the allocator.
            // The capacity must be reset manually.

            other._capacity           = 0;
            other._elementCountCached = 0;
            other._cellsCountCached   = 0;
        }
        else
        {
            BulkOperations::MoveLinearContent<Slot>(
                DATA_OF(Slot, _allocData),
                DATA_OF(Slot, other._allocData),
                other._capacity
            );
            BulkOperations::DestroyLinearContent<Slot>(
                DATA_OF(Slot, other._allocData),
                other._capacity
            );

            // It could be considered rebuilding the dictionary on move.

            _capacity           = other._capacity;
            _elementCountCached = other._elementCountCached;
            _cellsCountCached   = other._cellsCountCached;
            other._allocData.Free();
            other._capacity = 0;
            other._elementCountCached = 0;
            other._cellsCountCached = 0;
        }
    }


    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty dictionary with no active allocation. </summary>
    FORCE_INLINE constexpr
    Dictionary() = default;

    /// <summary> Initializes a dictionary by moving the allocation from another dictionary. </summary>
    FORCE_INLINE constexpr
    Dictionary(Dictionary&& other) noexcept
    {
        MoteToEmpty(MOVE(other));
    }

    /// <summary> Initializes a dictionary by copying another dictionary. </summary>
    FORCE_INLINE constexpr
    Dictionary(const Dictionary& other)
    {
        //TODO
    }

    /// <summary> Initializes an empty array with an active context-less allocation of the specified capacity. </summary>
    FORCE_INLINE explicit
    Dictionary(const int32 capacity)
    {
        const int32 requiredCapacity  = Math::NextPow2(capacity);
        const int32 requestedCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requestedCapacity);
    }

    /// <summary> Initializes an empty array with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext>
    FORCE_INLINE explicit
    Dictionary(const int32 capacity, AllocContext&& context) // Universal reference
        : _allocData{ FORWARD(AllocContext, context) }
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }


    /// <summary> Assigns the contents of the other dictionary to this one by move. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(Dictionary&& other) noexcept -> Dictionary&
    {
        if (this != &other)
        {
            Reset();
            MoteToEmpty(MOVE(other));
        }
        return *this;
    }

    /// <summary> Assigns the contents of the other dictionary to this one by copy. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(const Dictionary& other) -> Dictionary&
    {
        if (this != &other)
        {
            //TODO
        }
        return *this;
    }


    /// <summary> Destructor. </summary>
    ~Dictionary()
    {
        Reset();
    }


    // Constraints

    static_assert(AllocHelper::HasBinaryMaskingSupport(), "The allocator must support binary masking.");

    static_assert(!std::is_reference<Key>                    ::value, "Type must not be a reference type.");
    static_assert(!std::is_reference<Value>                  ::value, "Type must not be a reference type.");
    static_assert(!std::is_const<Element>                    ::value, "Type must not be a const-qualified type.");
    static_assert(!std::is_const<Value>                      ::value, "Type must not be a const-qualified type.");

    static_assert(std::is_move_constructible<Key>            ::value, "Type must be move-constructible.");
    static_assert(std::is_move_constructible<Value>          ::value, "Type must be move-constructible.");
    static_assert(std::is_destructible<Key>                  ::value, "Type must be destructible.");
    static_assert(std::is_destructible<Value>                ::value, "Type must be destructible.");

    static_assert(std::is_nothrow_move_constructible<Key>    ::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_nothrow_move_constructible<Value>  ::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_nothrow_destructible<Key>          ::value, "Type must be nothrow destructible.");
    static_assert(std::is_nothrow_destructible<Value>        ::value, "Type must be nothrow destructible.");
};
