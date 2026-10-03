#include "slang-ir-nvvm-legalize.h"

#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-emit-nvvm-type-lowering.h"
#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-legalize-global-values.h"
#include "slang-ir-util.h"

namespace Slang
{
namespace
{

static const IRIntegerValue kNVVMI32Max = 2147483647;

// Returns whether the compile request selected the bounds policy implemented by the CUDA and CPU
// preludes. Consider this example:
//
//     slangc shader.slang -target ptx -DSLANG_ENABLE_BOUND_ZERO_INDEX
//
// The definition is consumed by the generated CUDA prelude, but direct NVVM never preprocesses
// that text. Recognize the same request option at the direct target boundary so the policy can be
// represented in IR before preflight. The macro value is deliberately irrelevant, matching the
// prelude's `#ifdef` contract.
bool _isNVVMZeroIndexBoundsEnabled(CodeGenContext* codeGenContext)
{
    for (const auto& define : codeGenContext->getTargetProgram()->getOptionSet().getArray(
             CompilerOptionName::MacroDefine))
    {
        if (define.stringValue == "SLANG_ENABLE_BOUND_ZERO_INDEX")
            return true;
    }
    return false;
}

// Emits `index < count ? index : 0` with an unsigned comparison. Structured-buffer subscripts are
// allowed to retain a signed i32 index in canonical IR, but CUDA converts that value to `size_t`
// before applying `SLANG_BOUND_ZERO_INDEX`. Comparing an unsigned view preserves that behavior for
// negative indices while returning the original index type expected by the access instruction.
IRInst* _emitNVVMZeroBoundedElementIndex(IRBuilder& builder, IRInst* index, IRInst* count)
{
    if (!index || !count || !isNVVMInteger32Type(index->getDataType()) ||
        !isNVVMUnsignedI32Type(count->getDataType()))
    {
        return nullptr;
    }

    IRInst* unsignedIndex = index;
    if (!isNVVMUnsignedI32Type(index->getDataType()))
    {
        unsignedIndex = builder.emitIntrinsicInst(builder.getUIntType(), kIROp_IntCast, 1, &index);
    }
    IRInst* comparisonOperands[] = {unsignedIndex, count};
    IRInst* inBounds = builder.emitIntrinsicInst(
        builder.getBoolType(),
        kIROp_Less,
        SLANG_COUNT_OF(comparisonOperands),
        comparisonOperands);
    IRInst* zero = builder.getIntValue(index->getDataType(), 0);
    IRInst* selectOperands[] = {inBounds, index, zero};
    return builder.emitIntrinsicInst(
        index->getDataType(),
        kIROp_Select,
        SLANG_COUNT_OF(selectOperands),
        selectOperands);
}

// Obtains the runtime element count from either canonical read-only or read-write structured
// resource storage. Keeping this query typed lets preflight select the established physical
// resource recipe later.
IRInst* _emitNVVMStructuredBufferElementCount(IRBuilder& builder, IRInst* buffer)
{
    if (!buffer || !as<IRHLSLStructuredBufferTypeBase>(buffer->getDataType()))
        return nullptr;

    IRType* dimensionsType = builder.getVectorType(builder.getUIntType(), 2);
    IRInst* dimensions =
        builder.emitIntrinsicInst(dimensionsType, kIROp_StructuredBufferGetDimensions, 1, &buffer);
    return builder.emitGetElement(builder.getUIntType(), dimensions, 0);
}

// Reinterprets the canonical byte-address view as uint words and obtains its runtime byte size.
// `ByteAddressBuffer.GetDimensions` has exactly the same producer: it queries this equivalent view
// and multiplies the element count by four. Reusing that representation keeps the extent in typed
// IR rather than exposing the provider's `{data, count}` layout to legalization.
IRInst* _emitNVVMByteAddressBufferSize(IRBuilder& builder, IRInst* buffer)
{
    auto bufferType = buffer ? as<IRByteAddressBufferTypeBase>(buffer->getDataType()) : nullptr;
    if (!bufferType || bufferType->getOperandCount() != 0)
        return nullptr;

    IROp structuredBufferOp = kIROp_Invalid;
    switch (bufferType->getOp())
    {
    case kIROp_HLSLByteAddressBufferType:
        structuredBufferOp = kIROp_HLSLStructuredBufferType;
        break;
    case kIROp_HLSLRWByteAddressBufferType:
        structuredBufferOp = kIROp_HLSLRWStructuredBufferType;
        break;
    default:
        return nullptr;
    }

    IRInst* typeOperands[] = {builder.getUIntType(), builder.getDefaultBufferLayoutType()};
    IRType* equivalentType =
        builder.getType(structuredBufferOp, SLANG_COUNT_OF(typeOperands), typeOperands);
    IRInst* equivalentBuffer =
        builder.emitIntrinsicInst(equivalentType, kIROp_GetEquivalentStructuredBuffer, 1, &buffer);
    IRInst* wordCount = _emitNVVMStructuredBufferElementCount(builder, equivalentBuffer);
    if (!wordCount)
        return nullptr;
    IRInst* four = builder.getIntValue(builder.getUIntType(), 4);
    IRInst* multiplyOperands[] = {wordCount, four};
    return builder.emitIntrinsicInst(
        builder.getUIntType(),
        kIROp_Mul,
        SLANG_COUNT_OF(multiplyOperands),
        multiplyOperands);
}

// Carries the CUDA prelude's selected zero-index bounds policy into ordinary linked IR. Each
// access keeps its canonical opcode; only its index operand is replaced by compare/select
// arithmetic derived from the access's own resource or fixed-array type.
SlangResult _legalizeNVVMZeroIndexBounds(CodeGenContext* codeGenContext, LinkedIR& linkedIR)
{
    if (!_isNVVMZeroIndexBoundsEnabled(codeGenContext))
        return SLANG_OK;

    List<IRInst*> accesses;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                switch (inst->getOp())
                {
                case kIROp_ByteAddressBufferLoad:
                case kIROp_StructuredBufferLoad:
                case kIROp_RWStructuredBufferGetElementPtr:
                case kIROp_GetElement:
                    accesses.add(inst);
                    break;
                default:
                    break;
                }
            }
        }
    }

    IRBuilder builder(linkedIR.module);
    for (auto access : accesses)
    {
        if (access->getOperandCount() < 2)
            continue;
        IRInst* index = access->getOperand(1);
        if (!isNVVMInteger32Type(index->getDataType()))
            continue;

        builder.setInsertBefore(access);
        IRInst* boundedIndex = nullptr;
        switch (access->getOp())
        {
        case kIROp_ByteAddressBufferLoad:
            {
                IRInst* buffer = access->getOperand(0);
                IRType* valueType = access->getDataType();
                IRSizeAndAlignment valueLayout;
                if (!isNVVMUnsignedI32Type(index->getDataType()) ||
                    SLANG_FAILED(getSizeAndAlignment(
                        codeGenContext->getTargetReq(),
                        IRTypeLayoutRules::getCUDA(),
                        valueType,
                        &valueLayout)) ||
                    valueLayout.size <= 0 || valueLayout.size > UINT32_MAX)
                {
                    continue;
                }

                IRInst* sizeInBytes = _emitNVVMByteAddressBufferSize(builder, buffer);
                if (!sizeInBytes)
                    continue;
                IRInst* elementSize =
                    builder.getIntValue(builder.getUIntType(), IRIntegerValue(valueLayout.size));
                IRInst* subtractionOperands[] = {sizeInBytes, elementSize};
                IRInst* lastValidOffset = builder.emitIntrinsicInst(
                    builder.getUIntType(),
                    kIROp_Sub,
                    SLANG_COUNT_OF(subtractionOperands),
                    subtractionOperands);
                IRInst* comparisonOperands[] = {index, lastValidOffset};
                IRInst* inBounds = builder.emitIntrinsicInst(
                    builder.getBoolType(),
                    kIROp_Leq,
                    SLANG_COUNT_OF(comparisonOperands),
                    comparisonOperands);
                IRInst* zero = builder.getIntValue(index->getDataType(), 0);
                IRInst* selectOperands[] = {inBounds, index, zero};
                boundedIndex = builder.emitIntrinsicInst(
                    index->getDataType(),
                    kIROp_Select,
                    SLANG_COUNT_OF(selectOperands),
                    selectOperands);
            }
            break;

        case kIROp_StructuredBufferLoad:
        case kIROp_RWStructuredBufferGetElementPtr:
            {
                IRInst* count =
                    _emitNVVMStructuredBufferElementCount(builder, access->getOperand(0));
                boundedIndex = _emitNVVMZeroBoundedElementIndex(builder, index, count);
            }
            break;

        case kIROp_GetElement:
            {
                auto arrayType = as<IRArrayType>(access->getOperand(0)->getDataType());
                auto count = arrayType ? as<IRIntLit>(arrayType->getElementCount()) : nullptr;
                if (!arrayType || arrayType->getOperandCount() != 2 || !count ||
                    count->getValue() <= 0 || count->getValue() > UINT32_MAX)
                {
                    continue;
                }
                IRInst* unsignedCount =
                    builder.getIntValue(builder.getUIntType(), count->getValue());
                boundedIndex = _emitNVVMZeroBoundedElementIndex(builder, index, unsignedCount);
            }
            break;

        default:
            SLANG_UNEXPECTED("uncollected zero-index bounds access");
        }

        if (boundedIndex)
            access->setOperand(1, boundedIndex);
    }
    return SLANG_OK;
}

