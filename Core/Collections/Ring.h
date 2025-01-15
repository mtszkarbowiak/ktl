// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"

/// <summary>
/// A double-ended container for storing dynamically resizable queues of elements.
/// The elements are stored in one or two segments, depending on the head-tail relationship.
/// Both segments share a single contiguous memory block.
/// </summary>
///
/// <typeparam name="T">
/// The type of elements stored in the array.
/// Must be movable (both constructor and assignment), non-const, and non-reference.
/// </typeparam>
/// <typeparam name="A">
/// (Optional) The type of the allocator to use.
/// Can be either a dragging or non-dragging allocator.
/// </typeparam>
/// <typeparam name="G">
/// (Optional) A reference to a function that calculates the next capacity
/// before applying allocator limits.
/// </typeparam>
///
/// <remarks>
/// 1. The <c>Ring</c> class has similar API to STL's <c>std::deque</c>,
/// but uses a single memory block instead of multiple blocks.
/// 2. It operates effectively as a queue, with two ends representing the front and back of the
/// queue. It is recommended to use the <c>PushBack</c> as the primary method for adding elements.
/// 3. The amortized time complexity of adding elements is constant.
/// 4. The default capacity is defined by <c>RING_DEFAULT_CAPACITY</c>.
/// 5. The container minimizes allocator invocations, keeping the allocation active even when the
/// array is empty, unless explicitly freed by calling <c>Reset</c> (or destructor).
/// 6. <c>Ring</c> is not thread-safe.
/// External synchronization is required if used in a multi-threaded environment.
/// </remarks>
template<
    typename T,
    typename A = DefaultAlloc,
    int32(&G)(int32) = Growing::Default
>
class Ring
{
    using Element     = T;
    using AllocData   = typename A::Data;
    using AllocHelper = AllocHelperOf<Element, A, RING_DEFAULT_CAPACITY, G>;

    AllocData _allocData{};
    int32     _capacity{};
    int32     _head{};
    int32     _tail{}; // Points to the next FREE slot (not the last element).
    int32     _countCached{};

    // Internal Integrity

protected:
    /// <summary> Checks if the ring is in a valid state. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsValid() const -> bool
    {
        // This test is only valid if the capacity is non-zero.
        if (_capacity == 0)
        {
            return true;
        }

        // Check if the cached count is correct.
        const int32 expectedTail = (_head + _countCached) % _capacity;
        return _tail == expectedTail;
    }


    // Capacity Access

public:
    /// <summary> Checks if the ring has an active allocation. </summary>
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

    /// <summary> Checks if the ring has any elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsEmpty() const -> bool
    {
        return _head == _tail;
    }

    /// <summary> Number of currently stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Count() const -> int32
    {
        return _countCached;
    }

    /// <summary> Number of elements that can be added without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Slack() const -> int32
    {
        return _capacity - _countCached;
    }

protected:
    /// <summary> Index of the first element in the ring. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Head() const -> int32
    {
        return _head;
    }

    /// <summary> Index of the next free slot in the ring. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Tail() const -> int32
    {
        return _tail;
    }

public:
    /// <summary> Checks if the ring is wrapped around the capacity, meaning that elements are stored in two segments. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsWrapped() const -> bool
    {
        return _head > _tail;
    }


    // Allocation Manipulation

    /// <summary> Ensures that adding items up to the requested capacity will not invoke the allocator. </summary>
    void Reserve(const int32 minCapacity)
    {
        if (minCapacity < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        if (minCapacity <= _capacity)
            return; // Reserving the same capacity would not increase the capacity.

        if (_capacity == 0)
        {
            const int32 requiredCapacity = AllocHelper::InitCapacity(minCapacity);
            const int32 allocatedCapacity = AllocHelper::Allocate(_allocData, requiredCapacity);

            _capacity = allocatedCapacity;
            _head = 0;
            _tail = 0;
            _countCached = 0;

            ASSERT_COLLECTION_INTEGRITY(IsValid());
            return;
        }

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData };

        const int32 requiredCapacity = AllocHelper::NextCapacity(_capacity, minCapacity);
        const int32 allocatedCapacity = AllocHelper::Allocate(newData, requiredCapacity);

        // Move the content before reassigning the capacity
        const bool isWrapped = _head > _tail;
        if (!isWrapped)
        {
            // Straightforward case: continuous data
            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                DATA_OF(Element, newData),
                _countCached
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                _countCached
            );
        }
        else
        {
            const int32 wrapIndex = _capacity - _head;

            // Move and destroy the first wrapped segment
            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                DATA_OF(Element, newData),
                wrapIndex
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                wrapIndex
            );

