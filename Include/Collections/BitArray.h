// GameDev Template Library - Created by Mateusz Karbowiak 2024-25
// Repository: https://github.com/mtszkarbowiak/ktl/
//
// This project is licensed under the MIT License, which allows you to use, modify, distribute,
// and sublicense the code as long as the original license is included in derivative works.
// See the LICENSE file for more details.

#pragma once

#include "Collections/CollectionsUtils.h"
#include "Types/BitPuller.h"

/// <summary>
/// A specialized container for storing dynamically
/// resizable arrays of logical values (bits).
/// </summary>
///
/// <typeparam name="A">
/// (Optional) The type of the allocator to use.
/// Can be either a dragging or non-dragging allocator.
/// </typeparam>
/// <typeparam name="G">
/// (Optional) A reference to a function that calculates the next capacity
/// before applying allocator limits.
/// </typeparam>
///
/// <remarks>
/// 1. The <c>BitArray</c> class STL equivalent is the specialization of <c>std::vector<bool></c>.
/// 2. It operates effectively as a stack, with the array's end representing the top of the stack.
/// 3. The amortized time complexity of adding elements is constant.
/// 4. The default capacity is defined by <c>ARRAY_DEFAULT_CAPACITY</c>.
/// 5. The container minimizes allocator invocations, keeping the allocation active even when the
/// array is empty, unless explicitly freed by calling <c>Reset</c> (or destructor).
/// 6. <c>BitArray</c> is not thread-safe.
/// External synchronization is required if used in a multi-threaded environment.
/// </remarks>
template<
    typename A = DefaultAlloc,
    typename G = DefaultGrowth
>
class BitArray
{
public:
    /// <summary> Underlying data type used to store the bits. </summary>
    using AllocData   = typename A::Data;
    using AllocHelper = AllocHelperOf<BitsStorage::Block, A, ARRAY_DEFAULT_CAPACITY, G>;

PRIVATE:
    AllocData _allocData{ NullOptT{} };
    int32     _blockCapacity{};
    int32     _bitCount{};


protected:
    FORCE_INLINE
    auto Data() & -> BitsStorage::Block*
    {
        return DATA_OF(BitsStorage::Block, _allocData);
    }

    FORCE_INLINE
    auto Data() const& -> const BitsStorage::Block*
    {
        return DATA_OF(const BitsStorage::Block, _allocData);
    }


public:
    // Capacity Access

    /// <summary> Checks if the bit-array has an active allocation. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsAllocated() const -> bool
    {
        return _blockCapacity > 0;
    }

    /// <summary> Number of bits that can be stored without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Capacity() const -> int32
    {
        return _blockCapacity * BitsStorage::BitsPerBlock;
    }


    // Count Access

    /// <summary> Checks if the bit-array has any bits. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto IsEmpty() const -> bool
    {
        return _bitCount == 0;
    }

    /// <summary> Number of currently stored bits. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Count() const -> int32
    {
        return _bitCount;
    }

    /// <summary> Number of bits that can be added without invoking the allocator. </summary>
    NO_DISCARD FORCE_INLINE constexpr
    auto Slack() const -> int32
    {
        return Capacity() - _bitCount;
    }


    // Allocation Manipulation

    /// <summary>
    /// Ensures that adding bits up to the requested capacity will not invoke the allocator.
    /// </summary>
    void Reserve(const int32 minBitsCapacity)
    {
        using namespace BitsStorage;

        if (minBitsCapacity < 1)
            return; // Reserving 0 (or less) would never increase the capacity.

        const int32 minBlocksCapacity = BlocksForBits(minBitsCapacity);

        if (minBlocksCapacity <= _blockCapacity)
            return; // Reserving the same capacity would not increase the capacity.

        if (_blockCapacity == 0)
        {
            // If the array is empty, allocate the default capacity.
            const int32 requiredBlocksCapacity
                = AllocHelper::InitCapacity(minBlocksCapacity);
            _blockCapacity = AllocHelper::Allocate(_allocData, requiredBlocksCapacity);
        }
        else
        {
            // Higher capacity is required. Allocate new memory.
            const AllocData& oldData = _allocData;
            AllocData newData{ oldData }; // Copy the binding

            const int32  requiredBlocksCapacity
                = AllocHelper::NextCapacity(_blockCapacity, minBlocksCapacity);
            const int32 allocatedBlocksCapacity
                = AllocHelper::Allocate(newData, requiredBlocksCapacity);

            const int32 oldBlocksCount = BlocksForBits(_bitCount);
            if (oldBlocksCount > 0)
            {
                BulkOperations::MoveLinearContent<Block>(
                    DATA_OF(Block, _allocData),
                    DATA_OF(Block, newData),
                    oldBlocksCount
                );
            }

            _allocData.Free();
            _allocData = MOVE(newData);
            _blockCapacity = allocatedBlocksCapacity;
        }
    }

