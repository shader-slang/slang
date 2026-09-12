#include "parser.h"
#include <cstddef>

namespace spvdb
{

// SPIR-V magic numbers (native and byte-swapped).
static constexpr uint32_t kMagic = 0x07230203u;
static constexpr uint32_t kMagicRev = 0x03022307u;

static uint32_t swap32(uint32_t w)
{
    return ((w & 0xFF000000u) >> 24) | ((w & 0x00FF0000u) >> 8) | ((w & 0x0000FF00u) << 8)
         | ((w & 0x000000FFu) << 24);
}

// Opcodes that carry <type-id> as their first operand.
// (All opcodes that produce a result carry <type-id> <result-id> unless they
// are OpTypeXxx or a handful of special cases — we detect this per the SPIR-V
// spec: if an opcode's result-id range includes both a type and a result, it
// has both; type-only opcodes have neither type nor result as separate fields.
//
// Rather than encoding a full grammar table, we use the standard SPIR-V rule:
//   - If the opcode has a result-id, then words[1] == result_id.
//   - If the opcode ALSO has a type-id, then words[0] == type_id.
// OpTypeXxx opcodes have a result-id (the type's id) but NO type-id.
// We detect "has type operand" vs "no type operand" via the grammar below.)
//
// We keep a small lookup table for the subset of opcodes we care about.
// The full SPIRV grammar is available in the SPIRV-Headers JSON but we
// don't want the JSON parsing overhead at parse time.  Instead we encode
// the two bits we need (has_type, has_result) per opcode.

enum OpcodeFlags : uint8_t
{
    F_NONE = 0,
    F_RESULT = 1 << 0, // instruction produces a result-id
    F_TYPE = 1 << 1, // instruction carries a type-id before result-id
};

// Returns flags for a given opcode.  Derived from the SPIR-V 1.6 spec.
// This doesn't need to be exhaustive — unknown opcodes default to F_NONE
// which means the operand words are stored as-is without pre-extraction.
static uint8_t opcode_flags(SpvOp op)
{
    // Opcodes that produce a result but carry no type-id:
    // (All OpTypeXxx, OpLabel, OpExtInstImport, OpString, OpDecorationGroup,
    //  OpFunction, OpFunctionParameter, and a few others.)
    switch (op) {
        // Type declarations: have result-id, no type-id before it.
        case SpvOpTypeVoid:
        case SpvOpTypeBool:
        case SpvOpTypeInt:
        case SpvOpTypeFloat:
        case SpvOpTypeVector:
        case SpvOpTypeMatrix:
        case SpvOpTypeImage:
        case SpvOpTypeSampler:
        case SpvOpTypeSampledImage:
        case SpvOpTypeArray:
        case SpvOpTypeRuntimeArray:
        case SpvOpTypeStruct:
        case SpvOpTypeOpaque:
        case SpvOpTypePointer:
        case SpvOpTypeFunction:
        case SpvOpTypeEvent:
        case SpvOpTypeDeviceEvent:
        case SpvOpTypeReserveId:
        case SpvOpTypeQueue:
        case SpvOpTypePipe:
        case SpvOpTypeForwardPointer: return F_RESULT;

        // Other result-only (no type-id):
        case SpvOpLabel:
        case SpvOpExtInstImport:
        case SpvOpString:
        case SpvOpDecorationGroup: return F_RESULT;

        // Opcodes that have both type-id and result-id:
        case SpvOpUndef:
        case SpvOpConstantTrue:
        case SpvOpConstantFalse:
        case SpvOpConstant:
        case SpvOpConstantComposite:
        case SpvOpConstantSampler:
        case SpvOpConstantNull:
        case SpvOpSpecConstantTrue:
        case SpvOpSpecConstantFalse:
        case SpvOpSpecConstant:
        case SpvOpSpecConstantComposite:
        case SpvOpSpecConstantOp:
        case SpvOpVariable:
        case SpvOpImageTexelPointer:
        case SpvOpLoad:
        case SpvOpAccessChain:
        case SpvOpInBoundsAccessChain:
        case SpvOpPtrAccessChain:
        case SpvOpArrayLength:
        case SpvOpGenericPtrMemSemantics:
        case SpvOpInBoundsPtrAccessChain:
        case SpvOpFunction:
        case SpvOpFunctionParameter:
        case SpvOpFunctionCall:
        case SpvOpSampledImage:
        case SpvOpImageSampleImplicitLod:
        case SpvOpImageSampleExplicitLod:
        case SpvOpImageSampleDrefImplicitLod:
        case SpvOpImageSampleDrefExplicitLod:
        case SpvOpImageSampleProjImplicitLod:
        case SpvOpImageSampleProjExplicitLod:
        case SpvOpImageSampleProjDrefImplicitLod:
        case SpvOpImageSampleProjDrefExplicitLod:
        case SpvOpImageFetch:
        case SpvOpImageGather:
        case SpvOpImageDrefGather:
        case SpvOpImageRead:
        case SpvOpImage:
        case SpvOpImageQueryFormat:
        case SpvOpImageQueryOrder:
        case SpvOpImageQuerySizeLod:
        case SpvOpImageQuerySize:
        case SpvOpImageQueryLod:
        case SpvOpImageQueryLevels:
        case SpvOpImageQuerySamples:
        case SpvOpConvertFToU:
        case SpvOpConvertFToS:
        case SpvOpConvertSToF:
        case SpvOpConvertUToF:
        case SpvOpUConvert:
        case SpvOpSConvert:
        case SpvOpFConvert:
        case SpvOpQuantizeToF16:
        case SpvOpConvertPtrToU:
        case SpvOpSatConvertSToU:
        case SpvOpSatConvertUToS:
        case SpvOpConvertUToPtr:
        case SpvOpPtrCastToGeneric:
        case SpvOpGenericCastToPtr:
        case SpvOpGenericCastToPtrExplicit:
        case SpvOpBitcast:
        case SpvOpSNegate:
        case SpvOpFNegate:
        case SpvOpIAdd:
        case SpvOpFAdd:
        case SpvOpISub:
        case SpvOpFSub:
        case SpvOpIMul:
        case SpvOpFMul:
        case SpvOpUDiv:
        case SpvOpSDiv:
        case SpvOpFDiv:
        case SpvOpUMod:
        case SpvOpSRem:
        case SpvOpSMod:
        case SpvOpFRem:
        case SpvOpFMod:
        case SpvOpVectorTimesScalar:
        case SpvOpMatrixTimesScalar:
        case SpvOpVectorTimesMatrix:
        case SpvOpMatrixTimesVector:
        case SpvOpMatrixTimesMatrix:
        case SpvOpOuterProduct:
        case SpvOpDot:
        case SpvOpIAddCarry:
        case SpvOpISubBorrow:
        case SpvOpUMulExtended:
        case SpvOpSMulExtended:
        case SpvOpAny:
        case SpvOpAll:
        case SpvOpIsNan:
        case SpvOpIsInf:
        case SpvOpIsFinite:
        case SpvOpIsNormal:
        case SpvOpSignBitSet:
        case SpvOpLessOrGreater:
        case SpvOpOrdered:
        case SpvOpUnordered:
        case SpvOpLogicalEqual:
        case SpvOpLogicalNotEqual:
        case SpvOpLogicalOr:
        case SpvOpLogicalAnd:
        case SpvOpLogicalNot:
        case SpvOpSelect:
        case SpvOpIEqual:
        case SpvOpINotEqual:
        case SpvOpUGreaterThan:
        case SpvOpSGreaterThan:
        case SpvOpUGreaterThanEqual:
        case SpvOpSGreaterThanEqual:
        case SpvOpULessThan:
        case SpvOpSLessThan:
        case SpvOpULessThanEqual:
        case SpvOpSLessThanEqual:
        case SpvOpFOrdEqual:
        case SpvOpFUnordEqual:
        case SpvOpFOrdNotEqual:
        case SpvOpFUnordNotEqual:
        case SpvOpFOrdLessThan:
        case SpvOpFUnordLessThan:
        case SpvOpFOrdGreaterThan:
        case SpvOpFUnordGreaterThan:
        case SpvOpFOrdLessThanEqual:
        case SpvOpFUnordLessThanEqual:
        case SpvOpFOrdGreaterThanEqual:
        case SpvOpFUnordGreaterThanEqual:
        case SpvOpShiftRightLogical:
        case SpvOpShiftRightArithmetic:
        case SpvOpShiftLeftLogical:
        case SpvOpBitwiseOr:
        case SpvOpBitwiseXor:
        case SpvOpBitwiseAnd:
        case SpvOpNot:
        case SpvOpBitFieldInsert:
        case SpvOpBitFieldSExtract:
        case SpvOpBitFieldUExtract:
        case SpvOpBitReverse:
        case SpvOpBitCount:
        case SpvOpDPdx:
        case SpvOpDPdy:
        case SpvOpFwidth:
        case SpvOpDPdxFine:
        case SpvOpDPdyFine:
        case SpvOpFwidthFine:
        case SpvOpDPdxCoarse:
        case SpvOpDPdyCoarse:
        case SpvOpFwidthCoarse:
        case SpvOpPhi:
        case SpvOpVectorExtractDynamic:
        case SpvOpVectorInsertDynamic:
        case SpvOpVectorShuffle:
        case SpvOpCompositeConstruct:
        case SpvOpCompositeExtract:
        case SpvOpCompositeInsert:
        case SpvOpCopyObject:
        case SpvOpTranspose:
        case SpvOpExtInst:
        case SpvOpAtomicLoad:
        case SpvOpAtomicExchange:
        case SpvOpAtomicCompareExchange:
        case SpvOpAtomicCompareExchangeWeak:
        case SpvOpAtomicIIncrement:
        case SpvOpAtomicIDecrement:
        case SpvOpAtomicIAdd:
        case SpvOpAtomicISub:
        case SpvOpAtomicSMin:
        case SpvOpAtomicUMin:
        case SpvOpAtomicSMax:
        case SpvOpAtomicUMax:
        case SpvOpAtomicAnd:
        case SpvOpAtomicOr:
        case SpvOpAtomicXor:
        case SpvOpAtomicFlagTestAndSet: return F_TYPE | F_RESULT;

        default: return F_NONE;
    }
}

uint32_t Instruction::operand(uint32_t idx) const
{
    uint32_t base = 0;
    if (type_id)
        base++;
    if (result_id)
        base++;
    return words[base + idx];
}

uint32_t Instruction::operand_count() const
{
    uint32_t base = 0;
    if (type_id)
        base++;
    if (result_id)
        base++;
    return static_cast<uint32_t>(words.size()) - base;
}

Result<ParsedModule> parse_spirv(std::span<const uint32_t> input)
{
    if (input.size() < 5)
        return Result<ParsedModule>::err("SPIR-V binary too short (need at least 5 words)");

    // Detect endianness from the magic number.
    bool swap = false;
    if (input[0] == kMagic) {
        swap = false;
    } else if (input[0] == kMagicRev) {
        swap = true;
    } else {
        return Result<ParsedModule>::err("SPIR-V magic number not found");
    }

    auto w = [&](size_t i) -> uint32_t { return swap ? swap32(input[i]) : input[i]; };

    ParsedModule mod;
    mod.version = w(1);
    mod.generator = w(2);
    mod.id_bound = w(3);
    // w(4) is the reserved schema word (must be 0, we don't enforce it).

    size_t pos = 5;
    while (pos < input.size()) {
        uint32_t first = w(pos);
        uint32_t word_count = first >> 16;
        SpvOp opcode = static_cast<SpvOp>(first & 0xFFFFu);

        if (word_count == 0)
            return Result<ParsedModule>::err("Instruction with word-count 0 at word "
                                             + std::to_string(pos));
        if (pos + word_count > input.size())
            return Result<ParsedModule>::err("Instruction overruns binary at word "
                                             + std::to_string(pos));

        Instruction inst;
        inst.opcode = opcode;

        // Store all operand words.
        inst.words.reserve(word_count - 1);
        for (uint32_t i = 1; i < word_count; ++i) inst.words.push_back(w(pos + i));

        // Pre-extract type_id and result_id.
        uint8_t flags = opcode_flags(opcode);
        uint32_t off = 0;
        if (flags & F_TYPE) {
            inst.type_id = inst.words.empty() ? 0 : inst.words[off++];
        }
        if (flags & F_RESULT) {
            inst.result_id = (off < inst.words.size()) ? inst.words[off] : 0;
        }

        mod.instructions.push_back(std::move(inst));
        pos += word_count;
    }

    return mod;
}

} // namespace spvdb
