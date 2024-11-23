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
        // The rebuilding process consists of several steps:
        // 1. Move the items to a temporary storage, if possible drag the content.
        // 2. Allocate the new storage, reset the collection capacity.
        // 3. Iterate over the temporary storage, adding the items to the new storage and repositioning them.
        // 4. Destroy the temporary storage.


        // 1.1. Prepare intermediate bucket storage.
        AllocData tempAlloc{ _allocData }; // Copy the binding

        // 1.2. Transfer the items to the temporary storage.
        if (_allocData.MovesItems())
        {
            // If the allocator can move the items, we can just move the data.
            tempAlloc = MOVE(_allocData);
        }
        else
        {
            // If the allocator cannot move the items, we need to manually move the content.
            tempAlloc.Allocate(_capacity * sizeof(Bucket));
            BulkOperations::MoveLinearContent<Bucket>(
                DATA_OF(Bucket, _allocData),
                DATA_OF(Bucket, tempAlloc),
                _capacity
            );
        }

        // 2. Allocate the new storage.
        const int32 oldCapacity = _capacity;

        const int32 requestedCapacity =
            CollectionsUtils::GetRequiredCapacity<Bucket, Alloc, HASH_MAPS_DEFAULT_CAPACITY>(minCapacity);
        _capacity =
            CollectionsUtils::AllocateCapacity<Bucket, Alloc>(_allocData, requestedCapacity);

        // 2.1. We have raw memory, so we need to manually call the constructors.
        // This is necessary, as the memory is not zeroed. If the bucket is C-style, we can skip this step.
        BulkOperations::DefaultLinearContent<Bucket>(
            DATA_OF(Bucket, _allocData),
            _capacity
        );


        // 3. Iterate over the temporary storage, adding the items to the new storage and repositioning them.
        for (int32 i = 0; i < oldCapacity; ++i)
        {
            Bucket& oldBucket = DATA_OF(Bucket, tempAlloc)[i];

            if (oldBucket.State() != BucketState::Occupied)
                continue;

            BucketSearchResult searchResult;
            FindBucket(oldBucket.Key(), searchResult);

            ASSERT(searchResult.FoundObject == -1); // It would make no sense, if the object was found.
            ASSERT(searchResult.FreeBucket  != -1); // We should always find a free bucket, as we have just allocated the new storage.

            Bucket& newBucket = DATA_OF(Bucket, _allocData)[searchResult.FreeBucket];
            newBucket = MOVE(oldBucket);
        }
        // The collection itself is now rebuilt, time for cleaning...

        // 4. Destroy the temporary storage.
        BulkOperations::DestroyLinearContent<Bucket>(
            DATA_OF(Bucket, tempAlloc),
            oldCapacity
        );

        tempAlloc.Free();
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


    // Collections Lifecycle - Overriding Content

