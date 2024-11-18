// Created by Mateusz Karbowiak 2024

#pragma once

#include "CollectionsUtils.h"


/// <summary>
/// Basic container for storing dynamically resizable arrays of elements.
/// It stores the elements in a contiguous memory block and uses doubling growth strategy.
/// </summary>
/// <remarks>
/// 1. <c>Array</c> works effectively as a stack. If you need a queue, consider using <c>Ring</c> instead.
/// 2. The container is designed to invoke the allocator as little as possible.
/// Thus it will keep the allocation active even when the array is empty,
/// unless explicitly freed by calling <c>Reset</c>.
/// 3. <c>Array</c> STL inspiration is <c>std::vector</c>.
/// </remarks>
/// <typeparam name="T"> Type of elements stored in the array. Must be move-able, not CV-qualified, nor a reference. </typeparam>
/// <typeparam name="Alloc"> Type of the allocator to use. </typeparam>
template<typename T, typename Alloc = HeapAlloc>
class Array
{
    using AllocData = typename Alloc::Data;

    AllocData _allocData;
    int32     _capacity;
    int32     _count;

public:
    // Capacity Access

    /// <summary> Checks if the array has an active allocation. </summary>
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

    /// <summary> Checks if the array has any elements. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsEmpty() const noexcept -> bool
    {
        return _count == 0;
    }

    /// <summary> Number of currently stored elements. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Count() const noexcept -> int32
    {
        return _count;
    }

    /// <summary> Number of elements that can be added without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Slack() const noexcept -> int32
    {
        return _capacity - _count;
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

        minCapacity = CollectionsUtils::GetAllocCapacity<Alloc, ARRAY_DEFAULT_CAPACITY>(minCapacity);

        const int32 allocatedMemory   = newData.Allocate(sizeof(T) * minCapacity);
        const int32 allocatedCapacity = allocatedMemory / sizeof(T);

        // Move the content before reassigning the capacity
        if (_capacity > 0)
        {
            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(T, _allocData), 
                DATA_OF(T, newData), 
                _count
            );
            _allocData.Free();
        }

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the array is empty, the allocation will be freed.
    /// </summary>
    FORCE_INLINE
    void ShrinkToFit()
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
        const int32 requiredCapacity = CollectionsUtils::GetAllocCapacity<Alloc, ARRAY_DEFAULT_CAPACITY>(
            _count
        );

