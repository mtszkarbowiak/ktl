// Created by Mateusz Karbowiak 2024

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



// C++ Versions

#if __cplusplus >= 201703L // C++17 and above
    #define NODISCARD           [[nodiscard]]
    #define FALLTHROUGH         [[fallthrough]]
    #define MAYBE_UNUSED        [[maybe_unused]]

#else // C++14 and below
    #define NODISCARD         
    #define FALLTHROUGH       
    #define MAYBE_UNUSED

#endif


#if __cplusplus >= 202002L // C++20 and above
    #define IF_CONSTEXPR        constexpr
    #define CONSTINIT           constinit
    #define CONSTEXPR_LAMBDA    constexpr

#else // C++17 and below
    #define IF_CONSTEXPR
    #define CONSTINIT           constexpr
    #define CONSTEXPR_LAMBDA    constexpr

#endif