            // Move and destroy the second segment
            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, _allocData),
                DATA_OF(Element, newData) + wrapIndex,
                _tail
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData),
                _tail
            );
        }

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;

        _head      = 0;
        _tail      = _countCached;

        ASSERT_COLLECTION_INTEGRITY(IsValid());
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the ring is empty, the allocation will be freed.
    /// </summary>
    void Compact()
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
        const int32 requiredCapacity = AllocHelper::InitCapacity(_countCached); //TODO Review.

        if (_capacity <= requiredCapacity)
            return;

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData };

        const int32 allocatedCapacity = AllocHelper::Allocate(newData, requiredCapacity);

        // Move the content before reassigning the capacity
        const bool isWrapped = _head > _tail;
        if (!isWrapped)
        {
            // Straightforward case: continuous data
            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                DATA_OF(Element, newData),
                _countCached
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                _countCached
            );
        }
        else
        {
            const int32 wrapIndex = _capacity - _head;

            // Move and destroy the first wrapped segment
            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                DATA_OF(Element, newData),
                wrapIndex
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                wrapIndex
            );

            // Move and destroy the second segment
            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, _allocData),
                DATA_OF(Element, newData) + wrapIndex,
                _tail
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData),
                _tail
            );
        }

        _head = 0;
        _tail = _countCached;

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;

        ASSERT_COLLECTION_INTEGRITY(IsValid());
    }


    // Element Access

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator[](const int32 index) -> Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < _countCached);
        const int32 realIndex = (_head + index) % _capacity;
        return DATA_OF(Element, _allocData)[realIndex];
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator[](const int32 index) const -> const Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < _countCached);
        const int32 realIndex = (_head + index) % _capacity;
        return DATA_OF(const Element, _allocData)[realIndex];
    }


    /// <summary> Accesses the first element in the ring. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto PeekFront() -> Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_countCached > 0); // Ring must not be empty!
        return DATA_OF(Element, _allocData)[_head];
    }

    /// <summary> Accesses the first element in the ring. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto PeekFront() const -> const Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_countCached > 0); // Ring must not be empty!
        return DATA_OF(const Element, _allocData)[_head];
    }

    /// <summary> Accesses the last element in the ring. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto PeekBack() -> Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_countCached > 0); // Ring must not be empty!
        const int32 index = (_capacity + _tail - 1) % _capacity;
        return DATA_OF(Element, _allocData)[index];
    }

    /// <summary> Accesses the last element in the ring. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto PeekBack() const -> const Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_countCached > 0); // Ring must not be empty!
        const int32 index = (_capacity + _tail - 1) % _capacity;
        return DATA_OF(const Element, _allocData)[index];
    }


    // Element Manipulation

    /// <summary> Adds an element to the end of the ring. </summary>
    template<typename U> // Universal reference
    MAY_DISCARD
    auto PushBack(U&& element) -> Element&
    {
        static_assert(
            std::is_same<typename std::decay<U>::type, Element>::value,
            "PushBack requires explicit usage of element type. If not intended, consider using emplacement."
        );

        Reserve(_countCached + 1);

        Element* target = DATA_OF(Element, _allocData) + _tail;

        new (target) Element(FORWARD(U, element));
        _tail = (_tail + 1) % _capacity;
        _countCached += 1;

        ASSERT_COLLECTION_INTEGRITY(IsValid());

        return *target;
    }

    /// <summary> Adds an element to the end of the ring. </summary>
    template<typename... Args> // Parameter pack
    MAY_DISCARD
    auto EmplaceBack(Args&&... args) -> Element&
    {
        Reserve(_countCached + 1);

        Element* target = DATA_OF(Element, _allocData) + _tail;

        new (target) Element(FORWARD(Args, args)...);
        _tail = (_tail + 1) % _capacity;
        _countCached += 1;

        ASSERT_COLLECTION_INTEGRITY(IsValid());

        return *target;
    }


    /// <summary> Adds an element to the beginning of the ring. </summary>
    template<typename U> // Universal reference
    MAY_DISCARD
    auto PushFront(U&& element) -> Element&
    {
        static_assert(
            std::is_same<typename std::decay<U>::type, Element>::value,
            "PushBack requires explicit usage of element type. If not intended, consider using emplacement."
        );

        Reserve(_countCached + 1);

        _head = (_head - 1 + _capacity) % _capacity;
        Element* target = DATA_OF(Element, _allocData) + _head;
        new (target) Element(FORWARD(U, element));
        _countCached += 1;

        ASSERT_COLLECTION_INTEGRITY(IsValid());

        return *target;
    }

    /// <summary> Adds an element to the beginning of the ring. </summary>
    template<typename... Args> // Parameter pack
    MAY_DISCARD
    auto EmplaceFront(Args&&... args) -> Element&
    {
        Reserve(_countCached + 1);

        _head = (_head - 1 + _capacity) % _capacity;
        Element* target = DATA_OF(Element, _allocData) + _head;
        new (target) Element(FORWARD(Args, args)...);
        _countCached += 1;

        ASSERT_COLLECTION_INTEGRITY(IsValid());

        return *target;
    }


    //TODO(mtszkarbowiak) Add InsertAt and InsertAtStable
    //TODO(mtszkarbowiak) Add RemoveAt and RemoveAtStable


    /// <summary> Removes the last element from the ring. </summary>
    FORCE_INLINE
    void PopBack()
    {
        ASSERT_COLLECTION_SAFE_MOD(_countCached > 0); // Ring must not be empty!
        _tail = (_tail - 1 + _capacity) % _capacity;
        Element* target = DATA_OF(Element, _allocData) + _tail;
        target->~T();
        _countCached -= 1;

        ASSERT_COLLECTION_INTEGRITY(IsValid());
    }

    /// <summary> Removes the first element from the ring. </summary>
    FORCE_INLINE
    void PopFront()
    {
        ASSERT_COLLECTION_SAFE_MOD(_countCached > 0); // Ring must not be empty!
        Element* target = DATA_OF(Element, _allocData) + _head;
        target->~T();
        _head = (_head + 1) % _capacity;
        _countCached -= 1;

        ASSERT_COLLECTION_INTEGRITY(IsValid());
    }


    /// <summary> Removes all elements from the array without freeing the allocation. </summary>
    void Clear()
    {
        if (_countCached == 0)
            return;

        const bool isWrapped = _head > _tail;
        if (!isWrapped)
        {
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                _countCached
            );
        }
        else
        {
            const int32 wrapIndex = _capacity - _head;

            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData) + _head,
                wrapIndex
            );
            BulkOperations::DestroyLinearContent<Element>(
                DATA_OF(Element, _allocData),
                _tail
            );
        }


        _head        = 0;
        _tail        = 0;
        _countCached = 0;

        ASSERT_COLLECTION_INTEGRITY(IsValid());
    }

    /// <summary> Removes all elements from the array and frees the allocation. </summary>
    FORCE_INLINE constexpr
    void Reset()
    {
        if (_capacity == 0)
            return;

        Clear();

        _allocData.Free(); // Capacity is above zero!
        _capacity = 0;
    }


