// Created by Mateusz Karbowiak 2024

#pragma once

#include <cstdlib>
#include <cstring>


/// <summary>
/// Type alias for a 1-byte unsigned integer, distinguished from a character.
/// </summary>
/// <remarks>
/// This type is used for raw memory manipulation to avoiding interpreting
/// the value as a type which could violate the Strict Aliasing Rule.
/// https://en.cppreference.com/w/c/language/object
/// </remarks>
using byte = unsigned char;


#define MALLOC(size)             malloc(size)
#define REALLOC(ptr, size)       realloc((ptr), (size))
#define FREE(ptr)                free(ptr)

#define MEMCPY(dst, src, size)   memcpy((dst), (src), (size))
#define MEMSET(dst, value, size) memset((dst), (value), (size))
#define MEMMOVE(dst, src, size)  memmove((dst), (src), (size))
