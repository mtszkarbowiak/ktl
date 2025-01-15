// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/mk-stl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Allocators/HeapAlloc.h"
#include "Collections/AllocHelper.h"
#include "Language/Templates.h"

/// <summary>
/// Stores one (or zero) element using a custom allocator.
/// </summary>
///
/// <typeparam name="T">
/// Type of the element to store.
/// </typeparam>
/// <typeparam name="A">
/// (Optional) Type of the allocator to use. It must be nullable.
/// </typeparam>
///
/// <remarks>
/// 1. Use factory methods to create instances of this class.
/// 2. This class works effectively as <c>std::unique_ptr</c>, but customizes the allocator.
/// </remarks>
template<
    typename T,
    typename A = HeapAlloc
>
struct Box
{
    using Element   = T;
    using AllocData = typename A::Data;

PRIVATE:
    AllocData _allocData{};


    // Element Access

public:
    /// <summary> Checks if the box stores no valid element. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsEmpty() const -> bool
    {
        // Allocator is nullable.
        return _allocData.Get() == nullptr;
    }

    /// <summary> Checks if the box stores a valid element. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto HasValue() const -> bool
    {
        // Allocator is nullable.
        return _allocData.Get() != nullptr;
    }

    /// <summary> Accesses the stored element. </summary>
    /// <remarks> The box must not be empty. </remarks>
    NO_DISCARD FORCE_INLINE constexpr
    auto Get() -> Element*
    {
        return DATA_OF(Element, _allocData);
    }

    /// <summary> Accesses the stored element. </summary>
    /// <remarks> The box must not be empty. </remarks>
    NO_DISCARD FORCE_INLINE constexpr
    auto Get() const -> const Element*
    {
        return DATA_OF(const Element, _allocData);
    }


    NO_DISCARD FORCE_INLINE constexpr
    auto operator*() -> Element&
    {
        return *Get();
    }

    NO_DISCARD FORCE_INLINE constexpr
    auto operator*() const -> const Element&
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_allocData.Get() != nullptr); // Box must not be empty!
        return *Get();
    }


    NO_DISCARD FORCE_INLINE constexpr
    auto operator->() -> Element*
    {
        return Get();
    }

    NO_DISCARD FORCE_INLINE constexpr
    auto operator->() const -> const Element*
    {
        ASSERT_COLLECTION_SAFE_ACCESS(_allocData.Get() != nullptr); // Box must not be empty!
        return Get();
    }


    // Element Manipulation

    /// <summary>
    /// Destroys the stored element and clears the box.
    /// If no element is stored, the box remains empty.
    /// </summary>
    FORCE_INLINE constexpr
    void Reset()
    {
        if (!IsEmpty()) 
        {
            Get()->~Element();
            _allocData.Free();
        }
    }

    /// <summary> Replaces the stored element with a new one by constructing it. </summary>
    template<typename... Args>
    FORCE_INLINE constexpr
    void Emplace(Args&&... args)
    {
        Reset();
        _allocData.Allocate(sizeof(Element));
        new (Get()) Element(FORWARD(Args, args)...);
    }


    // Lifecycle

    /// <summary> Initializes an empty box. </summary>
    FORCE_INLINE constexpr
    Box() noexcept = default;

    
    /// <summary> Copying a box is not allowed. </summary>
    /// <remarks> Blocking copy operation allows using the box as a unique pointer. </remarks>
    Box(const Box& other) = delete;


    /// <summary> Moves the contents of the other box into this one. </summary>
    FORCE_INLINE
    Box(Box&& other) noexcept
    {
        if (!other.IsEmpty()) 
        {
            if (other._allocData.MovesItems()) 
            {
                _allocData = MOVE(other._allocData);
            }
            else 
            {
                const int32 allocated = _allocData.Allocate(sizeof(Element));
                ASSERT_COLLECTION_SAFE_MOD(allocated > 0); // Allocation must succeed.
                new(Get()) Element(MOVE(*other.Get()));
                other.Reset();
            }
        }
    }

    /// <summary> Copying a box is not allowed. </summary>
    /// <remarks> Blocking copy operation allows using the box as a unique pointer. </remarks>
    auto operator=(const Box& other) -> Box& = delete;

    /// <summary> Resets the box and moves the contents of the other box into this one. </summary>
    MAY_DISCARD FORCE_INLINE
    auto operator=(Box&& other) noexcept -> Box&
    {
        if (this == &other)
        {
            return *this;
        }

        Reset();

        if (!other.IsEmpty())
        {
            // If the alloc can't move the data, the collection must do it.
            if (!other._allocData.MovesItems())
            {
                _allocData = MOVE(other._allocData);
            }
            _allocData = MOVE(other._allocData);
        }
        return *this;
    }

    /// <summary> Destroys the box </summary>
    FORCE_INLINE
    ~Box()
    {
        Reset();
    }


    // Identity

    template<typename A2>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator==(const Box<T, A2>& other) const -> bool
    {
        if (IsEmpty() && other.IsEmpty())
        {
            return true;
        }
        if (IsEmpty() || other.IsEmpty())
        {
            return false;
        }
        return *Get() == *other.Get();
    }

    template<typename A2>
    NO_DISCARD FORCE_INLINE constexpr
    auto operator!=(const Box<T, A2>& other) const -> bool
    {
        return !(*this == other);
    }


    // Factorization

    /// <summary> Explicitly creates an empty box. </summary>
    NO_DISCARD static FORCE_INLINE constexpr
    auto Empty() -> Box
    {
        return Box{};
    }

    /// <summary> Creates a box constructed with the specified arguments. </summary>
    /// <remarks> This overload does not allows for allocators with context. </remarks>
    template<typename... Args>
    NO_DISCARD static FORCE_INLINE constexpr
    auto Make(Args&&... args) -> Box
    {
        Box box;
        const int32 allocated = box._allocData.Allocate(sizeof(Element));
        ASSERT_COLLECTION_SAFE_MOD(allocated > 0); // Allocation must succeed.
        new(box.Get()) Element(FORWARD(Args..., args)...);

        return box;
    }

    /// <summary> Creates a box constructed with the specified arguments. </summary>
    /// <remarks> This overload allows for allocators with context. </remarks>
    template<typename... Args, typename AllocContext>
    NO_DISCARD static FORCE_INLINE constexpr
    auto MakeWithContext(AllocContext&& context, Args&&... args) -> Box
    {
        Box box;
        box._allocData = AllocData{ FORWARD(AllocContext, context) };
        const int32 allocated = box._allocData.Allocate(sizeof(Element));
        ASSERT_COLLECTION_SAFE_MOD(allocated > 0); // Allocation must succeed.
        new(box.Get()) Element(FORWARD(Args, args)...);

        return box;
    }


    // Constraints

    static_assert(A::IsNullable, "Allocator must be nullable.");

    static_assert(!std::is_reference<T>                ::value, "Type must not be a reference.");
    static_assert(!std::is_const<T>                    ::value, "Type must not be a const-qualified type.");
    static_assert(std::is_move_constructible<T>        ::value, "Type must be move-constructible.");
    static_assert(std::is_destructible<T>              ::value, "Type must be destructible.");
    static_assert(std::is_nothrow_move_constructible<T>::value, "Type must be nothrow move-constructible.");
    static_assert(std::is_nothrow_destructible<T>      ::value, "Type must be nothrow destructible.");
};