SlangResult _diagnoseNVVMLegalization(
    CodeGenContext* codeGenContext,
    const UnownedStringSlice& construct)
{
    codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

// Lowers canonical bitfield operations before the ordinary NVVM preflight and value emitter.
// Consider this example:
//
//     uint whole = bitfieldInsert(base, value, 0, 32);
//     int empty = bitfieldExtract(int(value), 32, 0);
//
// The core intrinsics produce BitfieldInsert/Extract, as does AnyValue integer packing. A
// mask computed as `(1 << count) - 1` shifts by 32 in the first example, and extraction using
// `width - count` does so in the second. LLVM makes those shifts poison. Keep every intermediate
// count below the logical width, then explicitly select the empty result or mask. For the valid
// domain `offset + count <= width`, this also preserves full-width and signed extraction. It
// does not define out-of-range bitfields. Ordinary value lowering owns scalar-count broadcasting
// and narrow physical carriers; this pass only expresses the operation's logical semantics.
SlangResult _legalizeNVVMBitfields(CodeGenContext* codeGenContext, LinkedIR& linkedIR)
{
    IRBuilder builder(linkedIR.module);
    List<IRInst*> workList;
    workList.add(linkedIR.module->getModuleInst());
    while (workList.getCount())
    {
        auto inst = workList.getLast();
        workList.removeLast();
        for (auto child : inst->getChildren())
            workList.add(child);

        if (inst->getOp() != kIROp_BitfieldExtract && inst->getOp() != kIROp_BitfieldInsert)
            continue;
        const bool isInsert = inst->getOp() == kIROp_BitfieldInsert;
        if (inst->getOperandCount() != (isInsert ? 4u : 3u))
            return _diagnoseNVVMLegalization(codeGenContext, toSlice("bitfield signature"));

        auto type = inst->getDataType();
        auto vectorType = asNVVMRegisterVectorType(type);
        auto scalarType = vectorType ? vectorType->getElementType() : type;
        uint32_t width = 0;
        bool isSigned = false;
        auto value = inst->getOperand(0);
        auto offset = inst->getOperand(isInsert ? 2 : 1);
        auto count = inst->getOperand(isInsert ? 3 : 2);
        if (!value || !offset || !count || (isInsert && !inst->getOperand(1)) ||
            !isNVVMSupportedIntegerScalarType(scalarType, &width, &isSigned) ||
            !isTypeEqual(type, value->getDataType()) ||
            (isInsert && !isTypeEqual(type, inst->getOperand(1)->getDataType())) ||
            !isNVVMUnsignedI32Type(offset->getDataType()) ||
            !isNVVMUnsignedI32Type(count->getDataType()))
        {
            return _diagnoseNVVMLegalization(codeGenContext, toSlice("bitfield signature"));
        }

        builder.setInsertBefore(inst);
        auto unsignedType = isSigned ? getUnsignedTypeFromSignedType(&builder, type) : type;
        auto unsignedScalarType =
            isSigned ? getUnsignedTypeFromSignedType(&builder, scalarType) : scalarType;
        auto uintType = builder.getUIntType();
        auto zeroCount = builder.getIntValue(uintType, 0);
        auto countMask = builder.getIntValue(uintType, width - 1);
        auto safeOffset = builder.emitBitAnd(uintType, offset, countMask);
        auto highCount = builder.emitBitAnd(
            uintType,
            builder.emitSub(uintType, builder.getIntValue(uintType, width), count),
            countMask);
        IRInst* isEmpty = builder.emitEql(count, zeroCount);
        if (vectorType)
        {
            isEmpty = builder.emitMakeVectorFromScalar(
                builder.getVectorType(builder.getBoolType(), vectorType->getElementCount()),
                isEmpty);
        }
        IRInst* zero = builder.getIntValue(unsignedScalarType, 0);
        if (vectorType)
            zero = builder.emitMakeVectorFromScalar(unsignedType, zero);
        auto unsignedValue = isSigned ? builder.emitBitCast(unsignedType, value) : value;
        IRInst* result = nullptr;
        if (isInsert)
        {
            auto inserted = inst->getOperand(1);
            if (isSigned)
                inserted = builder.emitBitCast(unsignedType, inserted);
            auto lowMask =
                builder.emitShr(unsignedType, builder.emitBitNot(unsignedType, zero), highCount);
            IRInst* maskOperands[] = {isEmpty, zero, lowMask};
            auto mask = builder.emitShl(
                unsignedType,
                builder.emitIntrinsicInst(unsignedType, kIROp_Select, 3, maskOperands),
                safeOffset);
            result = builder.emitBitOr(
                unsignedType,
                builder.emitBitAnd(
                    unsignedType,
                    unsignedValue,
                    builder.emitBitNot(unsignedType, mask)),
                builder.emitBitAnd(
                    unsignedType,
                    builder.emitShl(unsignedType, inserted, safeOffset),
                    mask));
            if (isSigned)
                result = builder.emitBitCast(type, result);
        }
        else
        {
            auto highBits = builder.emitShl(
                unsignedType,
                builder.emitShr(unsignedType, unsignedValue, safeOffset),
                highCount);
            if (isSigned)
            {
                highBits = builder.emitBitCast(type, highBits);
                zero = builder.emitBitCast(type, zero);
            }
            auto extracted = builder.emitShr(type, highBits, highCount);
            IRInst* resultOperands[] = {isEmpty, zero, extracted};
            result = builder.emitIntrinsicInst(type, kIROp_Select, 3, resultOperands);
        }
        inst->replaceUsesWith(result);
        inst->removeAndDeallocate();
    }
    return SLANG_OK;
}

// Resolves one source query through the shared CUDA layout rules. An offset is owned by
// the exact struct-field key already present in IR, never by positional or structural matching.
bool _getNVVMOffsetQueryValue(
    CodeGenContext* codeGenContext,
    IROffsetOf* query,
    IRIntegerValue& outValue)
{
    outValue = 0;
    auto aggregateType = as<IRStructType>(query->getBase()->getDataType());
    if (!isNVVMSignedI32Type(query->getDataType()) || !aggregateType ||
        !as<IRStructKey>(query->getFieldKey()))
        return false;

    IRStructField* selectedField = nullptr;
    for (auto field : aggregateType->getFields())
    {
        if (field->getKey() == query->getFieldKey())
        {
            selectedField = field;
            break;
        }
    }
    if (!selectedField ||
        !isTypeEqual(selectedField->getFieldType(), query->getFieldValue()->getDataType()))
    {
        return false;
    }

    IRIntegerValue offset = 0;
    if (SLANG_FAILED(getOffset(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            selectedField,
            &offset)) ||
        offset < 0 || offset > kNVVMI32Max)
    {
        return false;
    }
    outValue = offset;
    return true;
}

struct NVVMFoldedOffsetQuery
{
    IROffsetOf* query = nullptr;
    IRIntegerValue value = 0;
};

SlangResult _foldNVVMCompileTimeOffsetQueries(CodeGenContext* codeGenContext, LinkedIR& linkedIR)
{
    List<NVVMFoldedOffsetQuery> folds;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                auto query = as<IROffsetOf>(inst);
                if (!query)
                    continue;

                IRIntegerValue value = 0;
                if (!_getNVVMOffsetQueryValue(codeGenContext, query, value))
                    return _diagnoseNVVMLegalization(codeGenContext, toSlice("CUDA layout query"));
                folds.add({query, value});
            }
        }
    }

    IRBuilder builder(linkedIR.module);
    for (const auto& fold : folds)
    {
        IRInst* constant = builder.getIntValue(fold.query->getDataType(), fold.value);
        fold.query->replaceUsesWith(constant);
        fold.query->removeAndDeallocate();
    }
    return SLANG_OK;
}

