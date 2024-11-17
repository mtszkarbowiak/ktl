// Created by Mateusz Karbowiak 2024

#pragma once

#include "../Allocators/HeapAlloc.h"
#include "../Language/Templates.h"

/// <summary>
/// Stores one (or zero) element using a custom allocator.
/// Use factory methods to create instances of this class.
/// </summary>
/// <typeparam name="T"> Type of the element to store. </typeparam>
/// <typeparam name="Alloc"> Type of the allocator to use. It must be nullable. </typeparam>
/// <remarks> This class works effectively as advanced `std::unique_ptr`. </remarks>
template<typename T, typename Alloc = HeapAlloc>
class Box
{
    using AllocData = typename Alloc::Data;

    AllocData _allocData;


public:
    // Element Access

    /// <summary> Checks if the box stores no valid element. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsEmpty() const noexcept -> bool
    {
        // Allocator is nullable.
        return _allocData.Get() == nullptr;
    }

    /// <summary> Checks if the box stores a valid element. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto HasValue() const noexcept -> bool
    {
        // Allocator is nullable.
        return _allocData.Get() != nullptr;
    }

    /// <summary> Accesses the stored element. </summary>
    /// <remarks> The box must not be empty. </remarks>
    FORCE_INLINE NODISCARD
    constexpr auto Get() -> T*
    {
        ASSERT_MEMORY_BOUNDS(!IsEmpty());
        return reinterpret_cast<T*>(_allocData.Get());
    }

    /// <summary> Accesses the stored element. </summary>
    /// <remarks> The box must not be empty. </remarks>
    FORCE_INLINE NODISCARD
    constexpr auto Get() const -> const T*
    {
        ASSERT_MEMORY_BOUNDS(!IsEmpty());
        return reinterpret_cast<const T*>(_allocData.Get());
    }


    FORCE_INLINE NODISCARD
    constexpr auto operator*() -> T& { return *Get(); }

    FORCE_INLINE NODISCARD
    constexpr auto operator*() const -> const T& { return *Get(); }


    FORCE_INLINE NODISCARD
    constexpr auto operator->() -> T* { return Get(); }

    FORCE_INLINE NODISCARD
    constexpr auto operator->() const -> const T* { return Get(); }


    // Element Manipulation

    /// <summary>
    /// Destroys the stored element and clears the box.
    /// If no element is stored, the box remains empty.
    /// </summary>
    FORCE_INLINE
    void Reset()
    {
        if (IsEmpty())
        {
            return;
        }

        Get()->~T();
        _allocData.Free();
    }

    /// <summary> Replaces the stored element with a new one by constructing it. </summary>
    template<typename... Args>
    FORCE_INLINE
    void Emplace(Args&&... args)
    {
        Reset();
        _allocData.Allocate(sizeof(T));
        new (Get()) T(FORWARD(Args, args)...);
    }

    /// <summary> Replaces the stored element with a new one by moving or copying it. </summary>
    template<typename U>
    FORCE_INLINE
    void Set(U&& value)
    {
        Reset();
        _allocData.Allocate(sizeof(T));
        new (Get()) T(value);
    }

    // Lifecycle

    /// <summary> Initializes an empty box. </summary>
    FORCE_INLINE
    Box() noexcept = default;

    
    /// <summary> Copying a box is not allowed. </summary>
    /// <remarks> Blocking copy operation allows using the box as a unique pointer. </remarks>
    FORCE_INLINE
    Box(const Box& other) = delete;


    /// <summary> Moves the contents of the other box into this one. </summary>
    FORCE_INLINE
    Box(Box&& other) noexcept
        : _allocData{}
    {
        if (other.IsEmpty())
        {
            // Pass.
        }
        else if (other._allocData.MovesItems())
        {
            _allocData = MOVE(other._allocData);
        }
        else
        {
            _allocData.Allocate(sizeof(T));
            new (Get()) T(MOVE(*other.Get()));
            other.Reset();
        }
    }

    /// <summary> Copying a box is not allowed. </summary>
    /// <remarks> Blocking copy operation allows using the box as a unique pointer. </remarks>
    auto operator=(const Box& other)->Box & = delete;

    /// <summary> Resets the box and moves the contents of the other box into this one. </summary>
    FORCE_INLINE
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


    // Factorization

    /// <summary> Explicitly creates an empty box. </summary>
    FORCE_INLINE NODISCARD
    static auto Empty() -> Box
    {
        return Box();
    }

    /// <summary> Creates a box constructed with the specified arguments. </summary>
    /// <remarks> This overload does not allows for allocators with context. </remarks>
    template<typename... Args>
    FORCE_INLINE NODISCARD
    static auto Make(Args&&... args) -> Box
    {
        Box box;
        box._allocData.Allocate(sizeof(T));
        new(box.Get()) T(FORWARD(Args..., args)...);

        return box;
    }

    /// <summary> Creates a box constructed with the specified arguments. </summary>
    /// <remarks> This overload allows for allocators with context. </remarks>
    template<typename... Args, typename AllocContext>
    FORCE_INLINE NODISCARD
    static auto MakeWithContext(AllocContext&& context, Args&&... args) -> Box
    {
        Box box;
        box._allocData = AllocData{ FORWARD(AllocContext, context) };
        box._allocData.Allocate(sizeof(T));
        new(box.Get()) T(FORWARD(Args, args)...);

        return box;
    }


    // Constraints

    static_assert(Alloc::IsNullable, "Allocator must be nullable.");
    static_assert(!std::is_reference<T>::value, "Type must not be a reference.");
};

