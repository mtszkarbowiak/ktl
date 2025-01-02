// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

/// <summary>
/// Non-owning view of a contiguous sequence of elements.
/// </summary>
/// <remarks>
/// Span is classified as null if it points to <c>null</c> OR its points to zero elements.
/// <remarks>
template<typename T>
class Span
{
    T*    _data{};
    int32 _count{};


    // Constructors

public:
    /// <summary> Initializes an empty span. </summary>
    FORCE_INLINE
    Span() = default;

    /// <summary> Initializes a span with the specified data and size. </summary>
    FORCE_INLINE
    Span(T* data, const int32 size)
        : _data{ data }
        , _count{ size }
    {
        ASSERT_COLLECTION_SAFE_MOD(size >= 0); // Span size must be non-negative.
        ASSERT_COLLECTION_SAFE_MOD((size == 0) || (data != nullptr)); // Span data must be valid if the size is non-zero.
    }

    /// <summary> Initializes a span between the specified begin and end pointers. </summary>
    FORCE_INLINE
    Span(T* begin, T* end)
        : _data{ begin }
        , _count{ static_cast<int32>(end - begin) }
    {
        ASSERT_COLLECTION_SAFE_MOD(begin <= end); // Span begin must be before the end.
    }


    // Properties

    /// <summary> Returns the number of elements in the span. </summary>
    NO_DISCARD FORCE_INLINE
    int32 Count() const
    {
        return _count;
    }

    /// <summary>
    /// Returns the pointer to the underlying data i.e. the first element, unless span length is zero.
    /// </summary>
    /// <remarks>
    /// If the underlying data is <c>null</c>, the span length is also zero.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    T* Data()
    {
        return _data;
    }

    /// <summary>
    /// Returns the pointer to the underlying data i.e. the first element, unless span length is zero.
    /// </summary>
    /// <remarks>
    /// If the underlying data is <c>null</c>, the span length is also zero.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    const T* Data() const
    {
        return _data;
    }

    /// <summary> Checks if the span points to any elements. </summary>
    NO_DISCARD FORCE_INLINE explicit
    operator bool() const
    {
        const bool result = _count > 0;
        ASSERT_COLLECTION_INTEGRITY(!result || (_data != nullptr)); // Span data must be valid if the size is non-zero.
        return result;
    }


    // Element Access

    NO_DISCARD FORCE_INLINE
    bool IsValidIndex(const int32 index) const
    {
        const bool obeysRange = (index >= 0 && index < _count); // Index must be within the span range.
        const bool isNotNull = (_data != nullptr); // Span data must be valid if the size is non-zero, implying that the data is not null.
        return obeysRange && isNotNull;
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE
    T& operator[](const int32 index)
    {
        ASSERT_COLLECTION_SAFE_ACCESS(IsValidIndex(index)); // Ensure span is not empty and index is valid
        return _data[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE
    const T& operator[](const int32 index) const
    {
        ASSERT_COLLECTION_SAFE_ACCESS(IsValidIndex(index)); // Ensure span is not empty and index is valid
        return _data[index];
    }


    // Iterators

    //TODO Add iterators to Span.
    //TODO Idea: Unify Array and Span enumerators 'PointerIterator'.
};
