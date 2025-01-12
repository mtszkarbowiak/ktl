// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once


/// <summary>
/// Tuple of two elements, often used to represent a function mapping.
/// </summary>
/// <typeparam name="K"> Key type, associated with argument of a function mapping. </typeparam>
/// <typeparam name="V"> Value type, associated with value of a function mapping. </typeparam>
template<
    typename K, 
    typename V
>
struct Pair 
{
    K Key;
    V Value;
};

// This type is not very advanced... The code ends here, I guess.
