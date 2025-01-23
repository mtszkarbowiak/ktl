// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

using uint8   = unsigned char;
using uint16  = unsigned short;
using uint32  = unsigned int;
using uint64  = unsigned long long;

using int8    = signed char;
using int16   = signed short;
using int32   = signed int;
using int64   = signed long long;

using float32 = float;
using float64 = double;

using uintptr = unsigned long long;
using intptr  = signed long long;

static_assert(sizeof(uint8)  == 1, "Size mismatch: uint8");
static_assert(sizeof(uint16) == 2, "Size mismatch: uint16");
static_assert(sizeof(uint32) == 4, "Size mismatch: uint32");
static_assert(sizeof(uint64) == 8, "Size mismatch: uint64");

static_assert(sizeof(int8)   == 1, "Size mismatch: int8");
static_assert(sizeof(int16)  == 2, "Size mismatch: int16");
static_assert(sizeof(int32)  == 4, "Size mismatch: int32");
static_assert(sizeof(int64)  == 8, "Size mismatch: int64");

static_assert(sizeof(float32) == 4, "Size mismatch: float32");
static_assert(sizeof(float64) == 8, "Size mismatch: float64");

static_assert(sizeof(uintptr) == sizeof(void*), "Pointer size mismatch!");
static_assert(sizeof(intptr)  == sizeof(void*), "Pointer size mismatch!");

// 2 lines above effectively enforce 64-bit architecture for the project.
// This code is a subject of The Ostrich Algorithm.
