// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"

/// <summary>
/// A container for dynamically resizable arrays of elements,
/// stored in a single contiguous block of memory.
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
/// 1. The <c>Array</c> class is inspired by STL's <c>std::vector</c> and Unreal Engine's
/// <c>TArray</c>, with a difference that it does not have a specialization for <c>bool</c>.
/// 2. It operates effectively as a stack, with the array's end representing the top of the stack.
/// 3. The amortized time complexity of adding elements is constant.
/// 4. The default capacity is defined by <c>ARRAY_DEFAULT_CAPACITY</c>.
/// 5. The container minimizes allocator invocations, keeping the allocation active even when the
/// array is empty, unless explicitly freed by calling <c>Reset</c> (or destructor).
/// 6. <c>Array</c> is not thread-safe.
/// External synchronization is required if used in a multi-threaded environment.
/// </remarks>
template<
    typename T,
    typename A = HeapAlloc,
    typename G = DefaultGrowth
>
class Array
{
public:
    using Element     = T;
    using AllocData   = typename A::Data;
    using AllocHelper = AllocHelperOf<Element, A, ARRAY_DEFAULT_CAPACITY, G>;

    using MutPuller   = typename Span<Element>::MutPuller;
    using ConstPuller = typename Span<Element>::ConstPuller;

PRIVATE:
    AllocData _allocData{};
    int32     _capacity{};
    int32     _count{};

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
        return _count == 0;
    }

    /// <summary> Number of currently stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Count() const -> int32
    {
        return _count;
    }

    /// <summary> Number of elements that can be added without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Slack() const -> int32
    {
        return _capacity - _count;
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
            if (_count > 0)
            {
                BulkOperations::MoveLinearContent<Element>(
                    DATA_OF(Element, _allocData),
                    DATA_OF(Element, newData),
                    _count
                );
                BulkOperations::DestroyLinearContent<Element>(
                    DATA_OF(Element, _allocData),
                    _count
                );
            }

            _allocData.Free();
            _allocData = MOVE(newData);
            _capacity = allocatedCapacity;
        }
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the array is empty, the allocation will be freed.
    /// </summary>
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

        BulkOperations::MoveLinearContent<Element>(
            DATA_OF(Element, _allocData), 
            DATA_OF(Element, newData), 
            _count
        );
        BulkOperations::DestroyLinearContent<Element>(
            DATA_OF(Element, _allocData),
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
    NO_DISCARD FORCE_INLINE constexpr
    auto Data() -> Element*
    {
        return DATA_OF(Element, _allocData);
    }

    /// <summary>
    /// Accesses the first element of the array.
    /// To be used with <c>Count</c> for C-style API, where the first element is at index 0.
    /// </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Data() const -> const Element*
    {
        return DATA_OF(const Element, _allocData);
    }


    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator[](const int32 index) -> Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < _count);
        return DATA_OF(Element, _allocData)[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator[](const int32 index) const -> const Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < _count);
        return DATA_OF(const Element, _allocData)[index];
    }


    // Element Manipulation

    /// <summary>
    /// Adds an element to the end of the array, by forwarding it to the constructor.
    /// </summary>
    /// <param name="element"> Element to add. </param>
    template<typename U> // Universal reference
    MAY_DISCARD FORCE_INLINE
    auto Add(U&& element) -> Element&
    {
        static_assert(
            TIsSameV<TRemoveCVRefT<U>, Element>,
            "Add requires explicit usage of element type. If not intended, consider using emplacement."
        );

        if (_count == _capacity)
            Reserve(_capacity + 1);

        Element* target = DATA_OF(Element, _allocData) + _count;

        // Placement new
        new (target) Element(FORWARD(U, element));
        _count += 1;

        return *target;
    }

    /// <summary> Adds an element to the end of the array, by constructing it in-place. </summary>
    /// <param name="args"> Arguments to forward to the constructor. </param>
    template<typename... Args> // Parameter pack
    MAY_DISCARD FORCE_INLINE
    auto Emplace(Args&&... args) -> Element&
    {
        if (_count == _capacity)
            Reserve(_capacity + 1);

        Element* target = DATA_OF(Element, _allocData) + _count;

        // Placement new
        new (target) Element(FORWARD(Args, args)...);
        _count += 1;

        return *target;
    }


    /// <summary>
    /// Adds element at the specified index, disregarding the order of the elements.
    /// </summary>
    /// <param name="index"> Index at which to insert the element. It must be in the range [0, Count]. </param>
    /// <param name="element"> Element to add. </param>
    /// <returns> Reference to the added element. </returns>
    template<typename U> // Universal reference
    MAY_DISCARD FORCE_INLINE
    auto InsertAt(const int32 index, U&& element) -> Element&
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index <= _count);  // Allow index == _count for appending

        if (_count == _capacity)
            Reserve(_capacity + 1);

        // Pointer to the slot at the insertion point.
        Element* insertPtr = DATA_OF(Element, _allocData) + index;

        if (index < _count)
        {
            // Pointer to the newly occupied slot.
            Element* endPtr = DATA_OF(Element, _allocData) + _count;

            // Move-construct of the last element to the end.
            new (endPtr) Element(MOVE(*insertPtr));

            // Move-assignment of the inserted element.
            *insertPtr = Element(FORWARD(U, element));
        }
        else
        {
            // Move-construct the element at the end.
            new (insertPtr) Element(FORWARD(U, element));
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
    template<typename U> // Universal reference
    MAY_DISCARD
    auto InsertAtStable(const int32 index, U&& element) -> Element&
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index <= _count);  // Allow index == _count for appending

        // Technically, we could reduce number of moves for relocation.
        // However, it would complicate the code even more. A task for another day.
        if (_count == _capacity)
            Reserve(_capacity + 1);

        // Pointer to the first slot.
        Element* dataPtr = DATA_OF(Element, _allocData);

        // Pointer to the slot at the insertion point.
        Element* insertPtr = dataPtr + index;

        if (index < _count)
        {
            // Pointer to the newly occupied slot.
            Element* endPtr = dataPtr + _count;

            // Move-construct of the last element to the end.
            new (endPtr) Element(MOVE(*(endPtr - 1)));

            // Move the elements to make space for the new element.
            for (int i = _count - 1; i > index; --i)
            {
                dataPtr[i] = MOVE(dataPtr[i - 1]);
            }

            // Move-assignment of the inserted element.
            dataPtr[index] = Element(FORWARD(U, element));
        }
        else
        {
            // Move-construct the element at the end.
            new (insertPtr) Element(FORWARD(U, element));
        }

        _count += 1;
        return *insertPtr;
    }


    /// <summary>
    /// Removes element at the specified index, disregarding the order of the elements.
    /// </summary>
    /// <param name="index"> Index of the element to remove. It must be in the range [0, Count). </param>
    FORCE_INLINE
    void RemoveAt(const int32 index)
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index < _count); // Ensure index is valid

        Element* basePtr    = DATA_OF(Element, _allocData);
        Element* removedPtr = basePtr + index;

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
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index < _count); // Ensure index is valid

        Element* basePtr    = DATA_OF(Element, _allocData);

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

        BulkOperations::DestroyLinearContent<Element>(DATA_OF(Element, _allocData), _count);
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
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() noexcept -> Span<Element>
    {
        return Span<Element>{ DATA_OF(Element, _allocData), _count };
    }

    /// <summary> Creates a read-only span of the stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() const noexcept -> Span<const Element>
    {
        return Span<const Element>{ DATA_OF(const Element, _allocData), _count };
    }

    /// <summary>
    /// Adds one-by-one copies of the specified elements to the end of the array.
    /// Max one allocation is performed.
    /// </summary>
    /// <remarks>
    /// Array has specialized method for adding multiple elements at once,
    /// because they may be returned from a function as a span.
    /// </remarks>
    MAY_DISCARD
    auto AddElements(const Element* source, const int32 count) -> Span<Element>
    {
        const int32 newCount = _count + count;
        Reserve(newCount);

        Element* target = DATA_OF(Element, _allocData) + _count;
        BulkOperations::CopyLinearContent<Element>(source, target, count);

        _count = newCount;
        return Span<Element>{ target, count };
    }

    /// <summary>
    /// Adds one-by-one copies of the specified elements to the end of the array.
    /// Max one allocation is performed.
    /// </summary>
    /// <remarks>
    /// Array has specialized method for adding multiple elements at once,
    /// because they may be returned from a function as a span.
    /// </remarks>
    MAY_DISCARD
    auto AddElements(const Span<Element> source) -> Span<Element>
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
    MAY_DISCARD
    auto AddRepetitions(const Element& source, const int32 count) -> Span<Element>
    {
        const int32 newCount = _count + count;
        Reserve(newCount);

        for (int i = 0; i < count; ++i) {
            Add(source);
        }

        const int32 startIndex = _count - count;
        const Element* startPtr = DATA_OF(Element, _allocData) + startIndex;

        return Span<Element>{ startPtr, count };
    }


