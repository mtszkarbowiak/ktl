I. Introduction

1. Objective

...

2. Assumptions
    
    1. Exceptions are used only for unrecoverable errors.
    2. Memory allocation is a critical operation.
    3. No allocation, collection size, etc. can exceed 2^32-1, thus 32-bit integer is enough.
    4. Moving objects is almost always faster than copying them thus move semantics are preferred.

II. Rules

1. Allocation data represents binding between allocation context and allocated unit of memory.
    1.1. Allocation data can store zero or one continuous memory block.
    1.2. Alloc-data destructor does NOT have to free memory.
    1.3. Alloc-data is not aware of stored type.
    1.4. Alloc-data discovers stored block size on allocation but is NOT responsible for remembering it.
    1.5. Every alloc-data must provide default constructor but it may put object into invalid state.
    1.6. Copying alloc-data means copying binding between allocation context and a new alloc-data.
    1.8. Moving alloc-data means moving binding between allocation context and a new alloc-data.
    1.9. Moving alloc-data may or may not move items during its own move. Collection can ask for it.
    1.10. Alloc-data may fail to allocate memory. It must return zero allocated capacity.
    1.11. Allocation policy shares information about min and max size of allocation.
    1.12. Alloc-data takes charge for data alignment.

2. Collections manage objects lifetimes, never allocators.
    2.1. Collections may assume that every object is moveable but not necessarily copyable.
    2.2. Collections must expose API for reserving or reducing capacity.
    2.3. Collections should treat C-style types with fast memory operations.
