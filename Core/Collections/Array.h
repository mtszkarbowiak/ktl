// Created by Mateusz Karbowiak 2024

#pragma once

#include "Collections/CollectionsUtils.h"


/// <summary>
/// Basic container for storing dynamically resizable arrays of elements in one contiguous memory block.
/// </summary>
///
/// <typeparam name="T"> Type of elements stored in the array. Must be move-able, not CV-qualified, and not a reference. </typeparam>
/// <typeparam name="Alloc"> Type of the allocator to use. Can be either dragging or non-dragging.</typeparam>
/// <typeparam name="Grow"> Function to calculate the next capacity (before capping by allocator). </typeparam>
///
/// <remarks>
/// 1. <c>Array</c> works effectively as a stack. If you need a queue, consider using <c>Ring</c> instead.
/// 2. The container is designed to invoke the allocator as little as possible.
/// Thus it will keep the allocation active even when the array is empty,
/// unless explicitly freed by calling <c>Reset</c>.
/// 3. <c>Array</c> STL inspiration is <c>std::vector</c>.
/// </remarks>
template<
    typename T,
    typename Alloc = HeapAlloc,
    int32(&Grow)(int32) = Growing::Default
>
class Array
{
    using AllocData   = typename Alloc::Data;
    using AllocHelper = AllocHelperOf<T, Alloc, ARRAY_DEFAULT_CAPACITY, Grow>;

    AllocData _allocData{};
    int32     _capacity{};
    int32     _count{};

    // Capacity Access

public:
    /// <summary> Checks if the array has an active allocation. </summary>
    FORCE_INLINE NODISCARD
    constexpr bool IsAllocated() const 
    {
        return _capacity > 0;
    }

    /// <summary> Number of elements that can be stored without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr int32 Capacity() const 
    {
        return _capacity;
    }


    // Count Access

    /// <summary> Checks if the array has any elements. </summary>
    FORCE_INLINE NODISCARD
    constexpr bool IsEmpty() const 
    {
        return _count == 0;
    }

    /// <summary> Number of currently stored elements. </summary>
    FORCE_INLINE NODISCARD
    constexpr int32 Count() const 
    {
        return _count;
    }

    /// <summary> Number of elements that can be added without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr int32 Slack() const 
    {
        return _capacity - _count;
    }


    // Allocation Manipulation