protected:
    void MoveToEmpty(Array&& other) noexcept
    {
        ASSERT_COLLECTION_SAFE_MOD(_count == 0 && _capacity == 0); // Array must be empty, but the collection must be initialized!

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

            BulkOperations::MoveLinearContent<Element>(
                DATA_OF(Element, other._allocData),
                DATA_OF(Element, this->_allocData),
                _count
            );

            other.Reset();
        }
    }


    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty array with no active allocation. </summary>
    FORCE_INLINE constexpr
    Array() = default;

    /// <summary> Initializes an array by moving the allocation from another array. </summary>
    FORCE_INLINE constexpr
    Array(Array&& other) noexcept
    {
        MoveToEmpty(MOVE(other));
    }

    /// <summary> Initializes an array by copying another array. </summary>
    FORCE_INLINE constexpr
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
    FORCE_INLINE explicit
    Array(const int32 capacity)
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }

    /// <summary> Initializes an empty array with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext> // Universal reference
    FORCE_INLINE explicit
    Array(const int32 capacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
    {
        const int32 requiredCapacity = AllocHelper::InitCapacity(capacity);
        _capacity = AllocHelper::Allocate(_allocData, requiredCapacity);
    }


    // Collection Lifecycle  - Assignments

    FORCE_INLINE
    auto operator=(Array&& other) noexcept -> Array&
    {
        if (this != &other) 
        {
            Reset();
            MoveToEmpty(MOVE(other));
        }
        return *this;
    }

    auto operator=(const Array& other) -> Array&
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
    NO_DISCARD static constexpr
    auto Of(std::initializer_list<U> list) -> Array<Element>
    {
        const int32 capacity = static_cast<int32>(list.size());
        Array<Element> result{ capacity };

        for (const auto& element : list)
            result.Add(element);

        return result;
    }


    // Pullers

    /// <summary> Creates a cursor for the array. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() -> MutPuller
    {
        Element* data = DATA_OF(Element, _allocData);
        return MutPuller{ data, data + _count };
    }

    /// <summary> Creates a cursor for the array. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() const -> ConstPuller
    {
        const Element* data = DATA_OF(const Element, _allocData);
        return ConstPuller{ data, data + _count };
    }


    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto begin() -> Element*
    {
        return DATA_OF(Element, _allocData);
    }

    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto begin() const -> const Element*
    {
        return DATA_OF(const Element, _allocData);
    }

    /// <summary> STL-style const begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto cbegin() const -> const Element*
    {
        return DATA_OF(const Element, _allocData);
    }


    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto end() -> Element*
    {
        return DATA_OF(Element, _allocData) + _count;
    }

    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto end() const -> const Element*
    {
        return DATA_OF(const Element, _allocData) + _count;
    }

    /// <summary> STL-style const end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto cend() const -> const Element*
    {
        return DATA_OF(const Element, _allocData) + _count;
    }


    // Constraints

    REQUIRE_TYPE_NOT_REFERENCE(Element);
    REQUIRE_TYPE_NOT_CONST(Element);
    REQUIRE_TYPE_MOVEABLE(Element);
};
