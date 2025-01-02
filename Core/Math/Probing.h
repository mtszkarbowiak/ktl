// Created by Mateusz Karbowiak 2024

#pragma once

#include "Types/Numbers.h"
#include "Language/Keywords.h"

/// <summary>
/// Set of functions used in hashing algorithms to resolve collisions.
/// </summary>
class Probing
{
public:
    FORCE_INLINE
    static int32 Linear(const int32 size, const int32 numChecks)
    {
        return numChecks;
    }

    FORCE_INLINE
    static int32 Quadratic(const int32 size, const int32 numChecks)
    {
        return numChecks * numChecks;
    }

    FORCE_INLINE
    static int32 DoubleHashing(const int32 size, const int32 numChecks)
    {
        const int32 secondHash = 1 + (size % (size - 1));
        return secondHash * numChecks;
    }

    FORCE_INLINE
    static int32 Fibonacci(const int32 size, const int32 numChecks)
    {
        int32 fib1 = 1, fib2 = 1;
        for (int i = 2; i <= numChecks; ++i) 
        {
            int32 nextFib = fib1 + fib2;
            fib1 = fib2;
            fib2 = nextFib;
        }
        return fib2;
    }

    FORCE_INLINE
    static int32 Prime(const int32 size, const int32 numChecks)
    {
        // Example: Use prime numbers to reduce clustering
        const int32 primes[] = { 3, 7, 11, 17, 19, 23, 29, 31 };
        return primes[numChecks % (sizeof(primes) / sizeof(primes[0]))];
    }
};
