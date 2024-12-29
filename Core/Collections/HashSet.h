// Created by Mateusz Karbowiak 2024

#pragma once

#include "Allocators/HeapAlloc.h"
#include "Collections/CollectionsUtils.h"
#include "Language/TypeInfo.h"
#include "Math/HashingFunctions.h"
#include "Math/Probing.h"

template<
    typename T,
    typename Alloc = HeapAlloc,
    int32(&Probe)(int32, int32) = Probing::Linear,
    int32(&Grow)(int32) = Growing::Default
>
class HashSet
{
    //TODO Implement HashSet
};