    /// <summary> Ensures that adding items up to the requested capacity will not invoke the allocator. </summary>
    FORCE_INLINE
    void Reserve(const int32 minCapacity)
    {
        if (minCapacity < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        if (minCapacity <= _capacity)
            return; // Reserving the same capacity would not increase the capacity.


        if (_capacity == 0)
        {
            // If the array is empty, allocate the default capacity.
            const int32 requiredCapacity = AllocHelper::InitCapacity(minCapacity);
            _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
        }
        else
        {
            // Higher capacity is required. Allocate new memory.
            const AllocData& oldData = _allocData;
            AllocData newData{ oldData };

            const int32 requiredCapacity = AllocHelper::NextCapacity(_capacity, minCapacity);
            const int32 allocatedCapacity = AllocHelper::Allocate(newData, requiredCapacity);

            // Move the content before reassigning the capacity
            if (_capacity > 0)
            {
                BulkOperations::MoveLinearContent<T>(
                    DATA_OF(T, _allocData),
                    DATA_OF(T, newData),
                    _count
                );
                BulkOperations::DestroyLinearContent<T>(
                    DATA_OF(T, _allocData),
                    _count
                );

                _allocData.Free();
            }

            _allocData = MOVE(newData);
            _capacity = allocatedCapacity;
        }
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the array is empty, the allocation will be freed.
    /// </summary>
    FORCE_INLINE
    void Compact()
    {
        // Check if there is possibility of relocation.
        if (_count == 0)
        {
            // If the array is empty, free the allocation.
            if (_capacity > 0)
            {
                _allocData.Free();
                _capacity = 0;
            }
            return;
        }

        // Test required capacity against the current capacity.
        const int32 requiredCapacity = AllocHelper::InitCapacity(_count);

        if (_capacity <= requiredCapacity)
            return;

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData }; // Copy the binding

        const int32 allocatedCapacity = AllocHelper::Allocate(newData, requiredCapacity);

        BulkOperations::MoveLinearContent<T>(
            DATA_OF(T, _allocData), 
            DATA_OF(T, newData), 
            _count
        );
        BulkOperations::DestroyLinearContent<T>(
            DATA_OF(T, _allocData),
            _count
        );

        _allocData.Free();

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;
    }


    // Element Access

    /// <summary>
    /// Accesses the first element of the array.
    /// To be used with <c>Count</c> for C-style API, where the first element is at index 0.
    /// </summary>
    FORCE_INLINE NODISCARD
    T* Data()
    {
        return DATA_OF(T, _allocData);
    }

    /// <summary>
    /// Accesses the first element of the array.
    /// To be used with <c>Count</c> for C-style API, where the first element is at index 0.
    /// </summary>
    FORCE_INLINE NODISCARD
    const T* Data() const
    {
        return DATA_OF(const T, _allocData);
    }


    /// <summary> Accesses the element at the given index. </summary>
    FORCE_INLINE
    T& operator[](const int32 index)
    {
        ASSERT_INDEX(index >= 0 && index < _count);
        return DATA_OF(T, _allocData)[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    FORCE_INLINE
    const T& operator[](const int32 index) const
    {
        ASSERT_INDEX(index >= 0 && index < _count);
        return DATA_OF(const T, _allocData)[index];
    }


    // Element Manipulation

    /// <summary>
    /// Adds an element to the end of the array, by forwarding it to the constructor.
    /// </summary>
    /// <param name="element"> Element to add. </param>
    template<typename U>
    FORCE_INLINE
    T& Add(U&& element)
    {
        static_assert(
            std::is_same<typename std::decay<U>::type, T>::value,
            "Add requires explicit usage of element type. If not intended, consider using emplacement."
        );

        if (_count == _capacity)
            Reserve(_capacity + 1);

        T* target = DATA_OF(T, _allocData) + _count;

        // Placement new
        new (target) T(FORWARD(U, element));
        _count += 1;

        return *target;
    }

    /// <summary> Adds an element to the end of the array, by constructing it in-place. </summary>
    /// <param name="args"> Arguments to forward to the constructor. </param>
    template<typename... Args>
    FORCE_INLINE
    T& Emplace(Args&&... args)
    {
        if (_count == _capacity)
            Reserve(_capacity + 1);

        T* target = DATA_OF(T, _allocData) + _count;

        // Placement new
        new (target) T(FORWARD(Args, args)...);
        _count += 1;

        return *target;
    }


    /// <summary>
    /// Adds element at the specified index, disregarding the order of the elements.
    /// </summary>
    /// <param name="index"> Index at which to insert the element. It must be in the range [0, Count]. </param>
    /// <param name="element"> Element to add. </param>
    /// <returns> Reference to the added element. </returns>
    template<typename U>
    T& InsertAt(const int32 index, U&& element)
    {
        ASSERT_INDEX(index >= 0 && index <= _count);  // Allow index == _count for appending

        if (_count == _capacity)
            Reserve(_capacity + 1);

        // Pointer to the slot at the insertion point.
        T* insertPtr = DATA_OF(T, _allocData) + index;

        if (index < _count)
        {
            // Pointer to the newly occupied slot.
            T* endPtr = DATA_OF(T, _allocData) + _count;

            // Move-construct of the last element to the end.
            new (endPtr) T(MOVE(*insertPtr));

            // Move-assignment of the inserted element.
            *insertPtr = T(FORWARD(U, element));
        }
        else
        {
            // Move-construct the element at the end.
            new (insertPtr) T(FORWARD(U, element));
        }

        _count += 1;
        return *insertPtr;
    }

    /// <summary>
    /// Adds element at the specified index, preserving the order of the elements by moving them.
    /// </summary>
    /// <param name="index"> Index at which to insert the element. It must be in the range [0, Count]. </param>
    /// <param name="element"> Element to add. </param>
    /// <returns> Reference to the added element. </returns>
    /// <remarks>
    /// This operation is significantly slower than basic insertion. It should be used only when the order of the elements matters.
    /// </remarks>
    template<typename U>
    T& InsertAtStable(const int32 index, U&& element)
    {
        ASSERT_INDEX(index >= 0 && index <= _count);  // Allow index == _count for appending

        // Technically, we could reduce number of moves for relocation.
        // However, it would complicate the code even more. A task for another day.
        if (_count == _capacity)
            Reserve(_capacity + 1);

        // Pointer to the first slot.
        T* dataPtr = DATA_OF(T, _allocData);

        // Pointer to the slot at the insertion point.
        T* insertPtr = dataPtr + index;

        if (index < _count)
        {
            // Pointer to the newly occupied slot.
            T* endPtr = dataPtr + _count;

            // Move-construct of the last element to the end.
            new (endPtr) T(MOVE(*(endPtr - 1)));

            // Move the elements to make space for the new element.
            for (int i = _count - 1; i > index; --i)
            {
                dataPtr[i] = MOVE(dataPtr[i - 1]);
            }

            // Move-assignment of the inserted element.
            dataPtr[index] = T(FORWARD(U, element));
        }
        else
        {
            // Move-construct the element at the end.
            new (insertPtr) T(FORWARD(U, element));
        }

        _count += 1;
        return *insertPtr;
    }


    /// <summary>
    /// Removes element at the specified index, disregarding the order of the elements.
    /// </summary>
    /// <param name="index"> Index of the element to remove. It must be in the range [0, Count). </param>
    void RemoveAt(const int32 index)
    {
        ASSERT_INDEX(index >= 0 && index < _count); // Ensure index is valid

        T* basePtr    = DATA_OF(T, _allocData);
        T* removedPtr = basePtr + index;

        if (index != _count - 1)  // If not removing the last element
        {
            *removedPtr = MOVE(basePtr[_count - 1]); // Replace with the last element
        }

        basePtr[_count - 1].~T(); // Destroy the last element
        _count -= 1;
    }

    /// <summary>
    /// Removes element at the specified index, preserving the order of the elements by moving them.
    /// </summary>
    /// <param name="index"> Index of the element to remove. It must be in the range [0, Count). </param>
    /// <remarks>
    /// This operation is significantly slower than basic insertion. It should be used only when the order of the elements matters.
    /// </remarks>
    void RemoveAtStable(const int32 index)
    {
        ASSERT_INDEX(index >= 0 && index < _count); // Ensure index is valid

        T* basePtr    = DATA_OF(T, _allocData);

        // Shift all subsequent elements left to fill the gap
        for (int32 i = index; i < _count - 1; ++i)
        {
            basePtr[i] = MOVE(basePtr[i + 1]);
        }

        basePtr[_count - 1].~T(); // Destroy the last (now duplicated) element
        _count -= 1;
    }


    /// <summary> Removes all elements from the array without freeing the allocation. </summary>
    FORCE_INLINE
    void Clear()
    {
        if (_count == 0)
            return;

        BulkOperations::DestroyLinearContent<T>(DATA_OF(T, _allocData), _count);
        _count = 0;
    }

    /// <summary> Removes all elements from the array and frees the allocation. </summary>
    FORCE_INLINE
    void Reset() noexcept
    {
        if (_capacity == 0)
            return;

        Clear();

        _allocData.Free(); // Capacity is above zero!
        _capacity = 0;
    }


    /// <summary> Creates a span of the stored elements. </summary>
    FORCE_INLINE NODISCARD
    Span<T> AsSpan() noexcept
    {
        return Span<T>{ DATA_OF(T, _allocData), _count };
    }

    /// <summary>
    /// Adds one-by-one copies of the specified elements to the end of the array.
    /// Max one allocation is performed.
    /// </summary>
    /// <remarks>
    /// Array has specialized method for adding multiple elements at once,
    /// because they may be returned from a function as a span.
    /// </remarks>
    Span<T> AddElements(const T* source, const int32 count)
    {
        const int32 newCount = _count + count;
        Reserve(newCount);

        T* target = DATA_OF(T, _allocData) + _count;
        BulkOperations::CopyLinearContent<T>(source, target, count);

        _count = newCount;
        return Span<T>{ target, count };
    }

    /// <summary>
    /// Adds one-by-one copies of the specified elements to the end of the array.
    /// Max one allocation is performed.
    /// </summary>
    /// <remarks>
    /// Array has specialized method for adding multiple elements at once,
    /// because they may be returned from a function as a span.
    /// </remarks>
    Span<T> AddElements(const Span<T> source)
    {
        return AddElements(source.Data(), source.Count());
    }

    /// <summary>
    /// Adds copies of one and the same element to the end of the array.
    /// </summary>
    /// <remarks>
    /// Array has specialized method for adding multiple elements at once,
    /// because they may be returned from a function as a span.
    /// </remarks>
    Span<T> AddRepetitions(const T& source, const int32 count)
    {
        const int32 newCount = _count + count;
        Reserve(newCount);

        for (int i = 0; i < count; ++i) {
            Add(source);
        }

        const int32 startIndex = _count - count;
        const T* startPtr = DATA_OF(T, _allocData) + startIndex;

        return Span<T>{ startPtr, count };
    }


protected:
    FORCE_INLINE
    void MoveToEmpty(Array&& other) noexcept
    {
        ASSERT(_count == 0 && _capacity == 0); // Array must be empty, but the collection must be initialized!

        if (other._capacity == 0 || other._count == 0)
            return;

        if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);
            _count     = other._count;
            _capacity  = other._capacity;

            // The items have been moved with the allocator.
            // The capacity must be reset manually.
            other._capacity = 0;
            other._count    = 0;
        }
        else 
        {
            const int32 requestedCapacity = AllocHelper::InitCapacity(other._count);

            _capacity  = AllocHelper::Allocate(_allocData, requestedCapacity);
            _count     = other._count;

            BulkOperations::MoveLinearContent<T>(
                DATA_OF(T, other._allocData),
                DATA_OF(T, this->_allocData),
                _count
            );

            other.Reset();
        }
    }


    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty array with no active allocation. </summary>
    FORCE_INLINE
    constexpr Array() = default;

    /// <summary> Initializes an array by moving the allocation from another array. </summary>
    FORCE_INLINE
    Array(Array&& other) noexcept
    {
        MoveToEmpty(MOVE(other));
    }

    /// <summary> Initializes an array by copying another array. </summary>
    Array(const Array& other)
    {
        if (other._count == 0)
            return;

        AddElements(
            other.Data(),
            other._count
        );
    }

    /// <summary> Initializes an empty array with an active context-less allocation of the specified capacity. </summary>
    FORCE_INLINE
    explicit Array(const int32 capacity)
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }

