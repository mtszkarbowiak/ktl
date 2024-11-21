// Created by Mateusz Karbowiak 2024

#pragma once

#include "Allocators/HeapAlloc.h"
#include "Collections/CollectionsUtils.h"
#include "Language/TypeInfo.h"

template<
    typename T,
    int32    ChunkSize  = 16,
    typename ChunkAlloc = HeapAlloc,
    typename MainAlloc  = HeapAlloc
>
class ChunkedArray
{
    //TODO Implement ChunkedArray
};