        if (_capacity <= requiredCapacity)
            return;

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData }; // Copy the binding

        const int32 allocatedMemory   = newData.Allocate(_count * sizeof(T));
        const int32 allocatedCapacity = allocatedMemory / sizeof(T);
        ASSERT_MEMORY(allocatedCapacity >= _count);

        CollectionsUtils::MoveLinearContent<T>(
            DATA_OF(T, _allocData), 
            DATA_OF(T, newData), 
            _count
        );
        _allocData.Free();

        _allocData = MOVE(newData);
        _capacity  = allocatedCapacity;
    }


    // Element Access

    /// <summary> Accesses the element at the given index. </summary>
    FORCE_INLINE NODISCARD
    auto operator[](const int32 index) -> T&
    {
        ASSERT_INDEX(index >= 0 && index < _count);
        return static_cast<T*>(_allocData.Get())[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    FORCE_INLINE NODISCARD
    auto operator[](const int32 index) const -> const T&
    {
        ASSERT_INDEX(index >= 0 && index < _count);
        return static_cast<const T*>(_allocData.Get())[index];
    }


    // Element Manipulation

    /// <summary>
    /// Adds an element to the end of the array, by forwarding it to the constructor.
    /// </summary>
    /// <param name="element"> Element to add. </param>
    template<typename U>
    FORCE_INLINE
    auto Add(U&& element) -> T&
    {
        if (_count == _capacity)
            Reserve(_capacity + 1);

        T* target = static_cast<T*>(_allocData.Get()) + _count;

        // Placement new
        new (target) T(FORWARD(U, element));
        _count += 1;

        return *target;
    }

    /// <summary> Adds an element to the end of the array, by constructing it in-place. </summary>
    /// <param name="args"> Arguments to forward to the constructor. </param>
    template<typename... Args>
    FORCE_INLINE
    auto Emplace(Args&&... args) -> T&
    {
        if (_count == _capacity)
            Reserve(_capacity + 1);

        T* target = static_cast<T*>(_allocData.Get()) + _count;

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
    FORCE_INLINE
    auto Insert(const int32 index, U&& element) -> T&
    {
        ASSERT_INDEX(index >= 0 && index <= _count);  // Allow index == _count for appending

        if (_count == _capacity)
            Reserve(_capacity + 1);

        T* target = static_cast<T*>(_allocData.Get()) + index;

        // If inserting at the end (_count), no need to move any element.
        if (index == _count)
        {
            // Just place the new element at the end.
            new (target) T(FORWARD(U, element));
        }
        else
        {
            // Move the element that will be replaced (if any).
            T* displacedElement = static_cast<T*>(_allocData.Get()) + _count - 1;
            *target = MOVE(*displacedElement);

            // Placement new for the element to be inserted.
            new (target) T(FORWARD(U, element));
        }

        _count += 1;

        return *target;
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
    FORCE_INLINE
    auto InsertStable(const int32 index, U&& element) -> T&
    {
        ASSERT_INDEX(index >= 0 && index <= _count);

        if (_count == _capacity)
            Reserve(_capacity + 1);

        T* target = static_cast<T*>(_allocData.Get()) + index;

        // Shift the elements toward the end.
        for (int32 i = _count; i > index; --i)
            target[i] = MOVE(target[i - 1]);

        // Placement new
        new (target) T(FORWARD(U, element));
        _count += 1;

        return *target;
    }


    /// <summary>
    /// Removes element at the specified index, disregarding the order of the elements.
    /// </summary>
    /// <param name="index"> Index of the element to remove. It must be in the range [0, Count). </param>
    FORCE_INLINE
    auto RemoveAt(const int32 index) -> void
    {
        ASSERT_INDEX(index >= 0 && index < _count);
        T* target = static_cast<T*>(_allocData.Get()) + index;

        // Destruct the element.
        target->~T();

        // If removing from the end, no need to move elements.
        if (index < _count - 1)
        {
            // Move the last element to the removal spot (only if not at the end).
            T* lastElement = static_cast<T*>(_allocData.Get()) + _count - 1;
            *target = MOVE(*lastElement);
        }

        _count -= 1;
    }

    /// <summary>
    /// Removes element at the specified index, preserving the order of the elements by moving them.
    /// </summary>
    /// <param name="index"> Index of the element to remove. It must be in the range [0, Count). </param>
    /// <remarks>
    /// This operation is significantly slower than basic insertion. It should be used only when the order of the elements matters.
    /// </remarks>
    FORCE_INLINE
    auto RemoveAtStable(const int32 index) -> void
    {
        ASSERT_INDEX(index >= 0 && index < _count);
        T* target = static_cast<T*>(_allocData.Get()) + index;

        // Destruct the element.
        target->~T();

        // Shift the elements towarW
        for (int32 i = index; i < _count - 1; ++i)
            target[i] = MOVE(target[i + 1]);

        _count -= 1;
    }


    /// <summary> Removes all elements from the array without freeing the allocation. </summary>
    FORCE_INLINE
    void Clear()
    {
        if (_count == 0)
            return;

        CollectionsUtils::DestroyLinearContent<T>(DATA_OF(T, _allocData), _count);
        _count = 0;
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


    // Collection Lifecycle - Constructors

    /// <summary> Initializes an empty array with no active allocation. </summary>
    constexpr Array() noexcept
        : _allocData{}
        , _capacity{ 0 }
        , _count{ 0 }
    {
    }

    /// <summary> Initializes an array by moving the allocation from another array. </summary>
    Array(Array&& other) noexcept
    {
        if (!other.IsAllocated())
        {
            _allocData = AllocData{};
            _count     = 0;
            _capacity  = 0;
        }
        else if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);
            _count     = other._count;
            _capacity  = other._capacity;

            other._capacity = 0; // Allocation moved - Reset the capacity!
            other._count    = 0;
        }
        else
        {
            _allocData = AllocData{};
            _count     = other._count;

            const int32 allocatedMemory = _allocData.Allocate(sizeof(T) * _count); // Minimal allocation
            _capacity = allocatedMemory / sizeof(T);

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(T, other._allocData), 
                DATA_OF(T, this->_allocData), 
                _count
            );

            other.Reset();
        }
    }

    /// <summary> Initializing an array by moving different allocator is not allowed. </summary>
    /// <remarks> This operation always disables desired allocator move. </remarks>
    template<typename OtherAlloc>
    Array(Array<T, OtherAlloc>&&) = delete;

    /// <summary> Initializes an array by copying another array. </summary>
    template<typename OtherAlloc>
    explicit Array(const Array<T, OtherAlloc>& other)
        : _allocData{}
    {
        static_assert(std::is_copy_constructible<T>::value, "Type must be copy-constructible.");

        if (other._capacity == 0)
        {
            // If no allocation is active, just zero the members.
            _capacity = 0;
            _count    = 0;
        }
        else
        {
            const int32 requiredCapacity = CollectionsUtils::GetAllocCapacity<Alloc, ARRAY_DEFAULT_CAPACITY>(other._count);
            const int32 allocatedMemory  = _allocData.Allocate(requiredCapacity * sizeof(T));

            _capacity = allocatedMemory / sizeof(T);
            _count = other._count;

            using SourceAlloc = OtherAlloc;
            using TargetAlloc = Alloc;

            CollectionsUtils::CopyLinearContent<T>(
                DATA_OF(const T, other._allocData),
                DATA_OF(T,       this->_allocData),
                _count
            );
        }
    }

    /// <summary> Initializes an array by copying another array. </summary>
    Array(const Array& other)
        : Array{ other }
    {
    }


    /// <summary> Initializes an empty array with an active context-less allocation of the specified capacity. </summary>
    explicit Array(const int32 capacity)
        : _allocData{}
        , _count{}
    {
        const int32 requiredCapacity = CollectionsUtils::GetAllocCapacity<Alloc, ARRAY_DEFAULT_CAPACITY>(capacity);
        const int32 allocatedMemory  = _allocData.Allocate(requiredCapacity * sizeof(T));
        _capacity = allocatedMemory / sizeof(T);
    }

    /// <summary> Initializes an empty array with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext>
    explicit Array(const int32 capacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
        , _count{}
    {
        const int32 requiredCapacity = CollectionsUtils::GetAllocCapacity<Alloc, ARRAY_DEFAULT_CAPACITY>(capacity);
        const int32 allocatedMemory  = _allocData.Allocate(requiredCapacity * sizeof(T));
        _capacity = allocatedMemory / sizeof(T);
    }


    // Collection Lifecycle  - Assignments

    FORCE_INLINE
    auto operator=(Array&& other) noexcept -> Array&
    {
        if (this == &other)
            return *this;

        Reset();

        if (!other.IsAllocated()) 
        {
            // Pass
        }
        else if (!other._allocData.MovesItems()) 
        {
            _allocData = MOVE(other._allocData);
            _capacity  = other._capacity;
            _count     = other._count;

            other._capacity = 0; // Allocation moved - Reset the capacity!
            other._count    = 0;
        }
        else
        {
            _count = other._count;

            const int32 allocatedMemory = _allocData.Allocate(sizeof(T) * _count);
            _capacity = allocatedMemory / sizeof(T);

            CollectionsUtils::MoveLinearContent<T>(
                DATA_OF(T, other._allocData), 
                DATA_OF(T, _allocData),
                _count
            );

            other.Reset();
        }

        return *this;
    }

    FORCE_INLINE
    auto operator=(const Array& other) -> Array&
    {
        if (this == &other)
            return *this;

        Reset();

        if (other._capacity != 0)
        {
            // If there's an active allocation, copy the elements.
            const int32 requiredCapacity = CollectionsUtils::GetAllocCapacity<Alloc, ARRAY_DEFAULT_CAPACITY>(other._count);
            const int32 allocatedMemory  = _allocData.Allocate(requiredCapacity * sizeof(T));
            _capacity = allocatedMemory / sizeof(T);
            _count    = other._count;

            ASSERT_MEMORY(_capacity >= requiredCapacity);

            CollectionsUtils::CopyLinearContent<T>(
                DATA_OF(const T, other._allocData), 
                DATA_OF(T,       this->_allocData), 
                _count
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


    // Iterators

    /// <summary>
    /// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
    /// </summary>
    class MutEnumerator
    {
        Array<T, Alloc>* _array;
        int32            _index;

    public:
        explicit MutEnumerator(Array<T, Alloc>& array)
            : _array{ &array }
            , _index{ 0 }
        {
        }


        FORCE_INLINE NODISCARD
        auto operator==(const MutEnumerator& other) const -> bool
        {
            ASSERT(_array == other._array);
            return _index == other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator!=(const MutEnumerator& other) const -> bool
        {
            ASSERT(_array == other._array);
            return _index != other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator<(const MutEnumerator& other) const -> bool
        {
            ASSERT(_array == other._array);
            return _index < other._index;
        }


        FORCE_INLINE NODISCARD
        auto operator*() -> T& { return (*_array)[_index]; }

        FORCE_INLINE NODISCARD
        auto operator->() -> T* { return &(*_array)[_index]; }

        FORCE_INLINE NODISCARD
        auto operator*() const -> const T& { return (*_array)[_index]; }

        FORCE_INLINE NODISCARD
        auto operator->() const -> const T* { return &(*_array)[_index]; }


        /// <summary> Check if the enumerator reached the end of the array. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            ASSERT(_array != nullptr);
            return _index < _array->_count;
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE NODISCARD
        auto Index() const noexcept -> int32
        {
            return _index;
        }


        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++() -> MutEnumerator&
        {
            _index += 1;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++(int) -> MutEnumerator
        {
            MutEnumerator copy{ *this };
            _index += 1;
            return copy;
        }
    };

    /// <summary>
    /// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
    /// </summary>
    class ConstEnumerator
    {
        const Array<T, Alloc>* _array;
        int32                  _index;

    public:
        explicit ConstEnumerator(const Array<T, Alloc>& array)
            : _array{ &array }
            , _index{ 0 }
        {
        }

        explicit ConstEnumerator(const MutEnumerator& enumerator)
            : _array{ enumerator._array }
            , _index{ enumerator._index }
        {
        }


        // Identity

        FORCE_INLINE NODISCARD
        auto operator==(const ConstEnumerator& other) const -> bool
        {
            ASSERT(_array == other._array);
            return _index == other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator!=(const ConstEnumerator& other) const -> bool
        {
            ASSERT(_array == other._array);
            return _index != other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator<(const ConstEnumerator& other) const -> bool
        {
            ASSERT(_array == other._array);
            return _index < other._index;
        }


        // Access

        FORCE_INLINE NODISCARD
        auto operator*() const -> const T& { return (*_array)[_index]; }

        FORCE_INLINE NODISCARD
        auto operator->() const -> const T* { return &(*_array)[_index]; }


        // End Condition and Movement

        /// <summary> Check if the enumerator reached the end of the array. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const noexcept
        {
            ASSERT(_array != nullptr);
            return _index < _array->_count;
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE NODISCARD
        auto Index() const noexcept -> int32
        {
            return _index;
        }


        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++() -> ConstEnumerator&
        {
            _index += 1;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++(int) -> ConstEnumerator
        {
            ConstEnumerator copy{ *this };
            _index += 1;
            return copy;
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



    /// <summary> STL-style begin iterator. </summary>
    FORCE_INLINE NODISCARD
    auto begin() -> T*
    {
        return static_cast<T*>(_allocData.Get());
    }

    /// <summary> STL-style begin iterator. </summary>
    FORCE_INLINE NODISCARD
    auto begin() const -> const T*
    {
        return static_cast<const T*>(_allocData.Get());
    }

    /// <summary> STL-style const begin iterator. </summary>
    FORCE_INLINE NODISCARD
    auto cbegin() const -> const T*
    {
        return static_cast<const T*>(_allocData.Get());
    }


    /// <summary> STL-style end iterator. </summary>
    FORCE_INLINE NODISCARD
    auto end() -> T*
    {
        return static_cast<T*>(_allocData.Get()) + _count;
    }

    /// <summary> STL-style end iterator. </summary>
    FORCE_INLINE NODISCARD
    auto end() const -> const T*
    {
        return static_cast<const T*>(_allocData.Get()) + _count;
    }

    /// <summary> STL-style const end iterator. </summary>
    FORCE_INLINE NODISCARD
    auto cend() const -> const T*
    {
        return static_cast<const T*>(_allocData.Get()) + _count;
    }


    // Constraints

    static_assert(std::is_move_constructible<T>        ::value, "Type must be move-constructible.");
    static_assert(std::is_nothrow_move_constructible<T>::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_move_assignable<T>           ::value, "Type must be move-assignable.");
    static_assert(std::is_nothrow_move_assignable<T>   ::value, "Type must be nothrow move-assignable.");
    static_assert(std::is_destructible<T>              ::value, "Type must be destructible.");
    static_assert(std::is_nothrow_destructible<T>      ::value, "Type must be nothrow destructible.");

    static_assert(!std::is_reference<T>                ::value, "Type must not be a reference type.");
    static_assert(!std::is_const<T>                    ::value, "Type must not be a const-qualified type.");
};