protected:
    void MoveToEmpty(Ring&& other) noexcept
    {
        ASSERT_COLLECTION_SAFE_MOD(_countCached == 0 && _capacity == 0); // Ring must be empty, but the collection must be initialized!
        ASSERT_COLLECTION_INTEGRITY(other.IsValid()); // Make sure the other ring is valid.

        if (other._capacity == 0 || other._countCached == 0)
            return;

        if (other._allocData.MovesItems())
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
            const int32 requiredCapacity = AllocHelper::InitCapacity(other._countCached);

            _allocData   = AllocData{};
            _capacity    = AllocHelper::Allocate(_allocData, requiredCapacity);
            _countCached = other._countCached;
            _head        = 0;
            _tail        = other._countCached;

            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, other._allocData) + other._head,
                DATA_OF(Element, _allocData),
                other._countCached
            );

            other.Reset();
        }
        else
        {
            const int32 requiredCapacity = AllocHelper::InitCapacity(other._countCached);

            _allocData   = AllocData{};
            _capacity    = AllocHelper::Allocate(_allocData, requiredCapacity);
            _countCached = other._countCached;
            _head        = 0;
            _tail        = other._countCached;

            const int32 wrapIndex = other._capacity - other._head;

            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, other._allocData) + other._head,
                DATA_OF(Element, _allocData),
                wrapIndex
            );

            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, other._allocData),
                DATA_OF(Element, _allocData) + wrapIndex,
                other._tail
            );

            other.Reset();
        }
    }

    void CopyToEmpty(const Ring& other)
    {
        static_assert(std::is_copy_constructible<Element>::value, "Type must be copy-constructible.");

        ASSERT_COLLECTION_SAFE_MOD(_countCached == 0 && _capacity == 0); // Ring must be empty, but the collection must be initialized!
        ASSERT_COLLECTION_INTEGRITY(other.IsValid()); // Make sure the other ring is valid.

        if (other._capacity == 0 || other._countCached == 0)
            return;

        if (!other.IsWrapped())
        {

            const int32 requiredCapacity = AllocHelper::InitCapacity(other._countCached);

            _capacity    = AllocHelper::Allocate(_allocData, requiredCapacity);
            _head        = 0;
            _tail        = other._countCached;
            _countCached = other._countCached;

            BulkOperations::CopyLinearContent<Element>(
                DATA_OF(const Element, other._allocData) + other._head,
                DATA_OF(Element, this->_allocData),
                other._countCached
            );
        }
        else
        {
            const int32 requiredCapacity = AllocHelper::InitCapacity(other._countCached);

            _capacity    = AllocHelper::Allocate(_allocData, requiredCapacity);
            _head        = 0;
            _tail        = other._countCached;
            _countCached = other._countCached;

            const int32 wrapIndex = other._capacity - other._head;

            BulkOperations::CopyLinearContent<Element>(
                DATA_OF(const Element, other._allocData) + other._head,
                DATA_OF(Element, this->_allocData),
                wrapIndex
            );

            BulkOperations::CopyLinearContent<Element>(
                DATA_OF(const Element, other._allocData),
                DATA_OF(Element, this->_allocData) + wrapIndex,
                other._tail
            );
        }
    }



    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty ring with no active allocation. </summary>
    constexpr FORCE_INLINE
    Ring() = default;

    /// <summary> Initializes a ring by moving the allocation from another array. </summary>
    FORCE_INLINE
    Ring(Ring&& other) noexcept
    {
        MoveToEmpty(MOVE(other));
    }

    /// <summary> Initializes a ring by copying another ring. </summary>
    FORCE_INLINE
    Ring(const Ring& other)
    {
        CopyToEmpty(other);
    }


    /// <summary> Initializes an empty ring with an active context-less allocation of the specified capacity. </summary>
    FORCE_INLINE explicit
    Ring(const int32 capacity)
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }

    /// <summary> Initializes an empty ring with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext> // Universal reference
    FORCE_INLINE explicit
    Ring(const int32 capacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }


    // Collection Lifecycle - Assignments

    MAY_DISCARD FORCE_INLINE
    auto operator=(Ring&& other) noexcept -> Ring&
    {
        if (this != &other) 
        {
            Reset();
            MoveToEmpty(MOVE(other));
        }

        return *this;
    }

    MAY_DISCARD FORCE_INLINE
    auto operator=(const Ring& other) -> Ring&
    {
        if (this != &other) 
        {
            Reset();
            CopyToEmpty(other);
        }

        return *this;
    }


    // Collection Lifecycle - Destructor

    ~Ring()
    {
        Reset();
    }


    // Factorization

    /// <summary> Creates an array with the specified elements. </summary>
    template<typename U>
    NO_DISCARD static constexpr
    auto Of(std::initializer_list<U> list) -> Ring<Element>
    {
        const int32 capacity = static_cast<int32>(list.size());
        Ring<Element> result{ capacity };

        for (const auto& element : list)
            result.PushBack(element);

        return result;
    }


    // Cursors

    class MutCursor
    {
        Ring* _ring;
        int32 _indexOfElement;
        int32 _indexOfSlot;


    public:
        FORCE_INLINE explicit
        MutCursor(Ring& ring)
            : _ring{ &ring }
            , _indexOfElement{ 0 }
            , _indexOfSlot{ ring.Head() }
        {
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        NO_DISCARD FORCE_INLINE
        auto Hint() const -> SizeHint
        {
            const int32 remaining = _ring->Count() - _indexOfElement;
            return { remaining, remaining };
        }

        NO_DISCARD FORCE_INLINE
        auto operator*() -> Element&
        {
            return DATA_OF(Element, _ring->_allocData)[_indexOfSlot];
        }

        NO_DISCARD FORCE_INLINE
        auto operator->() -> Element*
        {
            return DATA_OF(Element, _ring->_allocData) + _indexOfSlot;
        }

        NO_DISCARD FORCE_INLINE
        auto operator*() const -> const Element&
        {
            return DATA_OF(const Element, _ring->_allocData)[_indexOfSlot];
        }

        NO_DISCARD FORCE_INLINE
        auto operator->() const -> const Element*
        {
            return DATA_OF(const Element, _ring->_allocData) + _indexOfSlot;
        }

        /// <summary> Returns the index of the current element. </summary>
        NO_DISCARD FORCE_INLINE
        auto Index() const -> int32
        {
            return _indexOfElement;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        NO_DISCARD FORCE_INLINE explicit
        operator bool() const 
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring != nullptr);
            return _indexOfElement < _ring->_countCached;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        MAY_DISCARD FORCE_INLINE
        auto operator++() -> MutCursor&
        {
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> MutCursor
        {
            MutCursor copy{ *this };
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return copy;
        }


        // Identity

        NO_DISCARD FORCE_INLINE
        auto operator==(const MutCursor& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring == other._ring);
            return _indexOfElement == other._indexOfElement;
        }

        NO_DISCARD FORCE_INLINE
        auto operator!=(const MutCursor& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring == other._ring);
            return _indexOfElement != other._indexOfElement;
        }

        NO_DISCARD FORCE_INLINE
        auto operator<(const MutCursor& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring == other._ring);
            return _indexOfElement < other._indexOfElement;
        }
    };

    class ConstCursor
    {
        const Ring* _ring;
        int32       _indexOfElement;
        int32       _indexOfSlot;

    public:
        FORCE_INLINE explicit
        ConstCursor(const Ring& ring)
            : _ring{ &ring }
            , _indexOfElement{ 0 }
            , _indexOfSlot{ ring.Head() }
        {
        }

        FORCE_INLINE explicit
        ConstCursor(const MutCursor& other)
            : _ring{ other._ring }
            , _indexOfElement{ other._indexOfElement }
            , _indexOfSlot{ other._indexOfSlot }
        {
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        NO_DISCARD FORCE_INLINE
        auto Hint() const -> SizeHint
        {
            const int32 remaining = _ring->Count() - _indexOfElement;
            return { remaining, remaining };
        }

        NO_DISCARD FORCE_INLINE
        auto operator*() const -> const Element&
        {
            return DATA_OF(const Element, _ring->_allocData)[_indexOfSlot];
        }

        NO_DISCARD FORCE_INLINE
        auto operator->() const -> const Element*
        {
            return DATA_OF(const Element, _ring->_allocData) + _indexOfSlot;
        }

        /// <summary> Returns the index of the current element. </summary>
        NO_DISCARD FORCE_INLINE
        auto Index() const -> int32
        {
            return _indexOfElement;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        NO_DISCARD FORCE_INLINE explicit
        operator bool() const
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring != nullptr);
            return _indexOfElement < _ring->_countCached;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        MAY_DISCARD FORCE_INLINE
        auto operator++() -> ConstCursor&
        {
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        MAY_DISCARD FORCE_INLINE
        auto operator++(int) -> ConstCursor
        {
            ConstCursor copy{ *this };
            _indexOfElement += 1;
            _indexOfSlot = (_indexOfSlot + 1) % _ring->_capacity;
            return copy;
        }


        // Identity

        NO_DISCARD FORCE_INLINE
        auto operator==(const ConstCursor& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring == other._ring);
            return _indexOfElement == other._indexOfElement;
        }

        NO_DISCARD FORCE_INLINE
        auto operator!=(const ConstCursor& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring == other._ring);
            return _indexOfElement != other._indexOfElement;
        }

        NO_DISCARD FORCE_INLINE
        auto operator<(const ConstCursor& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_ring == other._ring);
            return _indexOfElement < other._indexOfElement;
        }
    };

    /// <summary> Creates a read-write cursor for the ring. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() -> MutCursor
    {
        return MutCursor{ *this };
    }

    /// <summary> Creates a read-only cursor for the ring. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() const -> ConstCursor
    {
        return ConstCursor{ *this };
    }


    // Constraints

    static_assert(std::is_move_constructible<Element>        ::value, "Type must be move-constructible.");
    static_assert(std::is_move_assignable<Element>           ::value, "Type must be move-assignable.");
    static_assert(std::is_destructible<Element>              ::value, "Type must be destructible.");
    static_assert(std::is_nothrow_move_constructible<Element>::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_nothrow_move_assignable<Element>   ::value, "Type must be nothrow move-assignable.");
    static_assert(std::is_nothrow_destructible<Element>      ::value, "Type must be nothrow destructible.");

    static_assert(!std::is_reference<Element>                ::value, "Type must not be a reference type.");
    static_assert(!std::is_const<Element>                    ::value, "Type must not be a const-qualified type.");
};
