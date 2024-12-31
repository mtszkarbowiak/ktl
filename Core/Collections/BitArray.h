// Created by Mateusz Karbowiak 2024

#pragma once

#include "Collections/CollectionsUtils.h"

/// <summary>
/// Specialized container for storing dynamically resizable arrays of logic values.
/// It stores the elements in a contiguous memory block and uses doubling growth strategy.
/// </summary>
/// <remarks>
/// The container is designed to invoke the allocator as little as possible.
/// Thus it will keep the allocation active even when the array is empty,
/// unless explicitly freed by calling <c>Reset</c>.
/// </remarks>
/// <typeparam name="Alloc"> Type of the allocator to use. </typeparam>
template<
    typename Alloc = DefaultAlloc,
    int32(&Grow)(int32) = Growing::Default
>
class BitArray
{
public:
    /// <summary> Underlying data type used to store the bits. </summary>
    using Block     = uint64;

private:
    using AllocData   = typename Alloc::Data;
    using AllocHelper = AllocHelperOf<Block, Alloc, ARRAY_DEFAULT_CAPACITY, Grow>;

    AllocData _allocData{};
    int32     _blockCapacity{};
    int32     _bitCount{};

    constexpr static int32 BytesPerBlock = sizeof(Block);
    constexpr static int32  BitsPerBlock = BytesPerBlock * 8;


protected:
    FORCE_INLINE NODISCARD
    auto Data()  -> Block*
    {
        return DATA_OF(Block, _allocData);
    }

    FORCE_INLINE NODISCARD
    auto Data() const  -> const Block*
    {
        return DATA_OF(const Block, _allocData);
    }


    /// <summary> Calculates the number of blocks required to store the given number of bits. </summary>
    FORCE_INLINE NODISCARD
    static constexpr auto BlocksForBits(const int32 bitCount)  -> int32
    {
        return (bitCount + BitsPerBlock - 1) / BitsPerBlock;
    }

public:
    // Capacity Access

    /// <summary> Checks if the bit-array has an active allocation. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsAllocated() const  -> bool
    {
        return _blockCapacity > 0;
    }

    /// <summary> Number of bits that can be stored without invoking the allocator. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Capacity() const  -> int32
    {
        return _blockCapacity * BitsPerBlock;
    }


    // Count Access

    /// <summary> Checks if the bit-array has any bits. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto IsEmpty() const  -> bool
    {
        return _bitCount == 0;
    }

    /// <summary> Number of currently stored bits. </summary>
    FORCE_INLINE NODISCARD
    constexpr auto Count() const  -> int32
    {
        return _bitCount;
    }

    /// <summary> Number of bits that can be added without invoking the allocator. </summary>
    constexpr auto Slack() const  -> int32
    {
        return Capacity() - _bitCount;
    }


    // Allocation Manipulation

