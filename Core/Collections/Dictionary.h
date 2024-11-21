// Created by Mateusz Karbowiak 2024

#pragma once

#include "Allocators/HeapAlloc.h"
#include "Collections/CollectionsUtils.h"
#include "Language/TypeInfo.h"
#include "Math/HashingFunctions.h"
#include "Math/Probing.h"

template<
    typename K,
    typename V,
    typename Alloc = HeapAlloc,
    int32(&Probe)(int32, int32) = Probing::Linear
>
class Dictionary;

/// <summary> Potential entry of the dictionary. </summary>
/// <remarks>
/// The bucket must be predeclared to allow for type traits.
/// </remarks>
template<typename K, typename V>
class DictionaryBucket final
{
public:
    using BucketState = CollectionsUtils::BucketState;

private:
    V _value;
    K _key;
    BucketState _state;

public:
    FORCE_INLINE NODISCARD
    BucketState State() const { return _state; }

    FORCE_INLINE NODISCARD
    const K& Key() const { return _key; }

    FORCE_INLINE NODISCARD
    const V& Value() const { return _value; }

    FORCE_INLINE NODISCARD
    V& Value() { return _value; }


    DictionaryBucket() : _state{ BucketState::Empty } {}

    template<typename K_, typename V_>
    explicit DictionaryBucket(K_&& key, V_&& value) noexcept
        : _value{ FORWARD(V_, value) }
        , _key{ FORWARD(K_, key) }
        , _state{ BucketState::Occupied }
    {
    }

    DictionaryBucket(DictionaryBucket&& other) noexcept
        : _value{ MOVE(other._value) }
        , _key{ MOVE(other._key) }
        , _state{ other._state }
    {
        other._state = BucketState::Empty;
        other._value.~V();
        other._key.  ~K();
    }

    DictionaryBucket(const DictionaryBucket& other) = delete;

    void Reset()
    {
        if (_state == BucketState::Occupied)
        {
            _value.~V();
            _key.  ~K();
        }
        _state = BucketState::Empty;
    }

    DictionaryBucket& operator=(DictionaryBucket&& other) noexcept
    {
        if (this == &other)
            return *this;

        Reset();

        if (other._state == BucketState::Occupied)
        {
            new (&_value) V(MOVE(other._value));
            new (&_key)   K(MOVE(other._key));
            _state = BucketState::Occupied;
        }

        return *this;
    }

    DictionaryBucket& operator=(const DictionaryBucket& other) = delete;

    ~DictionaryBucket()
    {
        Reset();
    }
};


/// <summary> Type trait indicating whether a type is a C-style type. </summary>
/// <remarks>
/// If both key and value types are C-style, the bucket is also C-style.
/// Allow the dictionary to use optimized memory operations.
/// </remarks>
template<typename K, typename V>
struct TIsCStyle<DictionaryBucket<K, V>>
{
    static constexpr bool Value = TIsCStyle<K>::Value && TIsCStyle<V>::Value;
};


template<
    typename K,
    typename V,
    typename Alloc,
    int32(&Probe)(int32, int32)
>
class Dictionary 
{
    using AllocData          = typename Alloc::Data;
    using Bucket             = DictionaryBucket<K, V>;
    using BucketSearchResult = CollectionsUtils::BucketSearchResult;
    using BucketState        = CollectionsUtils::BucketState;
    using HashType           = int32; //TODO Fix this.

    AllocData _allocData;
    int32 _capacity;
    int32 _elementsCount;
    int32 _deletedCount;


    // Capacity Access and Utility

public:
    /// <summary> Checks if the array has an active allocation. </summary>
    FORCE_INLINE NODISCARD
    constexpr bool IsAllocated() const noexcept
    {
        return _capacity > 0;
    }

    /// <summary> Number of elements that can be stored without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr int32 Capacity() const noexcept
    {
        return _capacity;
    }

protected:
    /// <summary> Searches for the bucket with the specified key. </summary>
    void FindBucket(const K& key, BucketSearchResult& result) const
    {
        ASSERT(IsAllocated());
        ASSERT(Math::NextPow2(_capacity) == _capacity);

        const HashType hashMask = static_cast<HashType>(_capacity) - 1;
        const Bucket* data = DATA_OF(const Bucket, _allocData);

        int32 bucketIndex = GetHash(key) & hashMask;
        int32 insertIndex = -1;
        int32 checkCount  = 0;

        result.FoundObject = -1;
        result.FreeBucket  = -1;

        while (checkCount < _capacity)
        {
            const Bucket& bucket = data[bucketIndex];

            if (bucket.State() == BucketState::Empty)
            {
                // Found an empty bucket
                result.FoundObject = -1;
                result.FreeBucket = (insertIndex != -1) ? insertIndex : bucketIndex;
                return;
            }

            if (bucket.State() == BucketState::Deleted)
            {
                // Keep track of the first available deleted slot
                if (insertIndex == -1)
                    insertIndex = bucketIndex;
            }
            else if (bucket.Key() == key)
            {
                // Found the key
                result.FoundObject = bucketIndex;
                return;
            }

            // Move to the next probe
            ++checkCount;
            bucketIndex = (bucketIndex + Probe(_capacity, checkCount)) & hashMask;
        }

        // If no match or free slot was found
        result.FoundObject = -1;
        result.FreeBucket  = insertIndex;
    }

