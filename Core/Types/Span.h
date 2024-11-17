// Created by Mateusz Karbowiak 2024

#pragma once

#include "../Debugging/Assertions.h"
#include "../Types/Numbers.h"


template<typename T>
class Span
{
    T* _data;
    int32 _size;


public:
    explicit Span(T* data, const int32 size)
        : _data{ data }
        , _size{ size }
    {
    }

    Span(const Span&) = default;

    Span(Span&&) noexcept = default;


    auto operator=(const Span&) -> Span & = default;

    auto operator=(Span&& other) noexcept -> Span&
    {
        if (this != &other)
        {
            _data = other._data;
            _size = other._size;
            other._data = nullptr;
            other._size = 0;
        }
        return *this;
    }

    ~Span() = default;


    auto operator[](const int32 index) -> T&
    {
        ASSERT_MEMORY_BOUNDS(index >= 0 && index < _size);
        return _data[index];
    }

    auto operator[](const int32 index) const -> const T&
    {
        ASSERT_MEMORY_BOUNDS(index >= 0 && index < _size);
        return _data[index];
    }
};
