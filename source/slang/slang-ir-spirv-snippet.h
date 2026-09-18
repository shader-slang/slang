// slang-ir-spirv-legalize.h
#pragma once
#include "core/slang-basic.h"
#include "spirv/unified1/spirv.h"

namespace Slang
{

struct SPIRVCoreGrammarInfo;

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
        // Refer to the GLSL450 Instruction Set.
        GLSL450ExtInstSet,
        // A select expression based on whether result type is float, e.g.
        // `fi(x,y)` selects `x` if resultType is `float`.
        FloatIntegerSelection,
        // A select expression based on whether result type is float, unsigned
        // or signed integer. e.g. `fus(f_opcode, u_opcode, s_opcode)`.
        FloatUnsignedSignedSelection,
        // Reference to a type defined in `ASMType`.
        TypeReference,
        // Reference to a Constant defined in `SpvSnippet::constants`.
        ConstantReference,
    };

    struct ASMOperand
    {
        ASMOperandType type;

        // The value of the spv word when type is `SpvWord`, or
        // the reference name when type is `ObjectReference`
        // (e.g. an argument reference (_1) has `content` == 1).
        SpvWord content;

        // Additional value contents.
        SpvWord content2;
        SpvWord content3;
    };

    enum class ASMType : SpvWord
    {
        None,
        Int,
        UInt,
        UInt16,
        Half,
        Float,
        Double,
        FloatOrDouble, // Float or double type, depending on the result type of the intrinsic.
        Float2,
        UInt2,
    };

    // Returns whether the SPIR-V emitter can lower `type`, both as a `_type(...)` operand
    // (emitSpvSnippetASMTypeOperand) and as a `const(...)` operand (emitSpvConstant); those two
    // switches handle exactly this set. A `const(...)` operand of type `FloatOrDouble` is first
    // resolved to `Float`/`Double` (resolveSnippetConstantType) before this predicate applies.
    static bool isEmittableASMType(ASMType type);

    // Returns a human-readable spelling of `type` for diagnostics, since a snippet diagnostic can
    // only point at the intrinsic call site, not at the offending token inside the snippet string.
    static UnownedStringSlice getASMTypeName(ASMType type);

    // Capacity of a `const(...)` operand's value arrays below, and the largest number of
    // comma-separated values the parser accepts before treating the list as malformed. The widest
    // emitted ASMType is two components (Float2/UInt2), so four is ample headroom.
    static const int kMaxASMConstantValues = 4;

    struct ASMConstant
    {
        ASMType type;
        SpvWord intValues[kMaxASMConstantValues];
        float floatValues[kMaxASMConstantValues];
        HashCode getHashCode() const
        {
            HashCode result = (HashCode)type;
            for (int i = 0; i < kMaxASMConstantValues; i++)
            {
                switch (type)
                {
                case ASMType::Half:
                case ASMType::Float:
                case ASMType::Double:
                case ASMType::Float2:
                case ASMType::FloatOrDouble:
                    result = combineHash(result, Slang::getHashCode(floatValues[i]));
                    break;
                default:
                    result = combineHash(result, Slang::getHashCode(intValues[i]));
                    break;
                }
            }
            return result;
        }
        bool operator==(const ASMConstant& other) const
        {
            if (type != other.type)
                return false;
            switch (type)
            {
            case ASMType::Half:
            case ASMType::Float:
            case ASMType::Double:
            case ASMType::FloatOrDouble:
                return floatValues[0] == other.floatValues[0];
            case ASMType::Float2:
                return floatValues[0] == other.floatValues[0] &&
                       floatValues[1] == other.floatValues[1];
            case ASMType::Int:
                return intValues[0] == other.intValues[0];
            case ASMType::UInt:
            case ASMType::UInt16:
                return intValues[0] == other.intValues[0];
            case ASMType::UInt2:
                return intValues[0] == other.intValues[0] && intValues[1] == other.intValues[1];
            default:
                return false;
            }
        }
    };

    struct ASMInst
    {
        SpvWord opCode = 0;
        List<ASMOperand> operands;
        // The `%name` this instruction defines (empty if unnamed). Retained so a diagnostic about
        // an InstReference can echo the name the user wrote rather than its resolved index.
        String resultName;
    };

    List<ASMInst> instructions;
    HashSet<SpvStorageClass> usedPtrResultTypeStorageClasses;
    List<ASMConstant> constants;
    SpvStorageClass resultStorageClass = SpvStorageClassMax;

    static RefPtr<SpvSnippet> parse(
        const SPIRVCoreGrammarInfo& spirvGrammar,
        UnownedStringSlice definition);
};


} // namespace Slang