    /// <summary>
    /// Realigns the dictionary entries, using a new allocation of the specified minimal capacity.
    /// </summary>
    void Rebuild(const int32 minCapacity)
    {
        // Prepare new allocation
        const AllocData& oldData = _allocData;
        AllocData newData = oldData; // Copy the binding

        const int32 requestedCapacity =
            CollectionsUtils::GetRequiredCapacity<Bucket, Alloc, HASH_MAPS_DEFAULT_CAPACITY>(minCapacity);
        const int32 newCapacity =
            CollectionsUtils::AllocateCapacity<Bucket, Alloc>(newData, requestedCapacity);

        // Initialize the new data
        for (int32 i = 0; i < newCapacity; ++i)
            new (DATA_OF(Bucket, _allocData) + i) Bucket();

        // Rebuilt the dictionary
        for (int32 i = 0; i < _capacity; ++i)
        {
            Bucket& oldBucket = DATA_OF(Bucket, _allocData)[i];

            if (oldBucket.State() != BucketState::Occupied)
                continue;

            BucketSearchResult searchResult;
            FindBucket(oldBucket.Key(), searchResult);

            ASSERT(searchResult.FoundObject == -1);
            ASSERT(searchResult.FreeBucket != -1);

            Bucket& newBucket = DATA_OF(Bucket, newData)[searchResult.FreeBucket];
            newBucket = MOVE(oldBucket);
        }

        // Destroy the old data
        BulkOperations::DestroyLinearContent<Bucket>(DATA_OF(Bucket, _allocData), _capacity);


        // The dictionary has been rebuilt into the new allocation.
        // Now we need to check if the new allocation can actually drag items to replace the old allocation.
        // If not, we need to manually move the content, and treat the new allocation as temporary storage.

        if (newData.MovesItems())
        {
            // If yes, we can just move the new data to the old data.
            // New, rebuilt data is ready to be used.
            _allocData = MOVE(newData);
            _capacity  = newCapacity;
            return;
        }

        // If not, we need to manually move the content back to the old allocation.

        // To be fully correct, we should reduce the size of original allocation:
        // oldData.Free();
        // oldData.Allocate(newCapacity * sizeof(Bucket));
        // But non-dragging allocators have generally fixed size, so we can assume,
        // this action will not bring any changes to the allocation size.

        BulkOperations::MoveLinearContent<Bucket>(
            DATA_OF(Bucket, newData),
            DATA_OF(Bucket, _allocData),
            newCapacity
        );

        BulkOperations::DestroyLinearContent<Bucket>(
            DATA_OF(Bucket, newData),
            newCapacity
        );

        newData.Free();
        // No need to update the capacity, as the old allocation size remains the same.
    }

public:
    /// <summary>
    /// Rebuilds the dictionary to the smallest possible size.
    /// If the dictionary is empty, the allocation is freed.
    /// </summary>
    void Compact()
    {
        // Check if there is possibility of relocation.
        if (_elementsCount == 0) 
        {
            if (_capacity > 0)
            {
                _allocData.Free();
                _capacity = 0;
            }
            return;
        }

        // Rebuild the dictionary
        Rebuild(_elementsCount * HASH_MAPS_DEFAULT_SLACK_RATIO);
    }

    /// <summary> Potentially rebuilds the dictionary to achieve the specified minimal capacity. </summary>
    void EnsureCapacity(const int32 minCapacity)
    {
        if (_capacity >= minCapacity)
            return;

        if (_capacity == 0)
        {
            // Allocate the dictionary
            const int32 requestedCapacity =
                CollectionsUtils::GetRequiredCapacity<Bucket, Alloc, HASH_MAPS_DEFAULT_CAPACITY>(minCapacity);
            _capacity = CollectionsUtils::AllocateCapacity<Bucket, Alloc>(_allocData, requestedCapacity);

            // Initialize the dictionary
            for (int32 i = 0; i < _capacity; ++i)
                new (DATA_OF(Bucket, _allocData) + i) Bucket();
        }
        else
        {
            // Rebuild the dictionary
            Rebuild(minCapacity);
        }
    }