    /// <summary> Initializes an empty array with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext>
    FORCE_INLINE
    explicit Array(const int32 capacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }


    // Collection Lifecycle  - Assignments

    FORCE_INLINE
    Array& operator=(Array&& other) noexcept
    {
        if (this != &other) 
        {
            Reset();
            MoveToEmpty(MOVE(other));
        }
        return *this;
    }

    Array& operator=(const Array& other)
    {
        if (this != &other) 
        {
            Reset();
            AddElements(
                other.Data(),
                other._count
            );
        }
        return *this;
    }


    // Collection Lifecycle - Destructor

    /// <summary> Destructor. </summary>
    ~Array()
    {
        Reset();
    }


    // Factorization

    /// <summary> Creates an array with the specified elements. </summary>
    template<typename U>
    static Array<T> Of(std::initializer_list<U> list)
    {
        const int32 capacity = static_cast<int32>(list.size());
        Array<T> result{ capacity };

        for (const auto& element : list)
            result.Add(element);

        return result;
    }


    // Iterators

    /// <summary>
    /// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
    /// </summary>
    class MutEnumerator
    {
        Array* _array;
        int32  _index;

    public:
        explicit MutEnumerator(Array& array)
            : _array{ &array }
            , _index{ 0 }
        {
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        FORCE_INLINE NODISCARD
        IterHint Hint() const
        {
            const int32 remaining = _array->_count - _index;
            return { remaining, remaining };
        }

        FORCE_INLINE T& operator*()
        {
            return (*_array)[_index];
        }

        FORCE_INLINE T* operator->()
        {
            return &(*_array)[_index];
        }

        FORCE_INLINE const T& operator*() const
        {
            return (*_array)[_index];
        }

        FORCE_INLINE const T* operator->() const
        {
            return &(*_array)[_index];
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE NODISCARD
        int32 Index() const 
        {
            return _index;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const 
        {
            ASSERT(_array != nullptr);
            return _index < _array->_count;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        MutEnumerator& operator++()
        {
            _index += 1;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        FORCE_INLINE
        MutEnumerator operator++(int)
        {
            MutEnumerator copy{ *this };
            _index += 1;
            return copy;
        }


        // Identity

        FORCE_INLINE NODISCARD
        bool operator==(const MutEnumerator& other) const
        {
            ASSERT(_array == other._array);
            return _index == other._index;
        }

        FORCE_INLINE NODISCARD
        bool operator!=(const MutEnumerator& other) const
        {
            ASSERT(_array == other._array);
            return _index != other._index;
        }

        FORCE_INLINE NODISCARD
        bool operator<(const MutEnumerator& other) const
        {
            ASSERT(_array == other._array);
            return _index < other._index;
        }
    };

    /// <summary>
    /// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
    /// </summary>
    class ConstEnumerator
    {
        const Array* _array;
        int32        _index;

    public:
        explicit ConstEnumerator(const Array& array)
            : _array{ &array }
            , _index{ 0 }
        {
        }

        explicit ConstEnumerator(const MutEnumerator& enumerator)
            : _array{ enumerator._array }
            , _index{ enumerator._index }
        {
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        FORCE_INLINE NODISCARD
        IterHint Hint() const
        {
            const int32 remaining = _array->_count - _index;
            return { remaining, remaining };
        }

        FORCE_INLINE
        const T& operator*() const
        {
            return (*_array)[_index];
        }

        FORCE_INLINE
        const T* operator->() const
        {
            return &(*_array)[_index];
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE NODISCARD
        int32 Index() const 
        {
            return _index;
        }


        // Iteration

        /// <summary> Check if the enumerator points to a valid element. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const 
        {
            ASSERT(_array != nullptr);
            return _index < _array->_count;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        ConstEnumerator& operator++()
        {
            _index += 1;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        /// <remarks> Prefixed increment operator is faster. </remarks>
        FORCE_INLINE
        ConstEnumerator operator++(int)
        {
            ConstEnumerator copy{ *this };
            _index += 1;
            return copy;
        }


        // Identity

        FORCE_INLINE NODISCARD
        bool operator==(const ConstEnumerator& other) const
        {
            ASSERT(_array == other._array);
            return _index == other._index;
        }

        FORCE_INLINE NODISCARD
        bool operator!=(const ConstEnumerator& other) const
        {
            ASSERT(_array == other._array);
            return _index != other._index;
        }

        FORCE_INLINE NODISCARD
        bool operator<(const ConstEnumerator& other) const
        {
            ASSERT(_array == other._array);
            return _index < other._index;
        }
    };

    /// <summary> Creates an enumerator for the array. </summary>
    FORCE_INLINE NODISCARD
    MutEnumerator Values()
    {
        return MutEnumerator{ *this };
    }

    /// <summary> Creates an enumerator for the array. </summary>
    FORCE_INLINE NODISCARD
    ConstEnumerator Values() const
    {
        return ConstEnumerator{ *this };
    }



    /// <summary> STL-style begin iterator. </summary>
    FORCE_INLINE NODISCARD
    T* begin()
    {
        return DATA_OF(T, _allocData);
    }

    /// <summary> STL-style begin iterator. </summary>
    FORCE_INLINE NODISCARD
    const T* begin() const
    {
        return DATA_OF(const T, _allocData);
    }

    /// <summary> STL-style const begin iterator. </summary>
    FORCE_INLINE NODISCARD
    const T* cbegin() const
    {
        return DATA_OF(const T, _allocData);
    }


    /// <summary> STL-style end iterator. </summary>
    FORCE_INLINE NODISCARD
    T* end()
    {
        return DATA_OF(T, _allocData) + _count;
    }

    /// <summary> STL-style end iterator. </summary>
    FORCE_INLINE NODISCARD
    const T* end() const
    {
        return DATA_OF(const T, _allocData) + _count;
    }

    /// <summary> STL-style const end iterator. </summary>
    FORCE_INLINE NODISCARD
    const T* cend() const
    {
        return DATA_OF(const T, _allocData) + _count;
    }


    // Constraints
    static_assert(!std::is_reference<T>                ::value, "Type must not be a reference type.");
    static_assert(!std::is_const<T>                    ::value, "Type must not be a const-qualified type.");

    static_assert(std::is_move_constructible<T>        ::value, "Type must be move-constructible.");
    static_assert(std::is_destructible<T>              ::value, "Type must be destructible.");
    static_assert(std::is_nothrow_move_constructible<T>::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_nothrow_destructible<T>      ::value, "Type must be nothrow destructible.");
};