    /// <summary> Ensures that adding bits up to the requested capacity will not invoke the allocator. </summary>
    FORCE_INLINE
    void Reserve(const int32 minBitsCapacity) //TODO Unify naming with EnsureCapacity.
    {
        if (minBitsCapacity < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        const int32 minBlocksCapacity = BlocksForBits(minBitsCapacity);

        if (minBlocksCapacity <= _blockCapacity)
            return; // Reserving the same capacity would not increase the capacity.

        if (_blockCapacity == 0)
        {
            // If the array is empty, allocate the default capacity.
            const int32 requiredBlocksCapacity = AllocHelper::InitCapacity(minBlocksCapacity);
            _blockCapacity = AllocHelper::Allocate(_allocData, requiredBlocksCapacity);
        }
        else
        {
            // Higher capacity is required. Allocate new memory.
            const AllocData& oldData = _allocData;
            AllocData newData{ oldData }; // Copy the binding

            const int32 requiredBlocksCapacity = AllocHelper::NextCapacity(_blockCapacity, minBlocksCapacity);
            const int32 allocatedBlocksCapacity = AllocHelper::Allocate(newData, requiredBlocksCapacity);

            if (_blockCapacity > 0)
            {
                const int32 oldBlocksCount = BlocksForBits(_bitCount);
                BulkOperations::MoveLinearContent<Block>(
                    DATA_OF(Block, _allocData),
                    DATA_OF(Block, newData),
                    oldBlocksCount
                );
                _allocData.Free();
            }

            _allocData = MOVE(newData);
            _blockCapacity = allocatedBlocksCapacity;
        }
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the array is empty, the allocation will be freed.
    /// </summary>
    FORCE_INLINE
    void Compact() //TODO Test this method.
    {
        if (_bitCount == 0)
        {
            // If the array is empty, free the allocation.
            if (_blockCapacity > 0)
            {
                _allocData.Free();
                _blockCapacity = 0;
            }
            return;
        }

        const int32 blocksCount = BlocksForBits(_bitCount);
        const int32 requiredBlocksCapacity = AllocHelper::InitCapacity(blocksCount);

        if (_blockCapacity <= requiredBlocksCapacity)
            return;

        // Higher capacity is required. Allocate new memory.
        const AllocData& oldData = _allocData;
        AllocData newData{ oldData }; // Copy the binding

        const int32 allocatedBlocksCapacity = AllocHelper::Allocate(newData, requiredBlocksCapacity);

        BulkOperations::MoveLinearContent<Block>(
            DATA_OF(Block, _allocData), 
            DATA_OF(Block, newData), 
            BlocksForBits(_bitCount)
        );
        BulkOperations::DestroyLinearContent<Block>(
            DATA_OF(Block, _allocData),
            BlocksForBits(_bitCount)
        );

        _allocData.Free();

        _allocData     = MOVE(newData);
        _blockCapacity = allocatedBlocksCapacity;
    }


    // Element Access

    /// <summary> Utility class that allows accessing the bit at the specified index using the assignment operator. </summary>
    class MutBitRef final
    {
        BitArray* _array;
        int32     _index;

    public:
        explicit MutBitRef(BitArray* array, const int32 index)
            : _array{ array }
            , _index{ index }
        {
        }

        FORCE_INLINE
        auto operator=(const bool value) -> MutBitRef&
        {
            _array->SetBit(_index, value);
            return *this;
        }

        FORCE_INLINE
        operator bool() const
        {
            return _array->GetBit(_index);
        }
    };

    /// <summary> Utility class that allows accessing the bit at the specified index using the assignment operator. </summary>
    class ConstBitRef final
    {
        const BitArray* _array;
        int32           _index;

    public:
        explicit ConstBitRef(const BitArray* array, const int32 index)
            : _array{ array }
            , _index{ index }
        {
        }

        auto operator=(bool value)->ConstBitRef & = delete;

        FORCE_INLINE
        operator bool() const
        {
            return _array->GetBit(_index);
        }
    };


    /// <summary> Accesses the bit at the specified index. </summary>
    /// <remarks>
    /// This method uses a proxy object to allow the assignment operator to be used.
    /// To read bit without overhead use <c>GetBit</c> method.
    /// </remarks>
    FORCE_INLINE NODISCARD
    auto operator[](const int32 index) const -> ConstBitRef
    {
        return ConstBitRef{ this, index };
    }

    /// <summary> Accesses the bit at the specified index. </summary>
    /// <remarks>
    /// This method uses a proxy object to allow the assignment operator to be used.
    /// To modify bit without overhead use <c>SetBit</c> method.
    /// </remarks>
    FORCE_INLINE NODISCARD
    auto operator[](const int32 index) -> MutBitRef
    {
        return MutBitRef{ this, index };
    }


    // Element Manipulation

    /// <summary> Returns the bit value. </summary>
    /// <param name="index"> Index of the bit to access. Must be in the range [0, Count). </param>
    /// <returns> Value of the bit at the specified index. </returns>
    FORCE_INLINE NODISCARD
    auto GetBit(const int32 index) const -> bool
    {
        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < _bitCount);

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex = index % BitsPerBlock;

        const Block* srcBlock = DATA_OF(const Block, _allocData) + blockIndex;
        const Block  mask     = Block{ 1 } << bitIndex;
        const bool   result   = (*srcBlock & mask) != 0;

        return result;
    }

    /// <summary> Sets the bit value. </summary>
    /// <param name="index"> Index of the bit to set. Must be in the range [0, Count). </param>
    /// <param name="value"> Value to set. </param>
    FORCE_INLINE
    void SetBit(const int32 index, const bool value)
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index < _bitCount);

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;

        Block* dstBlock  = DATA_OF(Block, _allocData) + blockIndex;
        const Block mask = Block{ 1 } << bitIndex;

