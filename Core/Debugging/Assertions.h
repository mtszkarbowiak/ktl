// Created by Mateusz Karbowiak 2024

#pragma once

#include <stdexcept>

#define MEMORY_BOUNDS_ASSERTIONS_ENABLED
#define INDEX_BOUNDS_ASSERTIONS_ENABLED


#define ASSERT(x) if (!(x)) \
    throw std::runtime_error("Assertion failed.")

#ifdef MEMORY_BOUNDS_ASSERTIONS_ENABLED
    #define ASSERT_MEMORY_BOUNDS(x) ASSERT(x)
#else
    #define ASSERT_MEMORY_BOUNDS(x)
#endif

#ifdef INDEX_BOUNDS_ASSERTIONS_ENABLED
    #define ASSERT_INDEX_BOUNDS(x) ASSERT(x)
#else
    #define ASSERT_INDEX_BOUNDS(x)
#endif