SlangResult _removeNVVMCompileTimeOnlyInstructions(
    CodeGenContext* codeGenContext,
    LinkedIR& linkedIR)
{
    List<IRInst*> instructionsToRemove;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                switch (inst->getOp())
                {
                case kIROp_RequireComputeDerivative:
                    // The common CUDA pipeline has already admitted compute derivatives. Unlike
                    // GLSL, CUDA requires no entry-point execution-mode decoration.
                    instructionsToRemove.add(inst);
                    break;
                case kIROp_Unmodified:
                    // `unused(inout T)` and `unmodified(out T)` are read-none source checks. They
                    // return void and cannot define an executable value at this handoff.
                    if (inst->getOperandCount() != 1 || inst->hasUses())
                        return _diagnoseNVVMLegalization(codeGenContext, toSlice("unmodified"));
                    instructionsToRemove.add(inst);
                    break;
                default:
                    break;
                }
            }
        }
    }
    for (auto inst : instructionsToRemove)
        inst->removeAndDeallocate();
    return SLANG_OK;
}

void _removeDeadNVVMAggregateInitializers(LinkedIR& linkedIR)
{
    List<IRCall*> deadAggregateCalls;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                auto call = as<IRCall>(inst);
                if (!call || call->hasUses() || !as<IRStructType>(call->getDataType()))
                    continue;

                auto callee = getResolvedInstForDecorations(call->getCallee());
                auto constructor =
                    callee ? callee->findDecoration<IRConstructorDecoration>() : nullptr;
                if (!callee || !callee->findDecoration<IRReadNoneDecoration>() || !constructor ||
                    !constructor->getSynthesizedStatus())
                    continue;

                bool hasOnlySideEffectFreeArguments = true;
                for (UInt i = 0; i < call->getArgCount(); ++i)
                {
                    auto argument = call->getArg(i);
                    if (isValueType(argument->getDataType()))
                        continue;
                    auto pointerLiteral = as<IRPtrLit>(argument);
                    if (!pointerLiteral || pointerLiteral->getValue())
                    {
                        hasOnlySideEffectFreeArguments = false;
                        break;
                    }
                }
                if (hasOnlySideEffectFreeArguments)
                    deadAggregateCalls.add(call);
            }
        }
    }
    for (auto call : deadAggregateCalls)
        call->removeAndDeallocate();
}

