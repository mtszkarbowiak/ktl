// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

// --- Communism Mode ---
// Communism mode is a programming style, where all members of all classes are public.
// This is a controversial programming style, which is often discouraged, but it has some advantages.
// Communism mode is useful in two scenarios:
// 1. Rapid prototyping, where you want to avoid writing getters and setters.
// 2. Useful for data structures, where you want to expose all members for performance reasons.

// Communism mode

#ifndef COMMUNISM_MODE
    #define COMMUNISM_MODE      0
#endif

#if COMMUNISM_MODE
    #define PRIVATE             public
#else
    #define PRIVATE             private
#endif
