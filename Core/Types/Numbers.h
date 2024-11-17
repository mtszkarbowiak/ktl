// Created by Mateusz Karbowiak 2024

#pragma once

#include "../Language/TypeInfo.h"


using uint8  = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

using int8   = signed char;
using int16  = signed short;
using int32  = signed int;
using int64  = signed long long;

using float32 = float;
using float64 = double;


DECLARE_C_STYLE_TYPE(uint8);
DECLARE_C_STYLE_TYPE(uint16);
DECLARE_C_STYLE_TYPE(uint32);
DECLARE_C_STYLE_TYPE(uint64);

DECLARE_C_STYLE_TYPE(int8);
DECLARE_C_STYLE_TYPE(int16);
DECLARE_C_STYLE_TYPE(int32);
DECLARE_C_STYLE_TYPE(int64);