// Legalizes nonescaping Boolean lane accesses to private local vectors. Consider this example:
//
//     bool3 flags = bool3(false, true, false);
//     flags[index] = value;
//
// IRBuilder creates a GetElementPtr for the subscript, and buffer-element lowering attaches its
// scalar layout. That is valid semantic IR, but LLVM packs the local value into <3 x i1>; a scalar
// i1 GEP advances by bytes and cannot address those packed bits. Keep the canonical vector value:
// extract on reads, and select the replacement lane while preserving its neighbors on writes.
// Only direct local Vars and nonescaping load/store users belong here. Shared/external storage
// needs its own physical representation and must still be checked by ordinary NVVM preflight.
void _legalizeNVVMLocalBooleanVectorAddresses(LinkedIR& linkedIR)
{
    List<IRGetElementPtr*> addresses;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
            for (auto inst : block->getOrdinaryInsts())
                if (auto address = as<IRGetElementPtr>(inst))
                    addresses.add(address);
    }

    IRBuilder builder(linkedIR.module);
    for (auto address : addresses)
    {
        auto base = address->getBase();
        IRType* valueType = nullptr;
        auto baseType =
            asNVVMSupportedLocalCopyableValuePointerType(base->getDataType(), &valueType);
        uint32_t laneCount = 0;
        auto vectorType = asNVVMSupportedValueVectorType(valueType, &laneCount);
        auto resultType = asNVVMSupportedDerivedCopyableValuePointerType(address->getDataType());
        if (base->getOp() != kIROp_Var || !baseType || !vectorType ||
            !isNVVMBoolType(vectorType->getElementType()) || !resultType ||
            resultType->getAccessQualifier() != AccessQualifier::ReadWrite ||
            !isTypeEqual(resultType->getValueType(), vectorType->getElementType()) ||
            !isNVVMInteger32Type(address->getIndex()->getDataType()))
        {
            continue;
        }

        bool hasOnlyMemoryUses = true;
        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if ((user->getOp() != kIROp_Load && user->getOp() != kIROp_Store) ||
                use != &user->getOperands()[0])
            {
                hasOnlyMemoryUses = false;
                break;
            }
        }
        if (!hasOnlyMemoryUses)
            continue;

        while (auto use = address->firstUse)
        {
            auto user = use->getUser();
            builder.setInsertBefore(user);
            IRBuilderSourceLocRAII sourceLocationScope(&builder, user->sourceLoc);
            auto oldValue = builder.emitLoad(base);
            if (user->getOp() == kIROp_Load)
            {
                user->replaceUsesWith(builder.emitElementExtract(oldValue, address->getIndex()));
            }
            else
            {
                IRInst* lanes[4] = {};
                for (uint32_t i = 0; i < laneCount; ++i)
                {
                    auto laneIndex = builder.getIntValue(address->getIndex()->getDataType(), i);
                    auto isSelected = builder.emitEql(address->getIndex(), laneIndex);
                    IRInst* operands[] = {
                        isSelected,
                        cast<IRStore>(user)->getVal(),
                        builder.emitElementExtract(oldValue, laneIndex)};
                    lanes[i] = builder.emitIntrinsicInst(
                        vectorType->getElementType(),
                        kIROp_Select,
                        3,
                        operands);
                }
                builder.emitStore(base, builder.emitMakeVector(vectorType, laneCount, lanes));
            }
            user->removeAndDeallocate();
        }
        address->removeAndDeallocate();
    }
}

