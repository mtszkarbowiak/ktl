// Created by Mateusz Karbowiak 2024

#pragma once

#include <algorithm>


#define MOVE(x)       std::move(x)
#define FORWARD(T, x) std::forward<T>(x)


template<typename T>
static void Swap(T& a, T& b)
{
    T temp = MOVE(a);
    a = MOVE(b);
    b = MOVE(temp);
}
