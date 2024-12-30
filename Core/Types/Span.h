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


public:
    explicit Span(T* data, const int32 size)
        : _data{ data }
        , _count{ size }
    {
    }

    Span(const Span&) = default;

    Span(Span&&) noexcept = default;


    Span& operator=(const Span&)= default;

    Span& operator=(Span&&) noexcept = default;

    ~Span() = default;


    // Properties

    /// <summary> Returns the number of elements in the span. </summary>
    FORCE_INLINE NODISCARD
    int32 Count() const
    {
        return _count;
    }

    /// <summary>
    /// Returns the pointer to the underlying data i.e.
    /// the first element or <c>nullptr</c> if the span is empty.
    /// </summary>
    FORCE_INLINE NODISCARD
    T* Data()
    {
        return _data;
    }

    /// <summary>
    /// Returns the pointer to the underlying data i.e.
    /// the first element or <c>nullptr</c> if the span is empty.
    /// </summary>
    FORCE_INLINE NODISCARD
    const T* Data() const
    {
        return _data;
    }

    /// <summary> Checks if the span points to any elements. </summary>
    FORCE_INLINE NODISCARD
    explicit operator bool() const
    {
        return
            static_cast<bool>(_data) || 
            static_cast<bool>(_count);
    }


    // Element Access

    FORCE_INLINE NODISCARD
    T& operator[](const int32 index)
    {
        return _data[index];
    }

    FORCE_INLINE NODISCARD
    const T& operator[](const int32 index) const
    {
        return _data[index];
    }


    // Iterators

    //TODO Add iterators to Span.
    //TODO Idea: Unify Array and Span enumerators 'PointerIterator'.
};