// Expresses texture descriptor word transport through the existing UInt64 representation.
// Consider `uint2 words = uint2(textureHandle)`: the checked cast retains the actual resource
// type, so this boundary can select read-only texture handles without reconstructing provenance.
// Buffers and samplers have different contracts and remain for ordinary preflight rejection.
void _legalizeNVVMTextureDescriptorWordConversions(LinkedIR& linkedIR)
{
    List<IRInst*> conversions;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                const bool toHandle = inst->getOp() == kIROp_CastUInt2ToDescriptorHandle;
                if ((!toHandle && inst->getOp() != kIROp_CastDescriptorHandleToUInt2) ||
                    inst->getOperandCount() != 1)
                    continue;
                auto value = inst->getOperand(0);
                auto handleType = as<IRDescriptorHandleType>(
                    toHandle ? inst->getDataType() : value->getDataType());
                auto wordsType =
                    as<IRVectorType>(toHandle ? value->getDataType() : inst->getDataType());
                auto count = wordsType ? as<IRIntLit>(wordsType->getElementCount()) : nullptr;
                NVVMReadOnlyTextureType texture;
                if (handleType && wordsType && count && count->getValue() == 2 &&
                    isNVVMUnsignedI32Type(wordsType->getElementType()) &&
                    getNVVMSupportedReadOnlyTextureType(handleType->getResourceType(), texture))
                    conversions.add(inst);
            }
        }
    }

    IRBuilder builder(linkedIR.module);
    auto uintType = builder.getUIntType();
    auto uint64Type = builder.getUInt64Type();
    auto shift = builder.getIntValue(uint64Type, 32);
    for (auto inst : conversions)
    {
        builder.setInsertBefore(inst);
        IRBuilderSourceLocRAII sourceLocationScope(&builder, inst->sourceLoc);
        auto value = inst->getOperand(0);
        IRInst* result = nullptr;
        if (inst->getOp() == kIROp_CastUInt2ToDescriptorHandle)
        {
            auto low =
                builder.emitCast(uint64Type, builder.emitElementExtract(value, IRIntegerValue(0)));
            auto high =
                builder.emitCast(uint64Type, builder.emitElementExtract(value, IRIntegerValue(1)));
            auto bits =
                builder.emitBitOr(uint64Type, low, builder.emitShl(uint64Type, high, shift));
            result = builder.emitIntrinsicInst(
                inst->getDataType(),
                kIROp_CastUInt64ToDescriptorHandle,
                1,
                &bits);
        }
        else
        {
            auto bits = builder.emitIntrinsicInst(
                uint64Type,
                kIROp_CastDescriptorHandleToUInt64,
                1,
                &value);
            IRInst* words[] = {
                builder.emitCast(uintType, bits),
                builder.emitCast(uintType, builder.emitShr(uint64Type, bits, shift))};
            result = builder.emitMakeVector(inst->getDataType(), 2, words);
        }
        inst->replaceUsesWith(result);
        inst->removeAndDeallocate();
    }
}

