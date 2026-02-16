// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

#include <spirv/unified1/spirv.h>

// Define operator< for public cooperative type structs, used internally.
namespace slang
{
bool operator<(const CooperativeMatrixType& a, const CooperativeMatrixType& b)
{
    if (a.componentType != b.componentType)
        return a.componentType < b.componentType;
    if (a.scope != b.scope)
        return a.scope < b.scope;
    if (a.rowCount != b.rowCount)
        return a.rowCount < b.rowCount;
    if (a.columnCount != b.columnCount)
        return a.columnCount < b.columnCount;
    return a.use < b.use;
}

bool operator<(const CooperativeMatrixCombination& a, const CooperativeMatrixCombination& b)
{
    if (a.m != b.m)
        return a.m < b.m;
    if (a.n != b.n)
        return a.n < b.n;
    if (a.k != b.k)
        return a.k < b.k;
    if (a.componentTypeA != b.componentTypeA)
        return a.componentTypeA < b.componentTypeA;
    if (a.componentTypeB != b.componentTypeB)
        return a.componentTypeB < b.componentTypeB;
    if (a.componentTypeC != b.componentTypeC)
        return a.componentTypeC < b.componentTypeC;
    if (a.componentTypeResult != b.componentTypeResult)
        return a.componentTypeResult < b.componentTypeResult;
    if (a.saturate != b.saturate)
        return a.saturate < b.saturate;
    return a.scope < b.scope;
}

bool operator<(const CooperativeVectorCombination& a, const CooperativeVectorCombination& b)
{
    if (a.inputType != b.inputType)
        return a.inputType < b.inputType;
    if (a.inputInterpretation != b.inputInterpretation)
        return a.inputInterpretation < b.inputInterpretation;
    if (a.matrixInterpretation != b.matrixInterpretation)
        return a.matrixInterpretation < b.matrixInterpretation;
    if (a.biasInterpretation != b.biasInterpretation)
        return a.biasInterpretation < b.biasInterpretation;
    if (a.resultType != b.resultType)
        return a.resultType < b.resultType;
    return a.transpose < b.transpose;
}
} // namespace slang

namespace Slang
{

// This file currently implements a pass that collects information about the shader parameters that
// are referenced in the IR. It's named 'metadata' in order to support other potential code
// analysis scenarios in the future.


// Inserts a single resource binding (which takes `count` slots, where 0 means unbounded) into the
// list of resource ranges.
static void _insertBinding(
    List<ShaderBindingRange>& ranges,
    LayoutResourceKind kind,
    UInt spaceIndex,
    UInt registerIndex,
    UInt count)
{
    // Construct a new range from the provided resource.
    ShaderBindingRange newRange;
    newRange.category = kind;
    newRange.spaceIndex = spaceIndex;
    newRange.registerIndex = registerIndex;
    newRange.registerCount = count;

    // See if the new range is adjacent to any of the existing ranges, merge with that.
    for (auto& range : ranges)
    {
        if (range.adjacentTo(newRange))
        {
            range.mergeWith(newRange);
            return;
        }
    }

    // No adjacent ranges found - create a new one.
    ranges.add(newRange);
}

static SlangCooperativeComponentType _getComponentTypeFromIRType(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_HalfType:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16;
    case kIROp_FloatType:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32;
    case kIROp_DoubleType:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT64;
    case kIROp_Int8Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT8;
    case kIROp_Int16Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT16;
    case kIROp_IntType:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT32;
    case kIROp_Int64Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT64;
    case kIROp_UInt8Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT8;
    case kIROp_UInt16Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT16;
    case kIROp_UIntType:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT32;
    case kIROp_UInt64Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT64;
    case kIROp_BFloat16Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_BFLOAT16;
    case kIROp_FloatE4M3Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT_E4M3;
    case kIROp_FloatE5M2Type:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT_E5M2;
    default:
        break;
    }
    return SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
}

