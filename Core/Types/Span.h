// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"

/// <summary>
/// Non-owning view of a contiguous sequence of elements.
/// </summary>
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

    FORCE_INLINE NODISCARD
    int32 Count() const
    {
        return _count;
    }

    FORCE_INLINE NODISCARD
    T* Data()
    {
        return _data;
    }

    FORCE_INLINE NODISCARD
    const T* Data() const
    {
        return _data;
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
