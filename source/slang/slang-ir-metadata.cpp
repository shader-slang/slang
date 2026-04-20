// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

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
    // The sub-element register space offset already includes the descriptor set allocated for the
    // parameter block. Do not accumulate the parent register-space offset here, or we will double
    // count the descriptor set index for the automatically introduced constant buffer.
    UInt containerSpaceOffsetValue = containerSpaceOffset->getOffset();
    for (auto sizeAttr : containerVarLayout->getTypeLayout()->getSizeAttrs())
    {
        auto kind = sizeAttr->getResourceKind();

        if (!ShaderBindingRange::isUsageTracked(kind))
            continue;

        if (auto offsetAttr = containerVarLayout->findOffsetAttr(kind))
        {
            auto spaceIndex = containerSpaceOffsetValue + offsetAttr->getSpace();
            auto registerIndex = offsetAttr->getOffset();
            auto size = sizeAttr->getSize();
            auto count = size.getFiniteValueOr(0);
            _insertBinding(outMetadata.m_usedBindings, kind, spaceIndex, registerIndex, count);
        }
    }
}

// Collects the metadata from the provided IR module, saves it in outMetadata.
void collectMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata)
{
    // Scan the instructions looking for global resource declarations
    // and exported functions.
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

        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        collectMetadataFromInst(param, outMetadata);
    }
}

static SlangScalarType _getScalarTypeFromIRType(IRType* type)
{
    if (!type)
        return SLANG_SCALAR_TYPE_NONE;
    switch (type->getOp())
    {
    case kIROp_HalfType:
        return SLANG_SCALAR_TYPE_FLOAT16;
    case kIROp_FloatType:
        return SLANG_SCALAR_TYPE_FLOAT32;
    case kIROp_DoubleType:
        return SLANG_SCALAR_TYPE_FLOAT64;
    case kIROp_Int8Type:
        return SLANG_SCALAR_TYPE_INT8;
    case kIROp_Int16Type:
        return SLANG_SCALAR_TYPE_INT16;
    case kIROp_IntType:
        return SLANG_SCALAR_TYPE_INT32;
    case kIROp_Int64Type:
        return SLANG_SCALAR_TYPE_INT64;
    case kIROp_UInt8Type:
        return SLANG_SCALAR_TYPE_UINT8;
    case kIROp_UInt16Type:
        return SLANG_SCALAR_TYPE_UINT16;
    case kIROp_UIntType:
        return SLANG_SCALAR_TYPE_UINT32;
    case kIROp_UInt64Type:
        return SLANG_SCALAR_TYPE_UINT64;
    case kIROp_BFloat16Type:
        return SLANG_SCALAR_TYPE_BFLOAT16;
    case kIROp_FloatE4M3Type:
        return SLANG_SCALAR_TYPE_FLOAT_E4M3;
    case kIROp_FloatE5M2Type:
        return SLANG_SCALAR_TYPE_FLOAT_E5M2;
    default:
        return SLANG_SCALAR_TYPE_NONE;
    }
}

// The scope operand in IRCoopMatrixType stores a raw SPIR-V MemoryScope integer.
// Cast it directly — no remapping needed since slang::MemoryScope matches the SPIR-V values.
static slang::MemoryScope _getScopeFromIRLit(IRInst* scopeInst)
{
    auto lit = as<IRIntLit>(scopeInst);
    if (!lit)
        return slang::MemoryScope::CrossDevice;
    return (slang::MemoryScope)(int32_t)lit->getValue();
}

// Interpretation operands in IRCoopVecMatMulAdd already store SlangScalarType integers,
// pre-mapped by __getCoopVecComponentScalarType() in hlsl.meta.slang at IR generation time.
static SlangScalarType _getCooperativeVectorInterpretation(IRInst* interpInst)
{
    auto lit = as<IRIntLit>(interpInst);
    if (!lit)
        return SLANG_SCALAR_TYPE_NONE;
    auto val = (SlangScalarTypeIntegral)lit->getValue();
    // SLANG_SCALAR_TYPE_FLOAT_E5M2 is the last defined value
    if (val > (SlangScalarTypeIntegral)SLANG_SCALAR_TYPE_FLOAT_E5M2)
        return SLANG_SCALAR_TYPE_NONE;
    return (SlangScalarType)val;
}

