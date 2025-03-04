// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Language/TypeTraits.h"

// References

template<typename T>
class Ref;
template<typename T>
class Span;

class MutBitRef;
class ConstBitRef;


// Simple Owning

template<typename K, typename V>
struct Pair;
template<typename T, bool M = (TMaxTombstoneDepth<T>::Value > 0)>
class Nullable;
template<typename E, typename S = typename TUnderlyingType<E>::Type>
class EnumSet;


// Memory

using DefaultAlloc = class HeapAlloc;

template<typename T, typename A = DefaultAlloc>
struct Box;

class Rc;
template<typename T>
class RcRead;
template<typename T>
class RcWrite;
template<typename T, typename A = DefaultAlloc>
class RcBox;


// Iteration

class Index;
class SizeHint;


// Dummies

class Dummy;
enum class DummyEnum { A };
