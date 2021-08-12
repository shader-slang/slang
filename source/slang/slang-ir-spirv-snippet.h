// slang-ir-spirv-legalize.h
#pragma once
#include "../core/slang-basic.h"
#include "spirv/unified1/spirv.h"

namespace Slang
{
//
// [2.2: Terms]
//
// > Word: 32 bits.
//
// Despite the importance to SPIR-V, the `spirv.h` header doesn't
// define a type for words, so we'll do it here.

/// A SPIR-V word.
typedef uint32_t SpvWord;

/// Represents a parsed Spv ASM from intrinsic definition.
struct SpvSnippet : public RefObject
{
    enum class ASMOperandType
    {
        // Plain SpvWord to inline without modifications.
        SpvWord,
        // Represents the result type of the intrinsic.
        ResultTypeId,
        // Represents the result Id of the ASM inst.
        ResultId,
        // Represents a reference to an intrinsic argument (e.g. `_1`).
        ObjectReference,
        // Represents a reference to an ASM inst (e.g. `%t`).
        InstReference,
    };

    struct ASMOperand
    {
        ASMOperandType type;

        // The value of the spv word when type is `SpvWord`, or
        // the reference name when type is `ObjectReference`
        // (e.g. an argument reference (_1) has `content` == 1).
        int content;
    };

    struct ASMInst
    {
        SpvWord opCode;
        List<ASMOperand> operands;
    };

    List<ASMInst> instructions;
    List<SpvStorageClass> usedResultTypeStorageClasses;

    SpvStorageClass resultStorageClass = SpvStorageClassMax;

    static RefPtr<SpvSnippet> parse(UnownedStringSlice definition);
};


}