        if (value)
            *dstBlock |= mask;
        else
            *dstBlock &= ~mask;
    }

    /// <summary> Sets all bits to the specified value. </summary>
    FORCE_INLINE
    void SetAll(const bool value)
    {
        if (_bitCount == 0)
            return;

        const Block fillValue   = value ? ~Block{} : Block{};
        const int32 blocksCount = BlocksForBits(_bitCount);

        Block* blocks = DATA_OF(Block, _allocData);
        for (int32 i = 0; i < blocksCount; ++i)
            blocks[i] = fillValue;
    }


    /// <summary> Accesses the block of bits bit at the specified index. </summary>
    FORCE_INLINE NODISCARD
    auto GetBlock(const int32 blockIndex) const -> Block
    {
        ASSERT_COLLECTION_SAFE_ACCESS(blockIndex >= 0 && blockIndex < _blockCapacity);
        const Block* srcBlock = DATA_OF(Block, _allocData) + blockIndex;
        return *srcBlock;
    }

    /// <summary> Sets the block of bits bit at the specified index. </summary>
    FORCE_INLINE
    void SetBlock(const int32 blockIndex, const Block value)
    {
        ASSERT_COLLECTION_SAFE_MOD(blockIndex >= 0 && blockIndex < _blockCapacity);
        Block* dstBlock = DATA_OF(Block, _allocData) + blockIndex;
        *dstBlock = value;
    }

    /// <summary> Adds a bit to the end of the array. </summary>
    FORCE_INLINE
    void Add(const bool value)
    {
        const int32 bitIndex = _bitCount;
        Reserve(_bitCount + 1);
        ++_bitCount;
        SetBit(bitIndex, value);
    }


    /// <summary> Removes all bits from the array without freeing the allocation. </summary>
    FORCE_INLINE
    void Clear()
    {
        if (_bitCount == 0)
            return;

        const int32 blocksCount = BlocksForBits(_bitCount);
        BulkOperations::DestroyLinearContent<Block>(DATA_OF(Block, _allocData), blocksCount);
        _bitCount = 0;
    }

    /// <summary> Removes all bits from the array and frees the allocation. </summary>
    FORCE_INLINE
    void Reset()
    {
        if (_blockCapacity == 0)
            return;

        Clear();

        _allocData.Free(); // Capacity is above zero!
        _blockCapacity = 0;
    }


    /// <summary> Inserts a bit without changing the order of the other bits. </summary>
    void InsertAtStable(const int32 index, const bool value)
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index <= _bitCount); // Allow index == _bitCount for appending

        Reserve(_bitCount + 1); // Ensure enough space for the new bit.

        const int32 blockIndex  = index / BitsPerBlock;
        const int32 bitIndex    = index % BitsPerBlock;
        const int32 blocksCount = BlocksForBits(_bitCount);

        Block* blocks = DATA_OF(Block, _allocData);

        if (index < _bitCount)
        {
            // Shift all bits to the right starting from the insertion point.
            for (int32 i = blocksCount - 1; i > blockIndex; --i)
            {
                const Block prevBlock = blocks[i - 1];
                blocks[i] = (blocks[i] >> 1) | (prevBlock << (BitsPerBlock - 1));
            }

            // Handle the block containing the insertion point.
            Block& targetBlock = blocks[blockIndex];
            const Block lowerMask = (Block{ 1 } << bitIndex) - 1; // Mask for bits below insertion.
            const Block upperBits = targetBlock & ~lowerMask;   // Preserve bits above insertion.
            const Block lowerBits = targetBlock & lowerMask;   // Preserve bits below insertion.

            targetBlock = (upperBits << 1) | lowerBits; // Merge shifted upper bits and lower bits.
        }

        // Insert the new bit.
        Block& block = blocks[blockIndex];
        const Block mask = Block{ 1 } << bitIndex;
        if (value)
            block |= mask;
        else
            block &= ~mask;

        ++_bitCount;
    }

    /// <summary> Removes the bit at the specified index without changing the order of the other bits. </summary>
    void RemoveAtStable(const int32 index)
    {
        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index < _bitCount); // Ensure the index is valid.

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex = index % BitsPerBlock;
        const int32 blocksCount = BlocksForBits(_bitCount);

        Block* blocks = DATA_OF(Block, _allocData);

        // Handle the block containing the removed bit.
        {
            Block& targetBlock = blocks[blockIndex];
            const Block lowerMask = (Block{ 1 } << bitIndex) - 1; // Mask for bits below the removed bit.
            const Block upperMask = ~lowerMask & ~(Block{ 1 } << bitIndex); // Mask for bits above the removed bit.

            const Block lowerBits = targetBlock & lowerMask;       // Preserve bits below the removed bit.
            const Block upperBits = (targetBlock & upperMask) >> 1; // Shift bits above the removed bit down.

            targetBlock = lowerBits | upperBits; // Merge lower and shifted upper bits.
        }

        // Shift remaining bits to the left for subsequent blocks.
        for (int32 i = blockIndex + 1; i < blocksCount; ++i)
        {
            const Block currentBlock = blocks[i];
            const Block carryBit = currentBlock & Block{ 1 }; // The lowest bit to carry.
            blocks[i - 1] |= carryBit << (BitsPerBlock - 1); // Carry it to the previous block.
            blocks[i] = currentBlock >> 1;                  // Shift current block to the left.
        }

        --_bitCount;
    }


    // Collection Lifecycle - Overriding Content

