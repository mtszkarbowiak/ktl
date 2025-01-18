// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

// --- Yolo Mode ---
// Yolo mode is a programming style, where all functions are noexcept.
// While this is a controversial programming style, it has some advantages,
// including small performance. Enabling Yolo mode will completely disable exceptions.

#ifndef YOLO_NOEXCEPT_MODE
    #define YOLO_NOEXCEPT_MODE 0
#endif

#if YOLO_MODE
    #define NOEXCEPT_YOLO  noexcept
#else
    #define NOEXCEPT_YOLO
#endif

// --- Safe Mode ---
// Safe mode is a programming style, where NO functions are noexcept.
// While this can hinder performance, or sometimes even completely ruin compilation,
// it allows to catch all exceptions and handle them properly.

#ifndef SAFE_NOEXCEPT_MODE
    #define SAFE_NOEXCEPT_MODE 0
#endif

#if SAFE_NOEXCEPT_MODE
    #define NOEXCEPT_SAFE
#else
    #define NOEXCEPT_SAFE noexcept
#endif