static SlangCooperativeComponentType _getComponentTypeFromSpvComponentType(
    IRInst* interpretationInst)
{
    auto lit = as<IRIntLit>(interpretationInst);
    if (!lit)
        return SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;

    switch (UInt(lit->getValue()))
    {
    case SpvComponentTypeFloat16NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16;
    case SpvComponentTypeFloat32NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32;
    case SpvComponentTypeFloat64NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT64;
    case SpvComponentTypeSignedInt8NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT8;
    case SpvComponentTypeSignedInt16NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT16;
    case SpvComponentTypeSignedInt32NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT32;
    case SpvComponentTypeSignedInt64NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT64;
    case SpvComponentTypeUnsignedInt8NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT8;
    case SpvComponentTypeUnsignedInt16NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT16;
    case SpvComponentTypeUnsignedInt32NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT32;
    case SpvComponentTypeUnsignedInt64NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT64;
    case SpvComponentTypeSignedInt8PackedNV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_INT8_PACKED;
    case SpvComponentTypeUnsignedInt8PackedNV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_UINT8_PACKED;
    case SpvComponentTypeFloatE4M3NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT_E4M3;
    case SpvComponentTypeFloatE5M2NV:
        return SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT_E5M2;
    default:
        break;
    }
    return SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
}

static SlangScope _getScopeFromIRValue(IRInst* scopeInst)
{
    switch ((MemoryScope)getIntVal(scopeInst))
    {
    case MemoryScope::Workgroup:
        return SLANG_SCOPE_THREAD_GROUP;
    case MemoryScope::Subgroup:
        return SLANG_SCOPE_WAVE;
    case MemoryScope::Invocation:
        return SLANG_SCOPE_THREAD;
    default:
        break;
    }
    return SLANG_SCOPE_NONE;
}

static SlangCooperativeMatrixUse _getMatrixUseFromIRValue(IRInst* matrixUseInst)
{
    switch (UInt(getIntVal(matrixUseInst)))
    {
    case SpvCooperativeMatrixUseMatrixAKHR:
        return SLANG_COOPERATIVE_MATRIX_USE_A;
    case SpvCooperativeMatrixUseMatrixBKHR:
        return SLANG_COOPERATIVE_MATRIX_USE_B;
    case SpvCooperativeMatrixUseMatrixAccumulatorKHR:
        return SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR;
    default:
        break;
    }

    return SLANG_COOPERATIVE_MATRIX_USE_NONE;
}

static slang::CooperativeMatrixType _getCooperativeMatrixType(IRInst* inst)
{
    slang::CooperativeMatrixType type = {};

    auto coopMatType = as<IRCoopMatrixType>(getResolvedInstForDecorations(inst));
    if (!coopMatType)
        return type;

    type.componentType = _getComponentTypeFromIRType(coopMatType->getElementType());
    if (type.componentType == SLANG_COOPERATIVE_COMPONENT_TYPE_NONE)
        return slang::CooperativeMatrixType{};

    type.scope = _getScopeFromIRValue(coopMatType->getScope());
    type.rowCount = getIntVal(coopMatType->getRowCount());
    type.columnCount = getIntVal(coopMatType->getColumnCount());
    type.use = _getMatrixUseFromIRValue(coopMatType->getMatrixUse());
    return type;
}

static SlangCooperativeComponentType _getCooperativeVectorComponentType(IRInst* inst)
{
    auto coopVecType = as<IRCoopVectorType>(getResolvedInstForDecorations(inst));
    if (!coopVecType)
        return SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;

    return _getComponentTypeFromIRType(coopVecType->getElementType());
}

template<typename T>
static void _insertSortedUnique(List<T>& list, const T& value)
{
    Index searchResult = list.binarySearch(value);
    if (searchResult < 0)
        list.insert(~searchResult, value);
}

