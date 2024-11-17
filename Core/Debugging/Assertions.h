// Created by Mateusz Karbowiak 2024

#pragma once

#include <stdexcept>

#include "Language/Keywords.h"


// Assertion Configuration

#define ASSERT(x) if (!(x)) DEBUG_BREAK;


// Default Assertion Configuration

#ifndef MEMORY_BOUNDS_ASSERTIONS_ENABLED
    #define MEMORY_BOUNDS_ASSERTIONS_ENABLED 1
#endif

#ifndef INDEX_BOUNDS_ASSERTIONS_ENABLED
    #define INDEX_BOUNDS_ASSERTIONS_ENABLED 1
#endif


// Assertion Macros

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