private:
    FORCE_INLINE
    void MoveToEmpty(BitArray&& other) noexcept
    {
        ASSERT_COLLECTION_SAFE_MOD(_bitCount == 0 && _blockCapacity == 0); // BitArray must be empty, but the collection must be initialized!

        if (other._bitCount == 0 ||other._blockCapacity == 0)
            return;

        if (other._allocData.MovesItems())
        {
            _allocData     = MOVE(other._allocData);
            _blockCapacity = other._blockCapacity;
            _bitCount      = other._bitCount;

            other._blockCapacity = 0; // Allocation moved - Reset the capacity!
            other._bitCount      = 0;
        }
        else
        {
            _bitCount = other._bitCount;

            const int32 requiredBlocks = BlocksForBits(_bitCount);
            const int32 requestedBlocksCapacity = AllocHelper::InitCapacity(requiredBlocks);

            _blockCapacity = AllocHelper::Allocate(_allocData, requestedBlocksCapacity);
            _bitCount = other._bitCount;

            BulkOperations::MoveLinearContent<Block>(
                DATA_OF(Block, other._allocData),
                DATA_OF(Block, this->_allocData),
                requiredBlocks
            );

            other.Reset();
        }
    }

    void CopyToEmpty(const BitArray& other)
    {
        ASSERT_COLLECTION_SAFE_MOD(_bitCount == 0 && _blockCapacity == 0); // BitArray must be empty, but the collection must be initialized!

        if (other._bitCount == 0 || other._blockCapacity == 0)
            return;

        const int32 requiredBlocks = BlocksForBits(other._bitCount);
        const int32 requestedBlocksCapacity = AllocHelper::InitCapacity(requiredBlocks);

        _blockCapacity = AllocHelper::Allocate(_allocData, requestedBlocksCapacity);
        _bitCount = other._bitCount;

        BulkOperations::CopyLinearContent<Block>(
            DATA_OF(const Block, other._allocData),
            DATA_OF(Block, this->_allocData),
            requiredBlocks
        );
    }


    // Collection Lifecycle - Constructors