    /// <summary>
    /// Attempts to reduce the capacity to the number of stored elements, without losing any elements.
    /// If the array is empty, the allocation will be freed.
    /// </summary>
    void Compact() //TODO(mtszkarbowiak) Test Test BitArray::Compact
    {
        using namespace BitsStorage;

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

    /// <summary> Accesses the bit at the specified index. </summary>
    /// <remarks>
    /// This method uses a proxy object to allow the assignment operator to be used.
    /// To read bit without overhead use <c>GetBit</c> method.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    auto operator[](const int32 index) const -> ConstBitRef
    {
        using namespace BitsStorage;

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;

        return ConstBitRef{
            DATA_OF(const Block, _allocData) + blockIndex,
            bitIndex
        };
    }

    /// <summary> Accesses the bit at the specified index. </summary>
    /// <remarks>
    /// This method uses a proxy object to allow the assignment operator to be used.
    /// To modify bit without overhead use <c>SetBit</c> method.
    /// </remarks>
    NO_DISCARD FORCE_INLINE
    auto operator[](const int32 index) -> MutBitRef
    {
        using namespace BitsStorage;

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;

        return MutBitRef{
            DATA_OF(Block, _allocData) + blockIndex,
            bitIndex
        };
    }


    // Element Manipulation

    /// <summary> Returns the bit value. </summary>
    /// <param name="index"> Index of the bit to access. Must be in the range [0, Count). </param>
    /// <returns> Value of the bit at the specified index. </returns>
    NO_DISCARD FORCE_INLINE
    auto GetBit(const int32 index) const -> bool
    {
        using namespace BitsStorage;

        ASSERT_COLLECTION_SAFE_ACCESS(index >= 0 && index < _bitCount);

        const int32 blockIndex = index / BitsPerBlock;
        const int32 bitIndex   = index % BitsPerBlock;

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
        using namespace BitsStorage;

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
        using namespace BitsStorage;

        if (_bitCount == 0)
            return;

        const Block fillValue   = value ? FullBlock : EmptyBlock;
        const int32 blocksCount = BlocksForBits(_bitCount);

        Block* blocks = DATA_OF(Block, _allocData);
        for (int32 i = 0; i < blocksCount; ++i)
            blocks[i] = fillValue;
    }


    /// <summary> Accesses the block of bits bit at the specified index. </summary>
    NO_DISCARD FORCE_INLINE
    auto GetBlock(const int32 blockIndex) const -> BitsStorage::Block
    {
        using namespace BitsStorage;

        ASSERT_COLLECTION_SAFE_ACCESS(blockIndex >= 0 && blockIndex < _blockCapacity);
        const Block* srcBlock = DATA_OF(Block, _allocData) + blockIndex;
        return *srcBlock;
    }

    /// <summary> Sets the block of bits bit at the specified index. </summary>
    FORCE_INLINE
    void SetBlock(const int32 blockIndex, const BitsStorage::Block value)
    {
        using namespace BitsStorage;

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
        using namespace BitsStorage;

        if (_bitCount == 0)
            return;

        const int32 blocksCount = BlocksForBits(_bitCount);
        BulkOperations::DestroyLinearContent<Block>(
            DATA_OF(Block, _allocData), 
            blocksCount
        );

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
        using namespace BitsStorage;

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
        using namespace BitsStorage;

        ASSERT_COLLECTION_SAFE_MOD(index >= 0 && index < _bitCount); // Ensure the index is valid.

        const int32 blockIndex  = index / BitsPerBlock;
        const int32 bitIndex    = index % BitsPerBlock;
        const int32 blocksCount = BlocksForBits(_bitCount);

        Block* blocks = DATA_OF(BitsStorage::Block, _allocData);

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

PRIVATE:
    void MoveToEmpty(BitArray&& other) noexcept
    {
        using namespace BitsStorage;

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
        using namespace BitsStorage;

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
    FORCE_INLINE constexpr
    BitArray() = default;

    /// <summary> Initializes a bit-array by moving the allocation from another array. </summary>
    FORCE_INLINE
    BitArray(BitArray&& other) noexcept
    {
        MoveToEmpty(MOVE(other));
    }

    /// <summary> Initializing an empty bit-array with an active context-less allocation. </summary>
    FORCE_INLINE
    BitArray(const BitArray& other)
    {
        CopyToEmpty(other);
    }


    /// <summary> Initializes an empty bit-array with an active allocation of the specified capacity and context. </summary>
    template<typename C_ = NullOptT>
    FORCE_INLINE explicit
    BitArray(const int32 bitCapacity, C_&& context = NullOptT{})
        : _allocData{ FORWARD(C_, context) }
    {
        using namespace BitsStorage;
        const int32 requiredBlocks  = BlocksForBits(bitCapacity);
        const int32 allocatedMemory = _allocData.Allocate(requiredBlocks * BytesPerBlock);
        _blockCapacity = allocatedMemory / BytesPerBlock;
    }


    // Collection Lifecycle - Assignments

    FORCE_INLINE
    auto operator=(BitArray&& other) noexcept -> BitArray&
    {
        if (this != &other)
        {
            Reset();
            MoveToEmpty(MOVE(other));
        }
        return *this;
    }

    FORCE_INLINE
    auto operator=(const BitArray& other) -> BitArray&
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

    NO_DISCARD FORCE_INLINE
    auto Values() -> BitMutPuller
    {
        return BitMutPuller{
            DATA_OF(BitsStorage::Block, _allocData),
            0,
            _bitCount
        };
    }

    NO_DISCARD FORCE_INLINE
    auto Values() const -> BitConstPuller
    {
        return BitConstPuller{
            DATA_OF(const BitsStorage::Block, _allocData),
            0,
            _bitCount
        };
    }
};