static void _insertOrUpdateCooperativeVectorType(
    List<slang::CooperativeVectorType>& list,
    SlangScalarType componentType,
    uint32_t maxSize,
    bool usedForTrainingOp)
{
    for (auto& entry : list)
    {
        if (entry.componentType == componentType)
        {
            if (maxSize > entry.maxSize)
                entry.maxSize = maxSize;
            if (usedForTrainingOp)
                entry.usedForTrainingOp = true;
            return;
        }
    }
    slang::CooperativeVectorType newEntry;
    newEntry.componentType = componentType;
    newEntry.maxSize = maxSize;
    newEntry.usedForTrainingOp = (SlangBool)usedForTrainingOp;
    list.add(newEntry);
}

// Appends value to list only if an equal entry is not already present.
// Lists of cooperative types/combinations are small (bounded by distinct ops in a shader),
// so linear scan is sufficient and avoids middle-of-array insertions.
template<typename T>
static void _insertIfNotPresent(List<T>& list, const T& value)
{
    for (const auto& item : list)
        if (item == value)
            return;
    list.add(value);
}

static void _collectCoopMatrixTypeNode(IRCoopMatrixType* matType, ArtifactPostEmitMetadata& out)
{
    if (!matType)
        return;
    slang::CooperativeMatrixType entry;
    entry.componentType = _getScalarTypeFromIRType(matType->getElementType());
    entry.scope = _getScopeFromIRLit(matType->getScope());
    auto rowLit = as<IRIntLit>(matType->getRowCount());
    auto colLit = as<IRIntLit>(matType->getColumnCount());
    auto useLit = as<IRIntLit>(matType->getMatrixUse());
    if (!rowLit || !colLit || !useLit)
        return;
    entry.rowCount = (uint32_t)rowLit->getValue();
    entry.columnCount = (uint32_t)colLit->getValue();
    entry.use = (SlangCooperativeMatrixUse)useLit->getValue();
    _insertIfNotPresent(out.m_cooperativeMatrixTypes, entry);
}

static void _collectFromCoopMatMulAdd(IRInst* inst, ArtifactPostEmitMetadata& out)
{
    auto mulAdd = as<IRCoopMatMulAdd>(inst);
    if (!mulAdd)
        return;

    auto aType = as<IRCoopMatrixType>(mulAdd->getMatA()->getDataType());
    auto bType = as<IRCoopMatrixType>(mulAdd->getMatB()->getDataType());
    auto cType = as<IRCoopMatrixType>(mulAdd->getMatC()->getDataType());
    auto rType = as<IRCoopMatrixType>(inst->getDataType());
    if (!aType || !bType || !cType || !rType)
        return;

    // Collect individual matrix types
    _collectCoopMatrixTypeNode(aType, out);
    _collectCoopMatrixTypeNode(bType, out);
    _collectCoopMatrixTypeNode(cType, out);
    _collectCoopMatrixTypeNode(rType, out);

    // Collect combination
    auto aRowLit = as<IRIntLit>(aType->getRowCount());
    auto aColLit = as<IRIntLit>(aType->getColumnCount());
    auto bColLit = as<IRIntLit>(bType->getColumnCount());
    auto saturateLit = as<IRBoolLit>(mulAdd->getSaturatingAccumulation());
    if (!aRowLit || !aColLit || !bColLit)
        return;

    slang::CooperativeMatrixCombination combo;
    combo.m = (uint32_t)aRowLit->getValue();
    combo.k = (uint32_t)aColLit->getValue();
    combo.n = (uint32_t)bColLit->getValue();
    combo.componentTypeA = _getScalarTypeFromIRType(aType->getElementType());
    combo.componentTypeB = _getScalarTypeFromIRType(bType->getElementType());
    combo.componentTypeC = _getScalarTypeFromIRType(cType->getElementType());
    combo.componentTypeResult = _getScalarTypeFromIRType(rType->getElementType());
    combo.saturate = saturateLit ? (SlangBool)saturateLit->getValue() : false;
    combo.scope = _getScopeFromIRLit(aType->getScope());

    _insertIfNotPresent(out.m_cooperativeMatrixCombinations, combo);
}

