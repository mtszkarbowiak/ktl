// Created by Mateusz Karbowiak 2024

#pragma once

#include "Collections/CollectionsUtils.h"

/// <summary>
/// Double-ended container for storing dynamically resizable queues of elements.
/// It uses a single contiguous memory block, growing by doubling the capacity.
/// The items are stored in one or two segments, depending on the head-tail relationship.
/// </summary>
/// <remarks>
/// 1. <c>Ring</c> works effectively as a queue. If you need a stack, consider using <c>Array</c> instead.
/// 2. The container is designed to invoke the allocator as little as possible.
/// Thus it will keep the allocation active even when the array is empty,
/// unless explicitly freed by calling <c>Reset</c>.
/// 3. <c>Ring</c> STL inspiration is <c>std::deque</c>.
/// </remarks>
template<typename T, typename Alloc = HeapAlloc>
class Ring
{
    using AllocData = typename Alloc::Data;

    AllocData _allocData;
    int32     _capacity;
    int32     _head;
    int32     _tail; // Points to the next FREE slot (not the last element).
    int32     _countCached;


public:
    // Capacity Access

    /// <summary> Checks if the ring has an active allocation. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsAllocated() const noexcept -> bool
    {
        return _capacity > 0;
    }

    /// <summary> Number of elements that can be stored without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Capacity() const noexcept -> int32
    {
        return _capacity;
    }


    // Count Access

    /// <summary> Checks if the ring has any elements. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsEmpty() const noexcept -> bool
    {
        return _head == _tail;
    }

    /// <summary> Number of currently stored elements. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Count() const noexcept -> int32
    {
        return _countCached;
    }

    /// <summary> Number of elements that can be added without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Slack() const noexcept -> int32
    {
        return _capacity - _countCached;
    }

protected:
    /// <summary> Index of the first element in the ring. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Head() const noexcept -> int32
    {
        return _head;
    }

    /// <summary> Index of the next free slot in the ring. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Tail() const noexcept -> int32
    {
        return _tail;
    }

public:
    /// <summary> Checks if the ring is wrapped around the capacity, meaning that elements are stored in two segments. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsWrapped() const noexcept -> bool
    {
        return _head > _tail;
    }


    // Allocation Manipulation