    /// <summary> Resets the dictionary to an empty state. </summary>
    void Reset()
    {
        if (_capacity == 0)
            return;

        BulkOperations::DestroyLinearContent<Bucket>(DATA_OF(Bucket, _allocData), _capacity);

        _allocData.Free();
        _capacity = 0;
        _elementsCount = 0;
        _deletedCount = 0;
    }



    // Count Access

    /// <summary> Number of valid key-value pairs in the dictionary. </summary>
    FORCE_INLINE NODISCARD
    int32 Count() const noexcept
    {
        return _elementsCount;
    }

    /// <summary> Checks if the dictionary has no valid key-value pairs. </summary>
    FORCE_INLINE NODISCARD
    bool IsEmpty() const noexcept
    {
        return _elementsCount == 0;
    }



    // Element Access

    /// <summary> Checks if there is a bucket associated with the specified key. </summary>
    FORCE_INLINE NODISCARD
    bool Contains(const K& key) const
    {
        if (_elementsCount == 0)
            return false;

        BucketSearchResult searchResult;
        FindBucket(key, searchResult);

        return searchResult.FoundObject != -1;
    }

    /// <summary>
    /// Tries to get the value associated with the specified key.
    /// If the key does not exist, nullptr is returned.
    /// </summary>
    NODISCARD FORCE_INLINE
    V* TryGet(const K& key)
    {
        if (_elementsCount == 0)
            return nullptr;

        BucketSearchResult searchResult;
        FindBucket(key, searchResult);

        if (searchResult.FoundObject == -1)
            return nullptr;

        return &DATA_OF(Bucket, _allocData)[searchResult.FoundObject].Value();
    }

    /// <summary>
    /// Tries to get the value associated with the specified key.
    /// If the key does not exist, nullptr is returned.
    /// </summary>
    NODISCARD FORCE_INLINE
    const V* TryGet(const K& key) const
    {
        if (_elementsCount == 0)
            return nullptr;

        BucketSearchResult searchResult;
        FindBucket(key, searchResult);

        if (searchResult.FoundObject == -1)
            return nullptr;

        return &DATA_OF(Bucket, _allocData)[searchResult.FoundObject].Value();
    }


    // Element Manipulation

    /// <summary> Ensures that a bucket can be associated with the specified key. </summary>
    /// <remarks>
    /// There must be no already associated bucket with the same key. Otherwise, it will cause error.
    /// Warning: The lifetime of the bucket has not started yet.
    /// </remarks>
    template<typename K_, typename V_>
    NODISCARD
    Bucket* Add(K_&& key, V_&& value)
    {
        // 1. Ensure correct capacity
        if (_deletedCount > (_capacity / HASH_MAPS_DEFAULT_SLACK_RATIO))
            Compact(); // If the number of deleted elements is too high, shrink the table

        EnsureCapacity((_elementsCount + 1) * HASH_MAPS_DEFAULT_SLACK_RATIO + _deletedCount);

        //TODO Prevent double rebuiding
        
        // 2. Find the bucket to reserve
        BucketSearchResult searchResult;
        FindBucket(key, searchResult);

        ASSERT(searchResult.FoundObject == -1); // The key already exists in the dictionary
        ASSERT(searchResult.FreeBucket  != -1); // No free bucket was found

        // 3. Reserve the bucket
        ++_elementsCount;
        Bucket* bucket = DATA_OF(Bucket, _allocData) + searchResult.FreeBucket;
        *bucket = Bucket(FORWARD(K_, key), FORWARD(V_, value));
        return bucket;
    }

    /// <summary> Removes the bucket associated with the specified key, if it exists. </summary>
    /// <param name="key">The key of the bucket to remove.</param>
    /// <returns>True if the bucket was removed, false if the bucket was not found.</returns>
    bool Remove(const K& key)
    {
        if (_elementsCount == 0)
            return false;

        BucketSearchResult searchResult;
        FindBucket(key, searchResult);

        if (searchResult.FoundObject == -1)
            return false;

        Bucket& bucket = DATA_OF(Bucket, _allocData)[searchResult.FoundObject];
        bucket.Reset();
        --_elementsCount;
        ++_deletedCount;

        return true;
    }


    // Collection Lifecycle - Constructors

    Dictionary()
        : _capacity{ 0 }
        , _elementsCount{ 0 }
        , _deletedCount{ 0 }
    {
        
    }

    //TODO Implement this.


    // Collection Lifecycle - Assignments

    //TODO Implement this.


    // Collection Lifecycle - Destructor

    ~Dictionary()
    {
        if (_capacity <= 0)
            return;

        BulkOperations::DestroyLinearContent<Bucket>(DATA_OF(Bucket, _allocData), _capacity);
        _allocData.Free();
    }


    // Iteration

    auto Enumerate() = delete; //TODO Implement this.

    auto EnumerateKeys() = delete; //TODO Implement this.

    auto EnumerateValues() = delete; //TODO Implement this.
};