// Reinterpret lowering preserves a genuine pointer-to-UInt64 BitCast. For example,
// `reinterpret<uint64_t>(layoutPointer)` observes the same address as `uint64_t(layoutPointer)`.
// Canonicalize that exact address-only shape so both spellings use the existing checked-root
// plan. This grants no provenance and leaves inverse, UInt2 and ordinary pointer bitcasts alone.
void _legalizeNVVMLayoutPointerObservations(LinkedIR& linkedIR)
{
    List<IRInst*> observations;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
            for (auto inst : block->getOrdinaryInsts())
                if (inst->getOp() == kIROp_BitCast && inst->getOperandCount() == 1 &&
                    inst->getDataType()->getOp() == kIROp_UInt64Type &&
                    asNVVMSupportedLayoutTransportPointerType(inst->getOperand(0)->getDataType()))
                    observations.add(inst);
    }
    IRBuilder builder(linkedIR.module);
    for (auto inst : observations)
    {
        builder.setInsertBefore(inst);
        IRBuilderSourceLocRAII sourceLoc(&builder, inst->sourceLoc);
        auto value = inst->getOperand(0);
        auto result = builder.emitIntrinsicInst(inst->getDataType(), kIROp_CastPtrToInt, 1, &value);
        inst->transferDecorationsTo(result);
        inst->replaceUsesWith(result);
        inst->removeAndDeallocate();
    }
}