    /// <summary> Ensures that adding items up to the requested capacity will not invoke the allocator. </summary>
    FORCE_INLINE
    void Reserve(int32 minCapacity)
    {
        if (minCapacity < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        if (minCapacity <= _capacity)
            return; // Reserving the same capacity would not increase the capacity.

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData };

        minCapacity = CollectionsUtils::GetAllocCapacity<Alloc, RING_DEFAULT_CAPACITY>(minCapacity);

        const int32 allocatedMemory = newData.Allocate(sizeof(T) * minCapacity);
        const int32 allocatedCapacity = allocatedMemory / sizeof(T);

        // Move the content before reassigning the capacity

        const bool isWrapped = _head > _tail;
        if (isWrapped)
        {
            // First segment: [head, capacity)
            const int32 firstBeginIndex  = 0;
            const int32 firstEndIndex    = _tail;

            CollectionsUtils::MoveLinearContent<T, Alloc, Alloc>(
                _allocData, newData, firstEndIndex, firstBeginIndex
            );

            // Second segment: [0, tail)
            const int32 secondBeginIndex = _head;
            const int32 secondEndIndex   = _capacity;

            CollectionsUtils::MoveLinearContent<T, Alloc, Alloc>(
                _allocData, newData, secondEndIndex, secondBeginIndex
            );
        }
        else
        {
            const int32 beginIndex = _head;
            const int32 endIndex   = _tail;

            CollectionsUtils::MoveLinearContent<T, Alloc, Alloc>(
                _allocData, newData, endIndex, beginIndex
            );
        }

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the ring is empty, the allocation will be freed.
    /// </summary>
    FORCE_INLINE
    void ShrinkToFit()
    {
        // Check if there is possibility of relocation.
        if (_countCached == 0)
        {
            // If the ring is empty, free the allocation.
            if (_capacity > 0)
            {
                _allocData.Free();
                _capacity = 0;
            }
            return;
        }

        // Test required capacity against the current capacity.
        const int32 requiredCapacity = CollectionsUtils::GetAllocCapacity<Alloc, RING_DEFAULT_CAPACITY>(
            _countCached
        );

        if (_capacity <= requiredCapacity)
            return;

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData };

        const int32 allocatedMemory   = newData.Allocate(sizeof(T) * requiredCapacity);
        const int32 allocatedCapacity = allocatedMemory / sizeof(T);

        // Move the content before reassigning the capacity
        const bool isWrapped = _head > _tail;
        if (isWrapped)
        {
            const int32 wrapIndex = _capacity - _head;

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(T, _allocData) + _head,
                DATA_OF(T, newData),
                wrapIndex
            );

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(T, _allocData),
                DATA_OF(T, newData) + wrapIndex,
                _tail
            );
        }
        else
        {
            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(T, _allocData) + _head,
                DATA_OF(T, newData), // Move to the beginning.
                _countCached
            );

        }

        _head = 0;
        _tail = _countCached;

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;
    }


    // Element Access

    /// <summary> Accesses the element at the given index. </summary>
    auto operator[](const int32 index) -> T&
    {
        ASSERT_INDEX(index >= 0 && index < _countCached);
        const int32 realIndex = (_head + index) % _capacity;
        return DATA_OF(T, _allocData)[realIndex];
    }

    /// <summary> Accesses the element at the given index. </summary>
    auto operator[](const int32 index) const -> const T&
    {
        ASSERT_INDEX(index >= 0 && index < _countCached);
        const int32 realIndex = (_head + index) % _capacity;
        return DATA_OF(const T, _allocData)[realIndex];
    }


    /// <summary> Accesses the first element in the ring. </summary>
    FORCE_INLINE NODISCARD
    auto PeekFront() -> T&
    {
        ASSERT(_countCached > 0); // Ring must not be empty!
        return DATA_OF(T, _allocData)[_head];
    }

    /// <summary> Accesses the first element in the ring. </summary>
    FORCE_INLINE NODISCARD
    auto PeekFront() const -> const T&
    {
        ASSERT(_countCached > 0); // Ring must not be empty!
        return DATA_OF(const T, _allocData)[_head];
    }

    /// <summary> Accesses the last element in the ring. </summary>
    FORCE_INLINE NODISCARD
    auto PeekBack() -> T&
    {
        ASSERT(_countCached > 0); // Ring must not be empty!
        const int32 index = (_capacity + _tail - 1) % _capacity;
        return DATA_OF(T, _allocData)[index];
    }

    /// <summary> Accesses the last element in the ring. </summary>
    auto PeekBack() const -> const T&
    {
        ASSERT(_countCached > 0); // Ring must not be empty!
        const int32 index = (_capacity + _tail - 1) % _capacity;
        return DATA_OF(const T, _allocData)[index];
    }


    // Element Manipulation

    /// <summary> Adds an element to the end of the ring. </summary>
    template<typename U>
    FORCE_INLINE
    auto PushBack(U&& element) -> U&
    {
        if (_countCached == _capacity)
            Reserve(_capacity + 1);

        T* target = static_cast<T*>(_allocData.Get()) + _tail;

        new (target) T(FORWARD(U, element));
        _tail = (_tail + 1) % _capacity;
        _countCached += 1;

        return *target;
    }

    /// <summary> Removes the last element from the ring. </summary>
    template<typename U>
    FORCE_INLINE
    auto PushFront(U&& element) -> U&
    {
        if (_countCached == _capacity)
            Reserve(_capacity + 1);

        _head = (_head - 1 + _capacity) % _capacity;
        T* target = static_cast<T*>(_allocData.Get()) + _head;
        new (target) T(FORWARD(U, element));
        _countCached += 1;

        return *target;
    }

    /// <summary> Removes the last element from the ring. </summary>
    FORCE_INLINE
    void PopBack()
    {
        ASSERT(_countCached > 0); // Ring must not be empty!
        _tail = (_tail - 1 + _capacity) % _capacity;
        T* target = static_cast<T*>(_allocData.Get()) + _tail;
        target->~T();
        _countCached -= 1;
    }

    /// <summary> Removes the first element from the ring. </summary>
    FORCE_INLINE
    void PopFront()
    {
        ASSERT(_countCached > 0); // Ring must not be empty!
        T* target = static_cast<T*>(_allocData.Get()) + _head;
        target->~T();
        _head = (_head + 1) % _capacity;
        _countCached -= 1;
    }


    /// <summary> Removes all elements from the array without freeing the allocation. </summary>
    FORCE_INLINE
    void Clear()
    {
        if (_countCached == 0)
            return;

        const bool isWrapped = _head > _tail;
        if (isWrapped)
        {
            BulkOperations::DestroyLinearContent<T>(DATA_OF(T, _allocData),  _tail);
            BulkOperations::DestroyLinearContent<T>(DATA_OF(T, _allocData) + _head, _capacity);
        }
        else
        {
            BulkOperations::DestroyLinearContent<T>(DATA_OF(T, _allocData) + _head, _countCached);
        }

        _head        = 0;
        _tail        = 0;
        _countCached = 0;
    }

    /// <summary> Removes all elements from the array and frees the allocation. </summary>
    FORCE_INLINE
    void Reset()
    {
        if (_capacity == 0)
            return;

        Clear();

        _allocData.Free(); // Capacity is above zero!
        _capacity = 0;
    }


    // Collection Lifecycle - Moves and Copies