static IRIntLit* _tryGetSPIRVOpcodeValue(IRSPIRVAsmInst* spirvAsmInst)
{
    auto opcodeOperand = spirvAsmInst->getOpcodeOperand();
    if (opcodeOperand->getOp() == kIROp_SPIRVAsmOperandTruncate)
        return nullptr;

    if (opcodeOperand->getOperandCount() < 1)
        return nullptr;

    return as<IRIntLit>(opcodeOperand->getValue());
}

static void collectMetadataFromCooperativeVectorCombination(
    IRSPIRVAsmInst* spirvAsmInst,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto opcodeValue = _tryGetSPIRVOpcodeValue(spirvAsmInst);
    if (!opcodeValue)
        return;

    // MulNV is the same as MulAddNV except for the Bias-related operands.
    int biasCount;
    switch (opcodeValue->getValue())
    {
    case SpvOpCooperativeVectorMatrixMulAddNV:
        biasCount = 3;
        break;
    case SpvOpCooperativeVectorMatrixMulNV:
        biasCount = 0;
        break;
    default:
        return;
    }

    auto spirvOperands = spirvAsmInst->getSPIRVOperands();
    auto operandCount = spirvOperands.getCount();
    if (operandCount < 11 + biasCount)
        return;

    if (spirvOperands[1]->getOp() != kIROp_SPIRVAsmOperandResult)
        return;

    auto input = spirvOperands[2]->getValue();
    auto inputInterpretation = spirvOperands[3]->getValue();
    auto matrixInterpretation = spirvOperands[6]->getValue();
    auto biasInterpretation = biasCount ? spirvOperands[9]->getValue() : nullptr;
    auto transpose = spirvOperands[10 + biasCount]->getValue();
    if (!input || !inputInterpretation || !matrixInterpretation || !transpose ||
        (biasCount && !biasInterpretation))
        return;

    auto resultType = as<IRType>(spirvOperands[0]->getValue());
    if (!resultType)
        return;

    slang::CooperativeVectorCombination combination = {};
    combination.inputType = _getCooperativeVectorComponentType(input->getDataType());
    combination.inputInterpretation = _getComponentTypeFromSpvComponentType(inputInterpretation);
    combination.matrixInterpretation = _getComponentTypeFromSpvComponentType(matrixInterpretation);
    combination.biasInterpretation = biasInterpretation
                                         ? _getComponentTypeFromSpvComponentType(biasInterpretation)
                                         : SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
    combination.resultType = _getCooperativeVectorComponentType(resultType);

    auto transposeLit = as<IRBoolLit>(transpose);
    if (!transposeLit)
        return;
    combination.transpose = transposeLit->getValue();

    if (!combination.inputType || !combination.inputInterpretation ||
        !combination.matrixInterpretation || !combination.resultType ||
        (biasCount && !combination.biasInterpretation))
    {
        return;
    }

    _insertSortedUnique(outMetadata.m_cooperativeVectorCombinations, combination);
}

static void collectMetadataFromCooperativeVectorTrainingType(
    IRSPIRVAsmInst* spirvAsmInst,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto opcodeValue = _tryGetSPIRVOpcodeValue(spirvAsmInst);
    if (!opcodeValue)
        return;

    auto spirvOperands = spirvAsmInst->getSPIRVOperands();
    SlangCooperativeComponentType trainingType = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;

    switch (opcodeValue->getValue())
    {
    case SpvOpCooperativeVectorOuterProductAccumulateNV:
        if (spirvOperands.getCount() < 6)
            return;
        trainingType = _getComponentTypeFromSpvComponentType(spirvOperands[5]->getValue());
        break;

    case SpvOpCooperativeVectorReduceSumAccumulateNV:
        if (spirvOperands.getCount() < 3)
            return;
        {
            auto vectorValue = spirvOperands[2]->getValue();
            if (!vectorValue)
                return;
            trainingType = _getCooperativeVectorComponentType(vectorValue->getDataType());
        }
        break;

    default:
        return;
    }

    if (trainingType == SLANG_COOPERATIVE_COMPONENT_TYPE_NONE)
        return;

    _insertSortedUnique(outMetadata.m_cooperativeVectorTrainingTypes, trainingType);
}