// Localizes executable constant expressions without changing which operations NVVM supports.
// Consider `static const float3 sum = a + b;`: replacing the global constant exposes a
// module-owned Add. The shared inliner clones that expression before each function use,
// including enclosing constructors and read-only vector projections, so ordinary operation
// and dominance checks can own it.
// Calls and loads must remain module-owned even beneath such constructors; cloning either
// could duplicate an effect or change when memory is observed.
struct NVVMGlobalInstInliningContext : GlobalInstInliningContextGeneric
{
    bool isLegalGlobalInstForTarget(IRInst* inst) override
    {
        switch (inst->getOp())
        {
        case kIROp_MakeVector:
        case kIROp_MakeVectorFromScalar:
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
            return true;
        default:
            return false;
        }
    }

    bool isInlinableGlobalInst(IRInst* inst) override
    {
        if (isLegalGlobalInstForTarget(inst))
            return true;
        switch (inst->getOp())
        {
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_Fma:
        case kIROp_FRem:
        case kIROp_IRem:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_BitNot:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_Not:
        case kIROp_Neg:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Less:
        case kIROp_Leq:
        case kIROp_Greater:
        case kIROp_Geq:
        case kIROp_Select:
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
        case kIROp_BitCast:
        case kIROp_Swizzle:
            break;
        default:
            return false;
        }
        // This is a placement policy, not a second numeric type/operation catalog. In
        // particular, a pointer-to-bits cast is not numeric placement; checked provenance
        // remains with its original producer. Exact widths and signatures are preflight's job.
        auto type = inst->getDataType();
        if (!as<IRBasicType>(type) && !as<IRVectorType>(type))
            return false;
        for (UInt i = 0; i < inst->getOperandCount(); ++i)
        {
            auto operandType = inst->getOperand(i)->getDataType();
            if (!as<IRBasicType>(operandType) && !as<IRVectorType>(operandType))
                return false;
        }
        return true;
    }

