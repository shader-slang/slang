// slang-compound-intrinsics.h
#pragma once

// Intrinsic functions in the Slang standard library are marked
// with the `__intrinsic_op(...)` modifier. Many of these map
// one-to-one to instruction opcodes in the Slang IR, and the
// argument to `__intrinsic_op(...)` is the IR instruction
// opcode in that case.
//
// In other cases, we have intrinsic operations like the `+=` or
// `&&` operator that either need to map to multiple IR instructions
// (or more generally, a number of instructions not equal to one),
// or otherwise have complications thake one-to-one lowering
// not possible.
//
// We refer to these as "compound" intrinsic ops, since the common
// case is that they represent a composition of multiple instructions.
//
// In order to not conflict with the opcodes of any IR instructions,
// these compound intrinsic ops will all be identified by *negative*
// integer opcodes.

// We start by defining an "X-macro" that lists all the compound
// intrinsic ops we support.

#define FOREACH_COMPOUND_INTRINSIC_OP(M) \
    M(Pos)                   \
    M(PreInc)                \
    M(PreDec)                \
    M(PostInc)               \
    M(PostDec)               \
    M(AddAssign)             \
    M(SubAssign)             \
    M(MulAssign)             \
    M(DivAssign)             \
    M(IRemAssign)            \
    M(FRemAssign)            \
    M(AndAssign)             \
    M(OrAssign)              \
    M(XorAssign )            \
    M(LshAssign)             \
    M(RshAssign)             \
    M(Assign)                \
    M(And)                   \
    M(Or)                    \
    /* end */

// We will use a simple type alias to capture the fact that
// a 32-bit integer is sufficient to represent compound
// intrinsic ops (as negative values) plus IR opcode values
// for single-instruction intrinsics (as non-negative values)
//
typedef int32_t IntrinsicOp;

// Next we use an enumeration declaration as an implementation
// detail, to associate each of the above cases with a (positive)
// integer.
//
enum class _CompoundIntrinsicOpVal : IntrinsicOp
{
#define DECLARE_COMPOUND_INTRINSIC_OP_VAL(NAME) NAME,
    FOREACH_COMPOUND_INTRINSIC_OP(DECLARE_COMPOUND_INTRINSIC_OP_VAL)
#undef DECLARE_COMPOUND_INTRINSIC_OP_VAL
};

// Finally, we define a second enumeration that takes the values
// from the first and performs a bitwise negation on them, which
// guarantees we get strictly negative values.

    /// Compound/complex intrinsic operations, which do not map to a single IR instruction.
    ///
    /// All of the values of this enumeration are guaranteed to be negative, and thus
    /// cannot conflict with any valid value of type `IROp`
    ///
enum CompoundIntrinsicOp : IntrinsicOp
{
#define DECLARE_COMPOUND_INTRINSIC_OP(NAME) kCompoundIntrinsicOp_##NAME = ~IntrinsicOp(_CompoundIntrinsicOpVal::NAME),
    FOREACH_COMPOUND_INTRINSIC_OP(DECLARE_COMPOUND_INTRINSIC_OP)
#undef DECLARE_COMPOUND_INTRINSIC_OP
};
