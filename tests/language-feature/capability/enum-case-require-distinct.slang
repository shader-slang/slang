//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl
// non-exhaustive: duplicate-case diagnostics can include the original case's location.

// These enum declarations are deliberately unused: validity does not depend on a cast.
enum SameRequirements
{
    [require(hlsl)] first = 1,
    [require(hlsl)] second = 1,
//CHECK: cases of capability-restricted enum 'SameRequirements' must have distinct values
}

enum DifferentRequirements
{
    [require(hlsl)] first = 1,
    [require(spirv)] second = 1,
//CHECK: cases of capability-restricted enum 'DifferentRequirements' must have distinct values
}

enum UnannotatedDuplicate
{
    [require(hlsl)] first = 1,
    second = 1,
//CHECK: cases of capability-restricted enum 'UnannotatedDuplicate' must have distinct values
}

enum LaterRequirement
{
    first = 1,
    second = 1,
//CHECK: cases of capability-restricted enum 'LaterRequirement' must have distinct values
    [require(hlsl)] third = 2,
}

enum ExplicitAlias
{
    [require(hlsl)] first = 1,
    second = first,
//CHECK: cases of capability-restricted enum 'ExplicitAlias' must have distinct values
}

enum ImplicitCollision
{
    [require(hlsl)] first = 0,
    second = 2,
    third = 1,
    fourth,
//CHECK: cases of capability-restricted enum 'ImplicitCollision' must have distinct values
}

enum NarrowedCollision : uint8_t
{
    [require(hlsl)] first = 0,
    second = uint8_t(256),
//CHECK: cases of capability-restricted enum 'NarrowedCollision' must have distinct values
}

enum ImplicitWrappedCollision : uint8_t
{
    [require(hlsl)] first = 0,
    last = 255,
    wrapped,
//CHECK: cases of capability-restricted enum 'ImplicitWrappedCollision' must have distinct values
}

enum WideCollision : uint64_t
{
    [require(hlsl)] first = 0xffffffffffffffffUL,
    second = uint64_t(-1),
//CHECK: cases of capability-restricted enum 'WideCollision' must have distinct values
}