public:
    /// <summary> Initializes an empty bit-array with no active allocation. </summary>
    FORCE_INLINE
    constexpr BitArray() = default;

    /// <summary> Initializes a bit-array by moving the allocation from another array. </summary>
    FORCE_INLINE
    BitArray(BitArray&& other) noexcept
    {
        MoveToEmpty(MOVE(other));
    }

    /// <summary> Initializing an empty bit-array with an active context-less allocation. </summary>
    BitArray(const BitArray& other)
    {
        CopyToEmpty(other);
    }


    /// <summary> Initializing an empty bit-array with an active context-less allocation of the specified capacity. </summary>
    FORCE_INLINE
    explicit BitArray(const int32 bitCapacity)
        : _allocData{}
        , _bitCount{}
    {
        const int32 requiredBlocks  = BlocksForBits(bitCapacity);
        const int32 allocatedMemory = _allocData.Allocate(requiredBlocks * BytesPerBlock);
        _blockCapacity = allocatedMemory / BytesPerBlock;
    }

    /// <summary> Initializes an empty bit-array with an active allocation of the specified capacity and context. </summary>
    template<typename AllocContext>
    FORCE_INLINE
    explicit BitArray(const int32 bitCapacity, AllocContext&& context)
        : _allocData{ FORWARD(AllocContext, context) }
        , _bitCount{}
    {
        const int32 requiredBlocks  = BlocksForBits(bitCapacity);
        const int32 allocatedMemory = _allocData.Allocate(requiredBlocks * BytesPerBlock);
        _blockCapacity = allocatedMemory / BytesPerBlock;
    }


    // Collection Lifecycle - Assignments

    FORCE_INLINE
    BitArray& operator=(BitArray&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            MoveToEmpty(MOVE(other));
        }
        return *this;
    }

    BitArray& operator=(const BitArray& other)
    {
        if (this != &other)
        {
            Reset();
            CopyToEmpty(other);
        }
        return *this;
    }


    // Collection Lifecycle - Destructor

    /// <summary> Destructor. </summary>
    ~BitArray()
    {
        Reset();
    }


    // Iterators

    /// <summary>
    /// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
    /// </summary>
    /// <remarks>
    /// Warning: Dereferencing the iterator returns a mutable bit reference, not a value.
    /// </remarks>
    class MutEnumerator
    {
        BitArray* _array;
        int32     _index;

    public:
        explicit MutEnumerator(BitArray& array)
            : _array{ &array }
            , _index{ 0 }
        {
        }


        // Identity

        FORCE_INLINE NODISCARD
        auto operator==(const MutEnumerator& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array == other._array);
            return _index == other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator!=(const MutEnumerator& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array == other._array);
            return _index != other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator<(const MutEnumerator& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array == other._array);
            return _index < other._index;
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        FORCE_INLINE NODISCARD
        IterHint Hint() const
        {
            const int32 remaining = _array->Count() - _index;
            return { remaining, remaining };
        }

        FORCE_INLINE NODISCARD
        auto operator*() -> MutBitRef
        {
            return MutBitRef{ _array, _index };
        }

        FORCE_INLINE NODISCARD
        auto operator*() const -> ConstBitRef
        {
            return ConstBitRef{ _array, _index };
        }


        // End Condition

        /// <summary> Check if the enumerator reached the end of the array. </summary>
        FORCE_INLINE NODISCARD
        explicit operator bool() const 
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array != nullptr);
            return _index < _array->_bitCount;
        }

        /// <summary> Returns the index of the current element. </summary>
        FORCE_INLINE
        auto Index() const  -> int32
        {
            return _index;
        }


        // Movement

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++() -> MutEnumerator&
        {
            ++_index;
            return *this;
        }

        /// <summary> Moves the enumerator to the next element. </summary>
        FORCE_INLINE
        auto operator++(int) -> MutEnumerator
        {
            MutEnumerator copy{ *this };
            ++_index;
            return copy;
        }
    };

    /// <summary>
    /// Iterator for the array which provides end condition and allows to iterate over the elements in a range-based for loop.
    /// </summary>
    /// <remarks>
    /// Warning: Dereferencing the iterator returns a const bit reference, not a value.
    /// </remarks>
    class ConstEnumerator
    {
        const BitArray* _array;
        int32           _index;

    public:
        explicit ConstEnumerator(const BitArray& array)
            : _array{ &array }
            , _index{ 0 }
        {
        }


        // Identity

        FORCE_INLINE NODISCARD
        auto operator==(const ConstEnumerator& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array == other._array);
            return _index == other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator!=(const ConstEnumerator& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array == other._array);
            return _index != other._index;
        }

        FORCE_INLINE NODISCARD
        auto operator<(const ConstEnumerator& other) const -> bool
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array == other._array);
            return _index < other._index;
        }


        // Access

        /// <summary> Returns the size hint about the numer of remaining elements. </summary>
        FORCE_INLINE NODISCARD
        IterHint Hint() const
        {
            const int32 remaining = _array->Count() - _index;
            return { remaining, remaining };
        }

        FORCE_INLINE NODISCARD
        auto operator*() const -> ConstBitRef
        {
            return ConstBitRef{ _array, _index };
        }


        // End Condition and Movement

        FORCE_INLINE NODISCARD
        explicit operator bool() const 
        {
            ASSERT_COLLECTION_SAFE_ACCESS(_array != nullptr);
            return _index < _array->_bitCount;
        }

        FORCE_INLINE
        auto operator++() -> ConstEnumerator&
        {
            ++_index;
            return *this;
        }
    };

    FORCE_INLINE NODISCARD
    auto Values() -> MutEnumerator
    {
        return MutEnumerator{ *this };
    }

    FORCE_INLINE NODISCARD
    auto Values() const -> ConstEnumerator
    {
        return ConstEnumerator{ *this };
    }
};