static void collectMetadataFromCooperativeMatrixCombination(
    IRSPIRVAsmInst* spirvAsmInst,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto opcodeValue = _tryGetSPIRVOpcodeValue(spirvAsmInst);
    if (!opcodeValue)
        return;

    if (opcodeValue->getValue() != SpvOpCooperativeMatrixMulAddKHR)
        return;

    auto spirvOperands = spirvAsmInst->getSPIRVOperands();
    auto operandCount = spirvOperands.getCount();
    if (operandCount < 5)
        return;

    if (spirvOperands[1]->getOp() != kIROp_SPIRVAsmOperandResult)
        return;

    auto argA = spirvOperands[2]->getValue();
    auto argB = spirvOperands[3]->getValue();
    auto argC = spirvOperands[4]->getValue();
    auto matrixOperands = operandCount > 5 ? as<IRIntLit>(spirvOperands[5]->getValue()) : nullptr;
    if (!argA || !argB || !argC || (operandCount > 5 && !matrixOperands))
        return;

    bool saturate =
        matrixOperands && ((UInt(matrixOperands->getValue()) &
                            UInt(SpvCooperativeMatrixOperandsSaturatingAccumulationKHRMask)) != 0);

    slang::CooperativeMatrixType typeA = _getCooperativeMatrixType(argA->getDataType());
    slang::CooperativeMatrixType typeB = _getCooperativeMatrixType(argB->getDataType());
    slang::CooperativeMatrixType typeC = _getCooperativeMatrixType(argC->getDataType());

    auto resultTypeInst = as<IRType>(spirvOperands[0]->getValue());
    if (!resultTypeInst)
        return;
    slang::CooperativeMatrixType typeResult = _getCooperativeMatrixType(resultTypeInst);

    slang::CooperativeMatrixCombination combination = {};

    // A: MxK, B: KxN
    combination.m = typeA.rowCount;
    combination.n = typeB.columnCount;
    combination.k = typeA.columnCount;

    combination.componentTypeA = typeA.componentType;
    combination.componentTypeB = typeB.componentType;
    combination.componentTypeC = typeC.componentType;
    combination.componentTypeResult = typeResult.componentType;

    if (!combination.componentTypeA || !combination.componentTypeB || !combination.componentTypeC ||
        !combination.componentTypeResult)
    {
        return;
    }

    combination.saturate = saturate;
    combination.scope = typeA.scope;

    _insertSortedUnique(outMetadata.m_cooperativeMatrixCombinations, combination);
}

static void collectCooperativeTypeFromInst(IRInst* inst, ArtifactPostEmitMetadata& outMetadata)
{
    auto resolved = getResolvedInstForDecorations(inst);

    if (as<IRCoopMatrixType>(resolved))
    {
        auto matrixType = _getCooperativeMatrixType(resolved);
        if (matrixType.componentType)
            _insertSortedUnique(outMetadata.m_cooperativeMatrixTypes, matrixType);
    }
    else if (as<IRCoopVectorType>(resolved))
    {
        auto vectorType = _getCooperativeVectorComponentType(resolved);
        if (vectorType)
            _insertSortedUnique(outMetadata.m_cooperativeVectorTypes, vectorType);
    }
    else if (auto spirvAsmInst = as<IRSPIRVAsmInst>(inst))
    {
        collectMetadataFromCooperativeMatrixCombination(spirvAsmInst, outMetadata);
        collectMetadataFromCooperativeVectorCombination(spirvAsmInst, outMetadata);
        collectMetadataFromCooperativeVectorTrainingType(spirvAsmInst, outMetadata);
    }
}

