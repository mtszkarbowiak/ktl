// Created by Mateusz Karbowiak 2024

#pragma once

#include <cstdlib>
#include <cstring>


using byte = unsigned char;


#define MALLOC(size)            malloc(size)
#define REALLOC(ptr, size)      realloc((ptr), (size))
#define FREE(ptr)               free(ptr)

#define MEMCPY(dst, src, size)  memcpy((dst), (src), (size))
#define MEMMOVE(dst, src, size) memmove((dst), (src), (size))
