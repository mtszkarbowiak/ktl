// Created by Mateusz Karbowiak 2024

#pragma once

#include "Language/Keywords.h"


// Assertion Configuration

#define ASSERT(x) if (!(x)) DEBUG_BREAK;

// WARNING: Assertions have been categorized ONLY to allow balance assertions power with performance.

// --- Assertions Level ---
// 0. No assertions.
// 1. Basic assertions.
// 2. Full assertions.
// 3. Extended assertions.
#ifndef ASSERTIONS_LEVEL
    #define ASSERTIONS_LEVEL 3
#endif


// --- Allocators Assertions ---
// Allocator are fundamental types, which can be invoked very frequently.
// While allocator assertions may be very useful during development of custom collections,
// they may be disabled in production builds to improve performance, without losing much safety.

#ifndef ALLOCATOR_ASSERTIONS_ENABLED
    #define ALLOCATOR_ASSERTIONS_ENABLED (ASSERTIONS_LEVEL >= 2)
#endif

#ifdef ALLOCATOR_ASSERTIONS_ENABLED
    #define ASSERT_ALLOCATOR_SAFETY(x) ASSERT(x)
#else
    #define ASSERT_ALLOCATOR_SAFETY(x)
#endif


// --- Collection Integrity Assertions ---
// Collections internal integrity assertions check if the collection correctly modifies its state.
// They are useful when developing those collections, but may be disabled in production builds.

#ifndef COLLECTION_INTEGRITY_ASSERTIONS_ENABLED
    #define COLLECTION_INTEGRITY_ASSERTIONS_ENABLED (ASSERTIONS_LEVEL >= 3)
#endif

#ifdef COLLECTION_INTEGRITY_ASSERTIONS_ENABLED
    #define ASSERT_COLLECTION_INTEGRITY(x) ASSERT(x)
#else
    #define ASSERT_COLLECTION_INTEGRITY(x)
#endif


// --- Collection Safe Modification Assertions ---
// Collection safety assertions check if the collection is modified correctly by the user.
// While they do harm performance, they are crucial for catching bugs in the user code.
// It is recommended to postpone disabling them after the codebase is declared stable.

#ifndef COLLECTION_SAFE_MOD_ASSERTIONS_ENABLED
    #define COLLECTION_SAFE_MOD_ASSERTIONS_ENABLED (ASSERTIONS_LEVEL >= 2)
#endif

#ifdef COLLECTION_SAFE_MOD_ASSERTIONS_ENABLED
    #define ASSERT_COLLECTION_SAFE_MOD(x) ASSERT(x)
#else
    #define ASSERT_COLLECTION_SAFE_MOD(x)
#endif


// --- Collections Safe Access Assertions ---
// Collection safety assertions check if the collection is accessed correctly by the user.
// While they do harm performance, they are crucial for catching bugs in the user code.
// It is recommended to postpone disabling them after the codebase is declared stable.

#ifndef COLLECTION_SAFE_ACCESS_ASSERTIONS_ENABLED
    #define COLLECTION_SAFE_ACCESS_ASSERTIONS_ENABLED (ASSERTIONS_LEVEL >= 2)
#endif

#ifdef COLLECTION_SAFE_ACCESS_ASSERTIONS_ENABLED
    #define ASSERT_COLLECTION_SAFE_ACCESS(x) ASSERT(x)
#else
    #define ASSERT_COLLECTION_SAFE_ACCESS(x)
#endif
