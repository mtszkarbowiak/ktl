// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Types/Numbers.h"
#include "Types/RawPuller.h"

/// <summary>
/// Non-owning view of a contiguous sequence of elements.
/// </summary>
/// <remarks>
/// Span is classified as null if it points to <c>null</c> OR its points to zero elements.
/// </remarks>
template<typename T>
class Span
{
    T*    _data{};
    int32 _count{};


    // Constructors

public:
    using Element = T;

    /// <summary> Initializes an empty span. </summary>
    FORCE_INLINE
    Span() = default;

    /// <summary> Initializes a span with the specified data and size. </summary>
    FORCE_INLINE
    Span(T* data, const int32 size) //TODO Ambiguous ctor
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

    /// <summary> Initializes a span from C-style array. </summary>
    template<uintptr N>
    FORCE_INLINE explicit
    Span(T(&array)[N])
        : Span{ array, static_cast<int32>(N) }
    {
    }


    // Properties

    /// <summary> Returns the number of elements in the span. </summary>
    NO_DISCARD FORCE_INLINE
    auto Count() const -> int32
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
    auto Data() -> T*
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
    auto Data() const -> const T*
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
    auto IsValidIndex(const int32 index) const -> bool
    {
        const bool obeysRange = (index >= 0 && index < _count); // Index must be within the span range.
        const bool isNotNull = (_data != nullptr); // Span data must be valid if the size is non-zero, implying that the data is not null.
        return obeysRange && isNotNull;
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator[](const int32 index) -> T&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(IsValidIndex(index)); // Ensure span is not empty and index is valid
        return _data[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE
    auto operator[](const int32 index) const -> const T&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(IsValidIndex(index)); // Ensure span is not empty and index is valid
        return _data[index];
    }


    // Iterators

    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto begin() -> T*
    {
        return _data;
    }

    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto begin() const -> const T*
    {
        return _data;
    }

    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto cbegin() const -> const T*
    {
        return _data;
    }

    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto end() -> T*
    {
        return _data + _count;
    }

    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto end() const -> const T*
    {
        return _data + _count;
    }

    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto cend() const -> const T*
    {
        return _data + _count;
    }


    /// <summary> Creates an enumerator for the array. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() -> RawPuller<T>
    {
        return RawPuller<T>{ begin(), end() };
    }

    /// <summary> Creates an enumerator for the array. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() const -> RawPuller<const T>
    {
        return RawPuller<const T>{ cbegin(), cend() };
    }
};
