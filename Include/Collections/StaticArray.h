// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"

/// <summary>
/// A container for statically sized arrays of elements, stored in a single contiguous block of memory.
/// The type provides additional functionality over C-style arrays.
/// </summary>
template<typename T, uintptr N>
class StaticArray
{
public:
    using Element = T;

    using MutPuller   = RawPuller<T>;
    using ConstPuller = RawPuller<const T>;


PRIVATE:
    T _data[N];


    // Lifecycle

    /// <summary> Initializes array with default values (unless fundamental type). </summary>
    FORCE_INLINE constexpr
    StaticArray() = default;

    /// <summary> Initializes array by copying another array. </summary>
    FORCE_INLINE constexpr
    StaticArray(const StaticArray& other) = default;

    /// <summary> Initializes array by moving another array. </summary>
    FORCE_INLINE constexpr
    StaticArray(StaticArray&& other) NOEXCEPT_S = default;

    /// <summary> Destroys the array. </summary>
    FORCE_INLINE
    ~StaticArray() = default;

    /// <summary> Assigns another array to this one by copying. </summary>
    MAY_DISCARD FORCE_INLINE constexpr
    auto operator=(const StaticArray& other) -> StaticArray& = default;

    /// <summary> Assigns another array to this one by moving. </summary>
    MAY_DISCARD FORCE_INLINE constexpr
    auto operator=(StaticArray&& other) NOEXCEPT_S -> StaticArray& = default;


    // Element Access

    /// <summary>
    /// Accesses the first element of the array.
    /// To be used with <c>Count</c> for C-style API, where the first element is at index 0.
    /// </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Data() -> Element*
    {
        return _data;
    }

    /// <summary>
    /// Accesses the first element of the array.
    /// To be used with <c>Count</c> for C-style API, where the first element is at index 0.
    /// </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Data() const -> const Element*
    {
        return _data;
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator[](const int32 index) -> Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < N);
        return _data[index];
    }

    /// <summary> Accesses the element at the given index. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator[](const int32 index) const -> const Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < N);
        return _data[index];
    }
    
    /// <summary> Returns the number of elements in the array. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Count() const -> int32
    {
        return N;
    }


    // Pullers

    /// <summary> Creates a puller for the array. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() -> MutPuller
    {
        return MutPuller{ _data, _data + N };
    }

    /// <summary> Creates a puller for the array. </summary>
    NO_DISCARD FORCE_INLINE
    auto Values() const -> ConstPuller
    {
        return ConstPuller{ _data, _data + N };
    }

    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto begin() -> Element*
    {
        return _data;
    }

    /// <summary> STL-style begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto begin() const -> const Element*
    {
        return _data;
    }

    /// <summary> STL-style const begin iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto cbegin() const -> const Element*
    {
        return _data;
    }

    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto end() -> Element*
    {
        return _data + N;
    }

    /// <summary> STL-style end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto end() const -> const Element*
    {
        return _data + N;
    }

    /// <summary> STL-style const end iterator. </summary>
    NO_DISCARD FORCE_INLINE
    auto cend() const -> const Element*
    {
        return _data + N;
    }

    /// <summary> Creates a span of the stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() noexcept -> Span<Element>
    {
        return Span<Element>{ _data, N };
    }

    /// <summary> Creates a read-only span of the stored elements. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto AsSpan() const noexcept -> Span<const Element>
    {
        return Span<const Element>{ _data, N };
    }
};
