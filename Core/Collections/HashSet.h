// Created by Mateusz Karbowiak 2024

#pragma once

#include "Collections/CollectionsUtils.h"

template<
    typename T,
    typename Alloc = DefaultAlloc,
    int32(&Probe)(int32, int32) = Probing::Linear,
    int32(&Grow)(int32) = Growing::Default
>
class HashSet
{
    //TODO Implement HashSet
};