private:
    FORCE_INLINE
    void MoveFrom(Dictionary&& other) noexcept
    {
        ASSERT(!IsAllocated()); // The dictionary must be empty!

        if (!other.IsAllocated())
        {
            _allocData = AllocData{};
            _capacity = 0;
            _elementsCount = 0;
            _deletedCount = 0;
        }
        else if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);
            _capacity      = other._capacity;
            _elementsCount = other._elementsCount;
            _deletedCount  = other._deletedCount;
            other._capacity      = 0; // The allocation has been moved - reset the capacity!
            other._elementsCount = 0;
            other._deletedCount  = 0;
        }
        else
        {
            const int32 requestedCapacity = 
                CollectionsUtils::GetRequiredCapacity<Bucket, Alloc, HASH_MAPS_DEFAULT_CAPACITY>(other._elementsCount);

            _allocData     = AllocData{};
            _capacity      = CollectionsUtils::AllocateCapacity<Bucket, Alloc>(_allocData, requestedCapacity);
            _elementsCount = other._elementsCount;
            _deletedCount  = other._deletedCount;

            // No dictionary rebuilding, because moving is supposed to be as cheap as possible.

            BulkOperations::MoveLinearContent<Bucket>(
                DATA_OF(Bucket, other._allocData),
                DATA_OF(Bucket, this->_allocData),
                _capacity
            );

            other.Reset();
        }
    }

    template<typename OtherAlloc>
    void CopyFrom(const Dictionary<K, V, OtherAlloc, Probe>& other)
    {
        static_assert(std::is_copy_constructible<K>::value, "Type must be copy-constructible.");
        static_assert(std::is_copy_constructible<V>::value, "Type must be copy-constructible.");

        ASSERT(!IsAllocated()); // The dictionary must be empty!

        if (other._capacity == 0)
        {
            // If no allocation is active, just zero the members.
            _allocData = AllocData{};
            _capacity = 0;
            _elementsCount = 0;
            _deletedCount = 0;
        }
        else
        {
            // Copy is expensive - Let's rebuild the dictionary.
            // Note: If we weren't rebuilding, we could have used BulkOperations::CopyLinearContent.

            EnsureCapacity(other.Capacity());

            for (auto iterator = other.Buckets(); iterator; ++iterator)
                Add(iterator->Key(), iterator->Value());
        }
    }



    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty dictionary with no active allocation. </summary>
    FORCE_INLINE
    Dictionary()
        : _allocData{}
        , _capacity{ 0 }
        , _elementsCount{ 0 }
        , _deletedCount{ 0 }
    {
    }

    /// <summary> Initializes a dictionary by moving the allocation from another array. </summary>
    FORCE_INLINE
    Dictionary(Dictionary&& other) noexcept
    {
        MoveFrom(MOVE(other));
    }

    /// <summary> Initializes a dictionary by copying another dictionary. </summary>
    template<
        typename K_ = K,
        typename V_ = V,
        typename = typename std::enable_if<((
            std::is_copy_constructible<K>::value &&
            std::is_copy_constructible<V>::value &&
            std::is_same<K, K_>::value &&
            std::is_same<V, V_>::value
        ))>::type>
    Dictionary(const Dictionary& other)
        : _allocData{}
        , _capacity{ 0 }
        , _elementsCount{ 0 }
        , _deletedCount{ 0 }
    {
        CopyFrom<Alloc>(other);
    }

    /// <summary> Initializes an dictionary array with an active context-less allocation of the specified capacity. </summary>
    FORCE_INLINE
    explicit Dictionary(const int32 capacity)
        : _allocData{}
        , _elementsCount{ 0 }
        , _deletedCount{ 0 }
    {
        const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<Bucket, Alloc, HASH_MAPS_DEFAULT_CAPACITY>(capacity);
        _capacity = CollectionsUtils::AllocateCapacity<Bucket, Alloc>(_allocData, requiredCapacity);
    }

    /// <summary> Initializes an empty dictionary with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext>
    FORCE_INLINE
    explicit Dictionary(const int32 capacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
        , _elementsCount{ 0 }
        , _deletedCount{ 0 }
    {
        const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<Bucket, Alloc, HASH_MAPS_DEFAULT_CAPACITY>(capacity);
        _capacity = CollectionsUtils::AllocateCapacity<Bucket, Alloc>(_allocData, requiredCapacity);
    }


    // Collection Lifecycle - Assignments

    FORCE_INLINE
    auto operator=(Dictionary&& other) noexcept -> Dictionary&
    {
        if (this != &other)
        {
            Reset();
            MoveFrom(MOVE(other));
        }
        return *this;
    }

    template<
        typename K_ = K,
        typename V_ = V,
        typename = typename std::enable_if<((
            std::is_copy_constructible<K>::value&&
            std::is_copy_constructible<V>::value&&
            std::is_same<K, K_>::value&&
            std::is_same<V, V_>::value
        ))>::type>
    auto operator=(const Dictionary& other) -> Dictionary&
    {
        if (this != &other)
        {
            Reset();
            CopyFrom<Alloc>(other);
        }
        return *this;
    }


    // Collection Lifecycle - Destructor

    ~Dictionary()
    {
        if (_capacity <= 0)
            return;

        BulkOperations::DestroyLinearContent<Bucket>(DATA_OF(Bucket, _allocData), _capacity);
        _allocData.Free();
    }


    // Factorization

    //TODO Implement Of(std::initializer_list<U> list)


    // Iteration

    // Note: Dictionary is a complex collection, thus it has so many different enumerators.

private:
    FORCE_INLINE NODISCARD
    int32 SkipToOccupied(const int32 index) const
    {
        for (int32 i = index; i < _capacity; ++i)
        {
            if (DATA_OF(const Bucket, _allocData)[i].State() == BucketState::Occupied)
                return i;
        }
        return _capacity;
    }


public:
    class KeyEnumerator
    {
        const Dictionary* _dictionary;
        int32 _index;

    public:
        explicit KeyEnumerator(const Dictionary& dictionary)
            : _dictionary{ &dictionary }
            , _index{ 0 }
        {
            _index = _dictionary->SkipToOccupied(_index);
        }

        // Access

        FORCE_INLINE NODISCARD
        const K& operator*() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Key();
        }

        FORCE_INLINE NODISCARD
        const K* operator->() const
        {
            return &DATA_OF(const Bucket, _dictionary->_allocData)[_index].Key();
        }


        // Iteration

        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            return _index < _dictionary->_capacity;
        }

        FORCE_INLINE
        KeyEnumerator& operator++()
        {
            ++_index;
            _index = _dictionary->SkipToOccupied(_index);
            return *this;
        }

        FORCE_INLINE
        KeyEnumerator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }
    };

    class MutValEnumerator
    {
        Dictionary* _dictionary;
        int32 _index;

    public:
        explicit MutValEnumerator(Dictionary& dictionary)
            : _dictionary{ &dictionary }
            , _index{ 0 }
        {
            _index = _dictionary->SkipToOccupied(_index);
        }


        // Access

        FORCE_INLINE NODISCARD
        V& operator*()
        {
            return DATA_OF(Bucket, _dictionary->_allocData)[_index].Value();
        }

        FORCE_INLINE NODISCARD
        const V& operator*() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Value();
        }

        FORCE_INLINE NODISCARD
        V* operator->()
        {
            return &DATA_OF(Bucket, _dictionary->_allocData)[_index].Value();
        }

        FORCE_INLINE NODISCARD
        const V* operator->() const
        {
            return &DATA_OF(const Bucket, _dictionary->_allocData)[_index].Value();
        }


        // Iteration

        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            return _index < _dictionary->_capacity;
        }

        FORCE_INLINE
        MutValEnumerator& operator++()
        {
            ++_index;
            _index = _dictionary->SkipToOccupied(_index);
            return *this;
        }

        FORCE_INLINE
        MutValEnumerator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }
    };

    class ConstValEnumerator
    {
        const Dictionary* _dictionary;
        int32 _index;

    public:
        explicit ConstValEnumerator(const Dictionary& dictionary)
            : _dictionary{ &dictionary }
            , _index{ 0 }
        {
            _index = _dictionary->SkipToOccupied(_index);
        }


        // Access

        FORCE_INLINE NODISCARD
        const V& operator*() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Value();
        }

        FORCE_INLINE NODISCARD
        const V* operator->() const
        {
            return &DATA_OF(const Bucket, _dictionary->_allocData)[_index].Value();
        }

        // Iteration

        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            return _index < _dictionary->_capacity;
        }

        FORCE_INLINE
        ConstValEnumerator& operator++()
        {
            ++_index;
            _index = _dictionary->SkipToOccupied(_index);
            return *this;
        }

        FORCE_INLINE
        ConstValEnumerator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }
    };

    class MutBucketEnumerator
    {
        Dictionary* _dictionary;
        int32 _index;


    public:
        explicit MutBucketEnumerator(Dictionary& dictionary)
            : _dictionary{ &dictionary }
            , _index{ 0 }
        {
            _index = _dictionary->SkipToOccupied(_index);
        }


        // Access

        /// <summary> Accesses the bucket at the current index. </summary>
        /// <remarks>
        /// The bucket itself must not be modified, as it would break the dictionary.
        /// Use <c>Value</c> and <c>Key</c> to access the bucket content.
        /// </remarks>
        FORCE_INLINE NODISCARD
        Bucket& operator*() = delete;

        FORCE_INLINE NODISCARD
        const Bucket& operator*() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index];
        }

        /// <summary> Accesses the bucket at the current index. </summary>
        /// <remarks>
        /// The bucket itself must not be modified, as it would break the dictionary.
        /// Use <c>Value</c> and <c>Key</c> to access the bucket content.
        /// </remarks>
        FORCE_INLINE NODISCARD
        Bucket* operator->() = delete;

        FORCE_INLINE NODISCARD
        const Bucket* operator->() const
        {
            return &DATA_OF(const Bucket, _dictionary->_allocData)[_index];
        }


        FORCE_INLINE NODISCARD
        const K& Key() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Key();
        }

        FORCE_INLINE NODISCARD
        V& Value()
        {
            return DATA_OF(Bucket, _dictionary->_allocData)[_index].Value();
        }

        FORCE_INLINE NODISCARD
        const V& Value() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Value();
        }


        // Iteration

        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            return _index < _dictionary->_capacity;
        }

        FORCE_INLINE
        MutBucketEnumerator& operator++()
        {
            ++_index;
            _index = _dictionary->SkipToOccupied(_index);
            return *this;
        }

        FORCE_INLINE
        MutBucketEnumerator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }
    };

    class ConstBucketEnumerator
    {
        const Dictionary* _dictionary;
        int32 _index;

    public:
        explicit ConstBucketEnumerator(const Dictionary& dictionary)
            : _dictionary{ &dictionary }
            , _index{ 0 }
        {
            _index = _dictionary->SkipToOccupied(_index);
        }
        

        // Access

        FORCE_INLINE NODISCARD
        const Bucket& operator*() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index];
        }

        FORCE_INLINE NODISCARD
        const Bucket* operator->() const
        {
            return &DATA_OF(const Bucket, _dictionary->_allocData)[_index];
        }


        FORCE_INLINE NODISCARD
        const K& Key() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Key();
        }

        FORCE_INLINE NODISCARD
        const V& Value() const
        {
            return DATA_OF(const Bucket, _dictionary->_allocData)[_index].Value();
        }


        // Iteration

        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            return _index < _dictionary->_capacity;
        }

        FORCE_INLINE
        ConstBucketEnumerator& operator++()
        {
            ++_index;
            _index = _dictionary->SkipToOccupied(_index);
            return *this;
        }

        FORCE_INLINE
        ConstBucketEnumerator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }
    };


    FORCE_INLINE NODISCARD
    KeyEnumerator Keys() const
    {
        return KeyEnumerator{ *this };
    }
    
    FORCE_INLINE NODISCARD
    MutValEnumerator Values()
    {
        return MutValEnumerator{ *this };
    }

    FORCE_INLINE NODISCARD
    ConstValEnumerator Values() const
    {
        return ConstValEnumerator{ *this };
    }

    FORCE_INLINE NODISCARD
    MutBucketEnumerator Buckets()
    {
        return MutBucketEnumerator{ this };
    }

    FORCE_INLINE NODISCARD
    ConstBucketEnumerator Buckets() const
    {
        return ConstBucketEnumerator{ this };
    }
};
