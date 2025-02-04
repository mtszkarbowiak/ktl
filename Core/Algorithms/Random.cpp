// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#include "Algorithms/Random.h"

XorwowRng::XorwowRng(const uint32 seed)
{
    _state[0] = seed + 0x9e3779b9;
    _state[1] = seed * 0x193a6754;
    _state[2] = seed * 0x4cbe58a5;
    _state[3] = seed * 0x5f356f79;
    _state[4] = seed * 0x9b8b2d05;
    _d = 362436;
}

auto XorwowRng::operator++() -> XorwowRng&
{
    const uint32 t = _state[0] ^ (_state[0] >> 2);

    _state[0] = _state[1];
    _state[1] = _state[2];
    _state[2] = _state[3];
    _state[3] = _state[4];

    _state[4] = (_state[4] ^ (_state[4] << 4)) ^ (t ^ (t << 1));

    _d += 362437;

    return *this;
}
