// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"
#include "Types/Numbers.h"


class AllocMath
{
public:
    FORCE_INLINE
    constexpr static auto Align(const int32 value, const int32 alignment) -> int32
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }
};
