// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once


// Inlining Keywords

#if defined(_MSC_VER)
    #define FORCE_INLINE        __forceinline
    #define NO_INLINE           __declspec(noinline)

#elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE        inline __attribute__((always_inline))
    #define NO_INLINE           __attribute__((noinline))

#else
    #define FORCE_INLINE        inline
    #define NO_INLINE

#endif


// Branch Prediction Keywords

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)           __builtin_expect(!!(x), 1)
    #define UNLIKELY(x)         __builtin_expect(!!(x), 0)

#else
    #define LIKELY(x)           (x)
    #define UNLIKELY(x)         (x)

#endif


// Debugging Keywords

#if defined(_MSC_VER)
    #define DEBUG_BREAK         __debugbreak()

#elif defined(__GNUC__) || defined(__clang__)
    #define DEBUG_BREAK         __builtin_trap()

#endif


// Pop-Count Keywords

#if defined(_MSC_VER)
    #include <intrin.h>
    #define POP_COUNT32(x)      static_cast<int>(__popcnt(x))
    #define POP_COUNT64(x)      static_cast<int>(__popcnt64(x))

#elif defined(__GNUC__) || defined(__clang__)
    #define POP_COUNT32(x)      static_cast<int>(__builtin_popcount(x))
    #define POP_COUNT64(x)      static_cast<int>(__builtin_popcountll(x))

#endif



// C++ Versions

#ifndef _MSVC_LANG
    #define CXX17_SUPPORT       (__cplusplus >= 201703L)
    #define CXX20_SUPPORT       (__cplusplus >= 202002L)
    #define CXX23_SUPPORT       (__cplusplus >= 202300L)
#else
    #define CXX17_SUPPORT       (_MSVC_LANG >= 201703L)
    #define CXX20_SUPPORT       (_MSVC_LANG >= 202002L)
    #define CXX23_SUPPORT       (_MSVC_LANG >= 202300L)
#endif



#if CXX17_SUPPORT
    #define NO_DISCARD          [[nodiscard]]
    #define MAY_DISCARD         [[maybe_unused]]
    #define FALLTHROUGH         [[fallthrough]];
    #define MAYBE_UNUSED        [[maybe_unused]]
    #define IF_CONSTEXPR        constexpr

#else // C++14 and below
    #define NO_DISCARD
    #define MAY_DISCARD
    #define FALLTHROUGH       
    #define MAYBE_UNUSED
    #define IF_CONSTEXPR

#endif


#if CXX20_SUPPORT
    #define CONCEPTS_ENABLED    1

    #define CONSTINIT           constinit
    #define CONSTEVAL           consteval
    #define CONSTEXPR_LAMBDA    constexpr

    #define LIKELY_HINT(x)      [[likely]] x
    #define UNLIKELY_HINT(x)    [[unlikely]] x

#else // C++17 and below
    #define CONCEPTS_ENABLED   0

    #define CONSTINIT           constexpr
    #define CONSTEVAL           constexpr
    #define CONSTEXPR_LAMBDA    constexpr

    #define LIKELY_HINT(x)      LIKELY(x)
    #define UNLIKELY_HINT(x)    UNLIKELY(x)

#endif
