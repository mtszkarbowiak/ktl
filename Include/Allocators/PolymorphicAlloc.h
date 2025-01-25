// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Debugging/Assertions.h"
#include "Language/Communism.h"
#include "Language/Keywords.h"
#include "Language/Templates.h"
#include "Language/Memory.h"
#include "Math/Arithmetic.h"
#include "Types/Numbers.h"

/// <summary>
/// Memory allocator that uses other allocators to allocate memory.
/// </summary>
template<typename A1, typename A2>
class PolymorphicAlloc
{
public:
    static constexpr bool  IsNullable  = true; // Because the allocator can always use `None` state.
    static constexpr int32 MinCapacity = A1::MinCapacity;
    static constexpr int32 MaxCapacity = A2::MaxCapacity;

    static_assert(MinCapacity < MaxCapacity, "The minimum capacity must be less than the maximum capacity.");

    class Data 
    {
    PRIVATE:
        typename A1::Data _mainData{};
        typename A2::Data _backupData{};

        enum class State : uint8
        {
            None,
            Main,
            Backup,
        }
        _state{ State::None };


        // Allocation Manipulation

    public:
        NO_DISCARD FORCE_INLINE
        auto MovesItems() const -> bool
        {
            switch (_state)
            {
            case State::Main:
                return _mainData.MovesItems();

            case State::Backup:
                return _backupData.MovesItems();

            case State::None:
                FALLTHROUGH
            default:
                // If there is no data, it's safe to move 'items'
                // It doesn't necessarily mean that it is safe to move the bindings themselves.
                return true; 
            };
        }

        NO_DISCARD FORCE_INLINE
        auto Get() -> void*
        {
            switch (_state)
            {
            case State::Main:
                return _mainData.Get();

            case State::Backup:
                return _backupData.Get();

            case State::None:
                FALLTHROUGH
            default:
                return nullptr;
            };
        }

        NO_DISCARD FORCE_INLINE
        auto Get() const -> const void* 
        {
            switch (_state)
            {
            case State::Main:
                return _mainData.Get();

            case State::Backup:
                return _backupData.Get();

            case State::None:
                FALLTHROUGH
            default:
                return nullptr;
            };
        }

        NO_DISCARD FORCE_INLINE
        auto Allocate(const int32 size) -> int32
        {
            ASSERT_ALLOCATOR_SAFETY(_state == State::None);

            const int32 mainAllocated = _mainData.Allocate(size);
            if (mainAllocated >= size)
            {
                _state = State::Main;
                return mainAllocated;
            }

            const int32 backupAllocated = _backupData.Allocate(size);
            if (backupAllocated >= size)
            {
                _state = State::Backup;
                return backupAllocated;
            }

            // If neither allocation succeeded, we must indicate that.
            return 0;
        }

        FORCE_INLINE
        auto Free()
        {
            switch (_state)
            {
            case State::Main:
                _mainData.Free();
                break;

            case State::Backup:
                _backupData.Free();
                break;

            case State::None:
                FALLTHROUGH
            default:
                break;
            };

            _state = State::None;
        }


        // Lifecycle

        Data() = default;

        Data(const Data& other)
            : _mainData{ other._mainData }
            , _backupData{ other._backupData }
            , _state{ State::None }
        {
        }

        Data(Data&& other) noexcept
            : _mainData{ other._mainData }     // DO NOT MOVE! ONLY COPY BINDINGS!
            , _backupData{ other._backupData } // DO NOT MOVE! ONLY COPY BINDINGS!
            , _state{ other._state }
        {
            // Move depending on state
            switch (_state)
            {
            case State::Main:
                _mainData = MOVE(other._mainData);
                break;
            case State::Backup:
                _backupData = MOVE(other._backupData);
                break;
            };
        }

        auto operator=(const Data& other) -> Data&
        {
            if (this != &other)
            {
                ASSERT_ALLOCATOR_SAFETY(_state == State::None); // Active allocation can never be overwritten!

                _mainData   = other._mainData;
                _backupData = other._backupData;
                _state      = State::None;
            }
            return *this;
        }

        auto operator=(Data&& other) noexcept -> Data&
        {
            // Bindings can not be overwritten.
            if (this != &other)
            {
                ASSERT_ALLOCATOR_SAFETY(_state == State::None); // Active allocation can never be overwritten!

                _state = other._state;
                _mainData = MOVE(other._mainData);
                _backupData = MOVE(other._backupData);

                other._state = State::None;
            }
            return *this;
        }

        ~Data()
        {
            ASSERT_ALLOCATOR_SAFETY(_state == State::None);
        }
    };
};