    bool isInlinableGlobalInstForTarget(IRInst*) override { return false; }
    bool shouldBeInlinedForTarget(IRInst*) override { return false; }
    IRInst* getOutsideASM(IRInst* inst) override { return inst; }
};

SlangResult _verifyNVVMReadyIR(CodeGenContext* codeGenContext, const LinkedIR& linkedIR)
{
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            for (auto inst : block->getOrdinaryInsts())
            {
                if (inst->getOp() == kIROp_RequireComputeDerivative ||
                    inst->getOp() == kIROp_Unmodified)
                {
                    return _diagnoseNVVMLegalization(
                        codeGenContext,
                        inst->getOp() == kIROp_RequireComputeDerivative
                            ? toSlice("RequireComputeDerivative")
                            : toSlice("unmodified"));
                }
                if (as<IROffsetOf>(inst))
                    return _diagnoseNVVMLegalization(codeGenContext, toSlice("CUDA layout query"));
            }
        }
    }
    return SLANG_OK;
}

} // namespace

SlangResult legalizeIRForNVVM(CodeGenContext* codeGenContext, LinkedIR& linkedIR)
{
    if (!linkedIR.module)
        return _diagnoseNVVMLegalization(codeGenContext, toSlice("CUDA layout query module"));

    SLANG_RETURN_ON_FAIL(_legalizeNVVMZeroIndexBounds(codeGenContext, linkedIR));
    SLANG_RETURN_ON_FAIL(_foldNVVMCompileTimeOffsetQueries(codeGenContext, linkedIR));
    SLANG_RETURN_ON_FAIL(_removeNVVMCompileTimeOnlyInstructions(codeGenContext, linkedIR));
    _legalizeNVVMLocalBooleanVectorAddresses(linkedIR);
    _legalizeNVVMTextureDescriptorWordConversions(linkedIR);
    _legalizeNVVMLayoutPointerObservations(linkedIR);
    SLANG_RETURN_ON_FAIL(_legalizeNVVMBitfields(codeGenContext, linkedIR));

    // No simplifying/hoisting pass follows this target handoff. Ordinary clones stay in
    // their consuming blocks without introducing GlobalValueRef wrappers.
    NVVMGlobalInstInliningContext globalValues;
    globalValues.wrapReferences = false;
    globalValues.inlineGlobalValuesAndRemoveIfUnused(linkedIR.module);

    IRDeadCodeEliminationOptions options;
    options.keepLayoutsAlive = true;
    eliminateDeadCode(linkedIR.module, options);
    _removeDeadNVVMAggregateInitializers(linkedIR);
    eliminateDeadCode(linkedIR.module, options);

    return _verifyNVVMReadyIR(codeGenContext, linkedIR);
}

} // namespace Slang