static void _collectFromCoopVecMatMulAdd(IRInst* inst, ArtifactPostEmitMetadata& out)
{
    auto mulAdd = as<IRCoopVecMatMulAdd>(inst);
    if (!mulAdd)
        return;

    auto inputVecType = as<IRCoopVectorType>(mulAdd->getInput()->getDataType());
    auto resultVecType = as<IRCoopVectorType>(inst->getDataType());
    if (!inputVecType || !resultVecType)
        return;

    slang::CooperativeVectorCombination combo;
    combo.inputType = _getScalarTypeFromIRType(inputVecType->getElementType());
    combo.inputInterpretation =
        _getCooperativeVectorInterpretation(mulAdd->getInputInterpretation());
    auto packLit = as<IRIntLit>(mulAdd->getInputInterpretationPackingFactor());
    combo.inputPackingFactor = packLit ? (uint32_t)packLit->getValue() : 1;
    combo.matrixInterpretation =
        _getCooperativeVectorInterpretation(mulAdd->getMatrixInterpretation());
    auto biasOp = mulAdd->getBiasInterpretation();
    combo.biasInterpretation =
        biasOp ? _getCooperativeVectorInterpretation(biasOp) : SLANG_SCALAR_TYPE_NONE;
    combo.resultType = _getScalarTypeFromIRType(resultVecType->getElementType());
    auto layoutLit = as<IRIntLit>(mulAdd->getMemoryLayout());
    combo.memoryLayout = layoutLit ? (SlangCooperativeVectorMatrixLayout)layoutLit->getValue()
                                   : SLANG_COOPERATIVE_VECTOR_MATRIX_LAYOUT_ROW_MAJOR;
    auto transposeLit = as<IRBoolLit>(mulAdd->getTranspose());
    combo.transpose = transposeLit ? (SlangBool)transposeLit->getValue() : false;

    _insertIfNotPresent(out.m_cooperativeVectorCombinations, combo);

    // Also update vector types for input and result
    auto inputSizeLit = as<IRIntLit>(inputVecType->getElementCount());
    uint32_t inputSize = inputSizeLit ? (uint32_t)inputSizeLit->getValue() : 0;
    _insertOrUpdateCooperativeVectorType(
        out.m_cooperativeVectorTypes,
        combo.inputType,
        inputSize,
        false);

    auto resultSizeLit = as<IRIntLit>(resultVecType->getElementCount());
    uint32_t resultSize = resultSizeLit ? (uint32_t)resultSizeLit->getValue() : 0;
    _insertOrUpdateCooperativeVectorType(
        out.m_cooperativeVectorTypes,
        combo.resultType,
        resultSize,
        false);
}

static void _collectFromCoopVecTrainingUsage(IRInst* inst, ArtifactPostEmitMetadata& out)
{
    if (inst->getOp() == kIROp_CoopVecOuterProductAccumulate)
    {
        auto outerProd = as<IRCoopVecOuterProductAccumulate>(inst);
        if (!outerProd)
            return;
        SlangScalarType trainingType =
            _getCooperativeVectorInterpretation(outerProd->getMatrixInterpretation());
        // maxSize=0: outer-product has no fixed vector width for Vulkan driver queries
        _insertOrUpdateCooperativeVectorType(out.m_cooperativeVectorTypes, trainingType, 0, true);
    }
    else if (inst->getOp() == kIROp_CoopVecReduceSumAccumulate)
    {
        auto reduceSum = as<IRCoopVecReduceSumAccumulate>(inst);
        if (!reduceSum)
            return;
        auto valVecType = as<IRCoopVectorType>(reduceSum->getValue()->getDataType());
        if (!valVecType)
            return;
        SlangScalarType elemType = _getScalarTypeFromIRType(valVecType->getElementType());
        auto sizeLit = as<IRIntLit>(valVecType->getElementCount());
        uint32_t maxSize = sizeLit ? (uint32_t)sizeLit->getValue() : 0;
        _insertOrUpdateCooperativeVectorType(out.m_cooperativeVectorTypes, elemType, maxSize, true);
    }
}

void collectCooperativeMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata)
{
    List<IRInst*> insts;
    findAllInstsBreadthFirst(irModule->getModuleInst(), insts);
    for (auto inst : insts)
    {
        // Skip decorations — they are attributes on other instructions, not standalone ops.
        if (as<IRDecoration>(inst))
            continue;

        switch (inst->getOp())
        {
        case kIROp_CoopMatrixType:
            _collectCoopMatrixTypeNode(as<IRCoopMatrixType>(inst), outMetadata);
            break;
        case kIROp_CoopMatMulAdd:
            _collectFromCoopMatMulAdd(inst, outMetadata);
            break;
        case kIROp_CoopVecMatMulAdd:
            _collectFromCoopVecMatMulAdd(inst, outMetadata);
            break;
        case kIROp_CoopVecOuterProductAccumulate:
        case kIROp_CoopVecReduceSumAccumulate:
            _collectFromCoopVecTrainingUsage(inst, outMetadata);
            break;
        default:
            break;
        }
    }
}

} // namespace Slang