void collectMetadataFromInst(IRInst* param, ArtifactPostEmitMetadata& outMetadata)
{
    auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
    if (!layoutDecoration)
        return;

    auto varLayout = as<IRVarLayout>(layoutDecoration->getLayout());
    if (!varLayout)
        return;

    UInt spaceOffset = 0;
    if (auto spaceAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
    {
        spaceOffset = spaceAttr->getOffset();
    }
    for (auto sizeAttr : varLayout->getTypeLayout()->getSizeAttrs())
    {
        auto kind = sizeAttr->getResourceKind();

        // Only track resource types that we can reliably track, such as textures.
        // Do not track individual uniforms, for example.
        if (!ShaderBindingRange::isUsageTracked(kind))
            continue;

        if (auto offsetAttr = varLayout->findOffsetAttr(kind))
        {
            // Get the binding information from this attribute and insert it into the list
            auto spaceIndex = spaceOffset + offsetAttr->getSpace();
            auto registerIndex = offsetAttr->getOffset();
            auto size = sizeAttr->getSize();
            auto count = size.getFiniteValueOr(0);
            _insertBinding(outMetadata.m_usedBindings, kind, spaceIndex, registerIndex, count);
        }
    }

    // If the global parameter is a parameter block, make sure to collect bindings for its
    // default constant buffer, if there is one.
    // The default constant buffer binding will be represented in the container var layout.
    //
    auto paramGroupTypeLayout = as<IRParameterGroupTypeLayout>(varLayout->getTypeLayout());
    if (!paramGroupTypeLayout)
        return;
    auto containerVarLayout = paramGroupTypeLayout->getContainerVarLayout();
    if (!containerVarLayout)
        return;
    auto containerSpaceOffset =
        varLayout->findOffsetAttr(LayoutResourceKind::SubElementRegisterSpace);
    if (!containerSpaceOffset)
        return;
    spaceOffset += containerSpaceOffset->getOffset();
    for (auto sizeAttr : containerVarLayout->getTypeLayout()->getSizeAttrs())
    {
        auto kind = sizeAttr->getResourceKind();

        if (!ShaderBindingRange::isUsageTracked(kind))
            continue;

        if (auto offsetAttr = containerVarLayout->findOffsetAttr(kind))
        {
            auto spaceIndex = spaceOffset + offsetAttr->getSpace();
            auto registerIndex = offsetAttr->getOffset();
            auto size = sizeAttr->getSize();
            auto count = size.getFiniteValueOr(0);
            _insertBinding(outMetadata.m_usedBindings, kind, spaceIndex, registerIndex, count);
        }
    }
}

// Collects the metadata from the provided IR module, saves it in outMetadata.
void collectMetadata(
    const IRModule* irModule,
    ArtifactPostEmitMetadata& outMetadata,
    CodeGenTarget target)
{
    const bool collectCooperativeTypesMetadata = isSPIRV(target);

    // Scan the instructions looking for global resource declarations,
    // exported functions, and (for SPIR-V) cooperative matrix/vector types.
    for (const auto& inst : irModule->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (func->findDecoration<IRDownstreamModuleExportDecoration>())
            {
                auto name = func->findDecoration<IRExportDecoration>()->getMangledName();
                outMetadata.m_exportedFunctionMangledNames.add(name);
            }

            // Collect metadata from entrypoint params.
            for (auto param : func->getParams())
            {
                collectMetadataFromInst(param, outMetadata);
            }
        }
        else if (auto param = as<IRGlobalParam>(inst))
        {
            collectMetadataFromInst(param, outMetadata);
        }

        if (collectCooperativeTypesMetadata)
        {
            if (auto func = as<IRFunc>(inst))
            {
                // Walk all instructions in the function for cooperative type metadata.
                List<IRInst*> funcInsts;
                findAllInstsBreadthFirst(func, funcInsts);
                for (auto funcInst : funcInsts)
                {
                    if (as<IRDecoration>(funcInst))
                        continue;
                    collectCooperativeTypeFromInst(funcInst, outMetadata);
                }
            }
            else
            {
                // Cooperative types can also appear at global scope.
                collectCooperativeTypeFromInst(inst, outMetadata);
            }
        }
    }
}

} // namespace Slang
