#include "slang-ir-nvvm-legalize.h"

#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-emit-nvvm-type-lowering.h"
#include "slang-ir-dce.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"

namespace Slang
{
namespace
{

static const IRIntegerValue kNVVMI32Max = 2147483647;

enum class NVVMCUDALayoutQueryKind
{
    None,
    Size,
    Alignment,
    Offset,
};

struct NVVMCUDALayoutQuery
{
    NVVMCUDALayoutQueryKind kind = NVVMCUDALayoutQueryKind::None;
    IRType* explicitType = nullptr;
};

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

// Recognizes one exact CUDA-prelude layout-query helper. These helpers describe compile-time
// metadata; their aggregate parameters are not part of the direct backend's runtime value ABI.
bool _getNVVMCUDALayoutQuery(IRFunc* function, NVVMCUDALayoutQuery& outQuery)
{
    outQuery = {};
    if (!function || !isNVVMSignedI32Type(function->getResultType()))
        return false;

    IRBlock* block = function->getFirstBlock();
    if (!block || block->getNextBlock())
        return false;
    auto genericAsm = as<IRGenericAsm>(block->getTerminator());
    if (!genericAsm || genericAsm->getOperandCount() == 0 ||
        !as<IRStringLit>(genericAsm->getOperand(0)))
        return false;
    for (auto inst : block->getOrdinaryInsts())
    {
        if (inst != genericAsm)
            return false;
    }

    const UnownedStringSlice assembly = genericAsm->getAsm();
    if (assembly == toSlice("sizeof($[0])") || assembly == toSlice("alignof($[0])"))
    {
        if (function->getParamCount() != 0 || genericAsm->getOperandCount() != 2)
            return false;
        auto explicitType = as<IRType>(genericAsm->getOperand(1));
        if (!explicitType)
            return false;
        outQuery.kind = assembly == toSlice("sizeof($[0])") ? NVVMCUDALayoutQueryKind::Size
                                                            : NVVMCUDALayoutQueryKind::Alignment;
        outQuery.explicitType = explicitType;
        return true;
    }
    if (assembly == toSlice("sizeof($T0)") || assembly == toSlice("alignof($T0)"))
    {
        if (function->getParamCount() != 1 || genericAsm->getOperandCount() != 1)
            return false;
        outQuery.kind = assembly == toSlice("sizeof($T0)") ? NVVMCUDALayoutQueryKind::Size
                                                           : NVVMCUDALayoutQueryKind::Alignment;
        return true;
    }
    if (assembly == toSlice("int(((char*)&($1)) - ((char*)&($0)))"))
    {
        if (function->getParamCount() != 2 || genericAsm->getOperandCount() != 1)
            return false;
        outQuery.kind = NVVMCUDALayoutQueryKind::Offset;
        return true;
    }
    return false;
}

// Resolves one canonical query call through the shared CUDA layout rules. An offset is owned by
// the exact struct-field key already present in IR, never by positional or structural matching.
bool _getNVVMCUDALayoutQueryValue(
    CodeGenContext* codeGenContext,
    IRCall* call,
    IRFunc* function,
    const NVVMCUDALayoutQuery& query,
    IRIntegerValue& outValue)
{
    outValue = 0;
    if (!call || !function || !isNVVMSignedI32Type(call->getDataType()) ||
        call->getArgCount() != function->getParamCount())
    {
        return false;
    }

    for (UInt argumentIndex = 0; argumentIndex < call->getArgCount(); ++argumentIndex)
    {
        IRInst* argument = call->getArg(argumentIndex);
        if (!argument ||
            !isTypeEqual(argument->getDataType(), function->getParamType(argumentIndex)))
        {
            return false;
        }
    }

    if (query.kind == NVVMCUDALayoutQueryKind::Offset)
    {
        auto aggregateType =
            call->getArgCount() == 2 ? as<IRStructType>(call->getArg(0)->getDataType()) : nullptr;
        auto fieldExtract =
            call->getArgCount() == 2 ? as<IRFieldExtract>(call->getArg(1)) : nullptr;
        if (!aggregateType || !fieldExtract || fieldExtract->getBase() != call->getArg(0))
            return false;

        IRStructField* selectedField = nullptr;
        for (auto field : aggregateType->getFields())
        {
            if (field->getKey() == fieldExtract->getField())
            {
                selectedField = field;
                break;
            }
        }
        if (!selectedField ||
            !isTypeEqual(selectedField->getFieldType(), fieldExtract->getDataType()))
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

    IRType* queriedType = query.explicitType;
    if (!queriedType && call->getArgCount() == 1)
        queriedType = function->getParamType(0);
    if (!queriedType)
        return false;

    IRSizeAndAlignment layout;
    if (SLANG_FAILED(getSizeAndAlignment(
            codeGenContext->getTargetReq(),
            IRTypeLayoutRules::getCUDA(),
            queriedType,
            &layout)))
    {
        return false;
    }

    const IRIntegerValue value =
        query.kind == NVVMCUDALayoutQueryKind::Alignment ? layout.alignment : layout.size;
    if (value <= 0 || value > kNVVMI32Max)
        return false;

    outValue = value;
    return true;
}

struct NVVMFoldedLayoutQuery
{
    IRCall* call = nullptr;
    IRIntegerValue value = 0;
};

// Converts producer-tagged target text into the direct backend's typed terminator. The tag is the
// semantic source of truth; the CUDA spelling is deliberately not copied into NVVM-ready IR.
void _legalizeNVVMSemanticIntrinsics(LinkedIR& linkedIR)
{
    List<IRGenericAsm*> genericAsmInstructions;
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        auto function = as<IRFunc>(globalInst);
        if (!function)
            continue;
        for (auto block : function->getBlocks())
        {
            auto genericAsm = as<IRGenericAsm>(block->getTerminator());
            if (genericAsm && genericAsm->findDecoration<IRNVVMSemanticDecoration>())
                genericAsmInstructions.add(genericAsm);
        }
    }

    IRBuilder builder(linkedIR.module);
    for (auto genericAsm : genericAsmInstructions)
    {
        auto semantic = genericAsm->findDecoration<IRNVVMSemanticDecoration>();
        SLANG_RELEASE_ASSERT(semantic);
        List<IRInst*> operands;
        operands.add(semantic->getSemanticOperand());
        for (UInt i = 1; i < genericAsm->getOperandCount(); ++i)
            operands.add(genericAsm->getOperand(i));

        builder.setInsertBefore(genericAsm);
        builder.emitIntrinsicInst(
            nullptr,
            kIROp_NVVMIntrinsic,
            operands.getCount(),
            operands.getBuffer());
        genericAsm->removeAndDeallocate();
    }
}

SlangResult _foldNVVMCompileTimeLayoutQueries(CodeGenContext* codeGenContext, LinkedIR& linkedIR)
{
    List<NVVMFoldedLayoutQuery> folds;
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
                auto callee = call ? as<IRFunc>(call->getCallee()) : nullptr;
                NVVMCUDALayoutQuery query;
                if (!callee || !_getNVVMCUDALayoutQuery(callee, query))
                    continue;

                IRIntegerValue value = 0;
                if (!_getNVVMCUDALayoutQueryValue(codeGenContext, call, callee, query, value))
                    return _diagnoseNVVMLegalization(codeGenContext, toSlice("CUDA layout query"));
                folds.add({call, value});
            }
        }
    }

    IRBuilder builder(linkedIR.module);
    for (const auto& fold : folds)
    {
        IRInst* constant = builder.getIntValue(fold.call->getDataType(), fold.value);
        fold.call->replaceUsesWith(constant);
        fold.call->removeAndDeallocate();
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
                auto call = as<IRCall>(inst);
                auto callee = call ? as<IRFunc>(call->getCallee()) : nullptr;
                NVVMCUDALayoutQuery query;
                if (callee && _getNVVMCUDALayoutQuery(callee, query))
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
    SLANG_RETURN_ON_FAIL(_foldNVVMCompileTimeLayoutQueries(codeGenContext, linkedIR));
    SLANG_RETURN_ON_FAIL(_removeNVVMCompileTimeOnlyInstructions(codeGenContext, linkedIR));
    _legalizeNVVMSemanticIntrinsics(linkedIR);

    IRDeadCodeEliminationOptions options;
    options.keepLayoutsAlive = true;
    eliminateDeadCode(linkedIR.module, options);
    _removeDeadNVVMAggregateInitializers(linkedIR);
    eliminateDeadCode(linkedIR.module, options);

    return _verifyNVVMReadyIR(codeGenContext, linkedIR);
}

} // namespace Slang