private:
    void MoveFrom(Ring&& other) noexcept
    {
        if (!other.IsAllocated())
        {
            _allocData   = AllocData{};
            _capacity    = 0;
            _head        = 0;
            _tail        = 0;
            _countCached = 0;
        }
        else if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);

            _head        = other._head;
            _tail        = other._tail;
            _capacity    = other._capacity;
            _countCached = other._countCached;

            other._capacity    = 0; // Allocation moved - Reset the capacity!
            other._head        = 0;
            other._tail        = 0;
            other._countCached = 0;
        }
        else if (!other.IsWrapped())
        {
            const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<Alloc, RING_DEFAULT_CAPACITY>(other._countCached);

            _allocData   = AllocData{};
            _capacity    = CollectionsUtils::AllocateCapacity<T, Alloc>(_allocData, requiredCapacity);
            _countCached = other._countCached;
            _head        = 0;
            _tail        = other._countCached;

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(const T, other._allocData) + other._head,
                DATA_OF(T, _allocData),
                other._countCached
            );

            other.Reset();
        }
        else
        {
            const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<Alloc, RING_DEFAULT_CAPACITY>(other._countCached);

            _allocData   = AllocData{};
            _capacity    = CollectionsUtils::AllocateCapacity<T, Alloc>(_allocData, requiredCapacity);
            _countCached = other._countCached;
            _head        = 0;
            _tail        = other._countCached;

            const int32 wrapIndex = other._capacity - other._head;

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(const T, other._allocData) + other._head,
                DATA_OF(T, _allocData),
                wrapIndex
            );

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(const T, other._allocData),
                DATA_OF(T, _allocData) + wrapIndex,
                other._tail
            );
        }
    }

    template<typename OtherAllocation>
    void CopyFrom(const Ring<T, OtherAllocation>& other)
    {
        static_assert(std::is_copy_constructible<T>::value, "Type must be copy-constructible.");

        if (other.IsEmpty()) 
        {
            _allocData   = AllocData{};
            _capacity    = 0;
            _head        = 0;
            _tail        = 0;
            _countCached = 0;
        }
        else if (!other.IsWrapped())
        {

            const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<Alloc, RING_DEFAULT_CAPACITY>(other._countCached);
            _allocData   = AllocData{};
            _capacity    = CollectionsUtils::AllocateCapacity<T, Alloc>(_allocData, requiredCapacity);
            _countCached = other._countCached;
            _head        = 0;
            _tail        = other._countCached;

            CollectionsUtils::CopyLinearContent<T>(
                DATA_OF(const T, other._allocData) + other._head,
                DATA_OF(T, _allocData),
                other._countCached
            );
        }
        else
        {
            const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<Alloc, RING_DEFAULT_CAPACITY>(other._countCached);
            _allocData   = AllocData{};
            _capacity    = CollectionsUtils::AllocateCapacity<T, Alloc>(_allocData, requiredCapacity);
            _countCached = other._countCached;
            _head        = 0;
            _tail        = other._countCached;

            const int32 wrapIndex = other._capacity - other._head;

            CollectionsUtils::CopyLinearContent<T>(
                DATA_OF(const T, other._allocData) + other._head,
                DATA_OF(T, _allocData),
                wrapIndex
            );

            CollectionsUtils::CopyLinearContent<T>(
                DATA_OF(const T, other._allocData),
                DATA_OF(T, _allocData) + wrapIndex,
                other._tail
            );
        }
    }



    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty ring with no active allocation. </summary>
    constexpr Ring() noexcept
        : _allocData{}
        , _capacity{}
        , _head{}
        , _tail{}
        , _countCached{}
    {
    }

    /// <summary> Initializes a ring by moving the allocation from another array. </summary>
    Ring(Ring&& other) noexcept
    {
        MoveFrom(MOVE(other));
    }

    /// <summary> Initializes a ring by copying another ring. </summary>
    template<typename U = T, typename = typename std::enable_if<((std::is_copy_constructible<T>::value&& std::is_same<U, T>::value))>::type>
    Ring(const Ring& other)
    {
        CopyFrom<Alloc>(other);
    }


    /// <summary> Initializes an empty ring with an active context-less allocation of the specified capacity. </summary>
    explicit Ring(const int32 capacity)
        : _allocData{}
        , _head{}
        , _tail{}
        , _countCached{}
    {
        const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<T, Alloc, RING_DEFAULT_CAPACITY>(capacity);
        _capacity = CollectionsUtils::AllocateCapacity<T, Alloc>(_allocData, requiredCapacity);
    }

    /// <summary> Initializes an empty ring with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext>
    explicit Ring(const int32 capacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
        , _head{}
        , _tail{}
        , _countCached{}
    {
        const int32 requiredCapacity = CollectionsUtils::GetRequiredCapacity<T, Alloc, RING_DEFAULT_CAPACITY>(capacity);
        _capacity = CollectionsUtils::AllocateCapacity<T, Alloc>(_allocData, requiredCapacity);
    }


    // Collection Lifecycle - Assignments

    FORCE_INLINE
    auto operator=(Ring&& other) noexcept -> Ring&
    {
        if (this != &other) 
        {
            Reset();
            MoveFrom(MOVE(other));
        }

        return *this;
    }

    template<typename U = T, typename = typename std::enable_if<((std::is_copy_constructible<T>::value&& std::is_same<U, T>::value))>::type>
    FORCE_INLINE
    auto operator=(const Ring& other) -> Ring&
    {
        if (this != &other) 
        {
            Reset();
            CopyFrom<Alloc>(other);
        }

        return *this;
    }


    // Collection Lifecycle - Destructor

    ~Ring()
    {
        Reset();
    }


    // Iterators

    class MutEnumerator
    {
        Ring* _ring;
        int32 _indexOfElement;
        int32 _indexOfSlot;


    public:
        explicit MutEnumerator(Ring& ring)
            : _ring{ &ring }
            , _indexOfElement{ 0 }
            , _indexOfSlot{ ring.Head() }
        {
        }


        // Access

        FORCE_INLINE NODISCARD
        auto operator*() -> T&
        {
            return DATA_OF(T, _ring->_allocData)[_indexOfSlot];
        }

        FORCE_INLINE NODISCARD
        auto operator->() -> T*
        {
            return DATA_OF(T, _ring->_allocData) + _indexOfSlot;
        }

        FORCE_INLINE NODISCARD
        auto operator*() const -> const T&
        {
            return DATA_OF(const T, _ring->_allocData)[_indexOfSlot];
        }

        FORCE_INLINE NODISCARD
        auto operator->() const -> const T*
        {
            return DATA_OF(const T, _ring->_allocData) + _indexOfSlot;
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE NODISCARD
        auto Index() const noexcept -> int32
        {
            return _indexOfElement;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            ASSERT(_ring != nullptr);
            return _indexOfElement < _ring->_countCached;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++() -> MutEnumerator&
        {
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        auto operator++(int) -> MutEnumerator
        {
            MutEnumerator copy{ *this };
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return copy;
        }


        // Identity

        auto operator==(const MutEnumerator& other) const -> bool
        {
            ASSERT(_ring == other._ring);
            return _indexOfElement == other._indexOfElement;
        }

        auto operator!=(const MutEnumerator& other) const -> bool
        {
            ASSERT(_ring == other._ring);
            return _indexOfElement != other._indexOfElement;
        }

        auto operator<(const MutEnumerator& other) const -> bool
        {
            ASSERT(_ring == other._ring);
            return _indexOfElement < other._indexOfElement;
        }
    };

    class ConstEnumerator
    {
        const Ring* _ring;
        int32       _indexOfElement;
        int32       _indexOfSlot;

    public:
        explicit ConstEnumerator(const Ring& ring)
            : _ring{ &ring }
            , _indexOfElement{ 0 }
            , _indexOfSlot{ ring.Head() }
        {
        }

        explicit ConstEnumerator(const MutEnumerator& other)
            : _ring{ other._ring }
            , _indexOfElement{ other._indexOfElement }
            , _indexOfSlot{ other._indexOfSlot }
        {
        }


        // Access

        FORCE_INLINE NODISCARD
        auto operator*() const -> const T&
        {
            return DATA_OF(const T, _ring->_allocData)[_indexOfSlot];
        }

        FORCE_INLINE NODISCARD
        auto operator->() const -> const T*
        {
            return DATA_OF(const T, _ring->_allocData) + _indexOfSlot;
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE NODISCARD
        auto Index() const noexcept -> int32
        {
            return _indexOfElement;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            ASSERT(_ring != nullptr);
            return _indexOfElement < _ring->_countCached;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++() -> ConstEnumerator&
        {
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        auto operator++(int) -> ConstEnumerator
        {
            ConstEnumerator copy{ *this };
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return copy;
        }


        // Identity

        auto operator==(const ConstEnumerator& other) const -> bool
        {
            ASSERT(_ring == other._ring);
            return _indexOfElement == other._indexOfElement;
        }

        auto operator!=(const ConstEnumerator& other) const -> bool
        {
            ASSERT(_ring == other._ring);
            return _indexOfElement != other._indexOfElement;
        }

        auto operator<(const ConstEnumerator& other) const -> bool
        {
            ASSERT(_ring == other._ring);
            return _indexOfElement < other._indexOfElement;
        }
    };

    /// <summary> Creates an enumerator for the array. </summary>
    FORCE_INLINE NODISCARD
    auto Enumerate() -> MutEnumerator
    {
        return MutEnumerator{ *this };
    }

    /// <summary> Creates an enumerator for the array. </summary>
    FORCE_INLINE NODISCARD
    auto Enumerate() const -> ConstEnumerator
    {
        return ConstEnumerator{ *this };
    }


    // Constraints

    static_assert(std::is_move_constructible<T>        ::value, "Type must be move-constructible.");
    static_assert(std::is_move_assignable<T>           ::value, "Type must be move-assignable.");
    static_assert(std::is_destructible<T>              ::value, "Type must be destructible.");
    static_assert(std::is_nothrow_move_constructible<T>::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_nothrow_move_assignable<T>   ::value, "Type must be nothrow move-assignable.");
    static_assert(std::is_nothrow_destructible<T>      ::value, "Type must be nothrow destructible.");

    static_assert(!std::is_reference<T>                ::value, "Type must not be a reference type.");
    static_assert(!std::is_const<T>                    ::value, "Type must not be a const-qualified type.");
};
