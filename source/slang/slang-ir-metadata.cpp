// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

#include <utility>

namespace Slang
{
template<typename T>
static int _compare(T a, T b)
{
    if (a < b)
        return -1;
    if (a > b)
        return +1;
    return 0;
}

static int _compareCooperativeMatrixType(
    const slang::CooperativeMatrixType& a,
    const slang::CooperativeMatrixType& b)
{
    if (int result = _compare(a.componentType, b.componentType))
        return result;
    if (int result = _compare(a.scope, b.scope))
        return result;
    if (int result = _compare(a.rowCount, b.rowCount))
        return result;
    if (int result = _compare(a.columnCount, b.columnCount))
        return result;
    return _compare(a.use, b.use);
}

static int _compareCooperativeMatrixCombination(
    const slang::CooperativeMatrixCombination& a,
    const slang::CooperativeMatrixCombination& b)
{
    if (int result = _compare(a.m, b.m))
        return result;
    if (int result = _compare(a.n, b.n))
        return result;
    if (int result = _compare(a.k, b.k))
        return result;
    if (int result = _compare(a.componentTypeA, b.componentTypeA))
        return result;
    if (int result = _compare(a.componentTypeB, b.componentTypeB))
        return result;
    if (int result = _compare(a.componentTypeC, b.componentTypeC))
        return result;
    if (int result = _compare(a.componentTypeResult, b.componentTypeResult))
        return result;
    if (int result = _compare(a.saturate, b.saturate))
        return result;
    return _compare(a.scope, b.scope);
}

static int _compareCooperativeVectorTypeUsageInfoByComponentType(
    const slang::CooperativeVectorTypeUsageInfo& a,
    const slang::CooperativeVectorTypeUsageInfo& b)
{
    return _compare(a.componentType, b.componentType);
}

static int _compareCooperativeVectorCombination(
    const slang::CooperativeVectorCombination& a,
    const slang::CooperativeVectorCombination& b)
{
    if (int result = _compare(a.inputType, b.inputType))
        return result;
    if (int result = _compare(a.inputInterpretation, b.inputInterpretation))
        return result;
    if (int result = _compare(a.inputPackingFactor, b.inputPackingFactor))
        return result;
    if (int result = _compare(a.matrixInterpretation, b.matrixInterpretation))
        return result;
    if (int result = _compare(a.biasInterpretation, b.biasInterpretation))
        return result;
    if (int result = _compare(a.resultType, b.resultType))
        return result;
    return _compare(a.transpose, b.transpose);
}

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
        break;
    }
    return SLANG_SCALAR_TYPE_NONE;
}

static bool _tryGetIntLiteralValue(IRInst* inst, IRIntegerValue& outValue)
{
    if (auto intLit = as<IRIntLit>(inst))
    {
        outValue = intLit->getValue();
        return true;
    }
    return false;
}

static SlangScope _getCooperativeMatrixScope(IRInst* scopeInst)
{
    IRIntegerValue val = 0;
    if (!_tryGetIntLiteralValue(scopeInst, val) || !std::in_range<int32_t>(val))
        return SLANG_SCOPE_NONE;
    switch (MemoryScope(val))
    {
    case MemoryScope::Workgroup:
        return SLANG_SCOPE_THREAD_GROUP;
    case MemoryScope::Subgroup:
        return SLANG_SCOPE_WAVE;
    case MemoryScope::Invocation:
        return SLANG_SCOPE_THREAD;
    default:
        return SLANG_SCOPE_NONE;
    }
}

static SlangScalarType _getCooperativeVectorInterpretation(IRInst* interpInst)
{
    IRIntegerValue val = 0;
    if (!_tryGetIntLiteralValue(interpInst, val) || !std::in_range<SlangScalarTypeIntegral>(val))
        return SLANG_SCALAR_TYPE_NONE;

    auto scalarType = SlangScalarType(val);
    switch (scalarType)
    {
    case SLANG_SCALAR_TYPE_NONE:
    case SLANG_SCALAR_TYPE_VOID:
    case SLANG_SCALAR_TYPE_BOOL:
    case SLANG_SCALAR_TYPE_INT32:
    case SLANG_SCALAR_TYPE_UINT32:
    case SLANG_SCALAR_TYPE_INT64:
    case SLANG_SCALAR_TYPE_UINT64:
    case SLANG_SCALAR_TYPE_FLOAT16:
    case SLANG_SCALAR_TYPE_FLOAT32:
    case SLANG_SCALAR_TYPE_FLOAT64:
    case SLANG_SCALAR_TYPE_INT8:
    case SLANG_SCALAR_TYPE_UINT8:
    case SLANG_SCALAR_TYPE_INT16:
    case SLANG_SCALAR_TYPE_UINT16:
    case SLANG_SCALAR_TYPE_INTPTR:
    case SLANG_SCALAR_TYPE_UINTPTR:
    case SLANG_SCALAR_TYPE_BFLOAT16:
    case SLANG_SCALAR_TYPE_FLOAT_E4M3:
    case SLANG_SCALAR_TYPE_FLOAT_E5M2:
        return scalarType;
    }
    // No default case above: want compiler warning if enum grows.
    return SLANG_SCALAR_TYPE_NONE;
}

static slang::CooperativeMatrixType _getCooperativeMatrixType(IRInst* inst)
{
    slang::CooperativeMatrixType type = {};

    auto coopMatType = as<IRCoopMatrixType>(inst);
    if (!coopMatType)
        return {};

    type.componentType = _getScalarTypeFromIRType(coopMatType->getElementType());
    if (type.componentType == SLANG_SCALAR_TYPE_NONE)
        return {};

    if (!as<IRIntLit>(coopMatType->getRowCount()) || !as<IRIntLit>(coopMatType->getColumnCount()) ||
        !as<IRIntLit>(coopMatType->getMatrixUse()))
        return {};

    type.scope = _getCooperativeMatrixScope(coopMatType->getScope());
    if (type.scope == SLANG_SCOPE_NONE)
        return {};

    IRIntegerValue rowValue = getIntVal(coopMatType->getRowCount());
    IRIntegerValue columnValue = getIntVal(coopMatType->getColumnCount());
    if (!std::in_range<uint32_t>(rowValue) || !std::in_range<uint32_t>(columnValue))
        return {};

    type.rowCount = uint32_t(rowValue);
    type.columnCount = uint32_t(columnValue);

    IRIntegerValue matrixUseValue = getIntVal(coopMatType->getMatrixUse());
    if (!std::in_range<SlangCooperativeMatrixUseIntegral>(matrixUseValue))
        return {};

    auto matrixUse = SlangCooperativeMatrixUse(matrixUseValue);
    switch (matrixUse)
    {
    case SLANG_COOPERATIVE_MATRIX_USE_A:
    case SLANG_COOPERATIVE_MATRIX_USE_B:
    case SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR:
        type.use = matrixUse;
        return type;
    }
    // No default case above: want compiler warning if enum grows.
    return {};
}

static slang::CooperativeVectorTypeUsageInfo _getCooperativeVectorTypeUsageInfo(IRInst* inst)
{
    slang::CooperativeVectorTypeUsageInfo type = {};

    auto coopVecType = as<IRCoopVectorType>(inst);
    if (!coopVecType)
        return {};

    type.componentType = _getScalarTypeFromIRType(coopVecType->getElementType());
    if (type.componentType == SLANG_SCALAR_TYPE_NONE)
        return {};

    if (!as<IRIntLit>(coopVecType->getElementCount()))
        return {};

    IRIntegerValue maxSizeValue = getIntVal(coopVecType->getElementCount());
    if (!std::in_range<uint32_t>(maxSizeValue))
        return {};

    type.maxSize = uint32_t(maxSizeValue);
    return type;
}

template<typename T, typename T2, typename Compare>
static Index lowerBound(const List<T>& list, const T2& value, Compare compare)
{
    Index imin = 0;
    Index imax = list.getCount();
    while (imax > imin)
    {
        Index imid = imin + ((imax - imin) >> 1);
        if (compare(list[imid], value) < 0)
            imin = imid + 1;
        else
            imax = imid;
    }
    return imin;
}

template<typename T, typename Compare>
static void _insertSortedUnique(List<T>& list, const T& value, Compare compare)
{
    Index insertIndex = lowerBound(list, value, compare);
    if (insertIndex >= list.getCount() || compare(list[insertIndex], value) != 0)
        list.insert(insertIndex, value);
}

static void _insertOrUpdateCooperativeVectorTypeUsageInfo(
    List<slang::CooperativeVectorTypeUsageInfo>& list,
    SlangScalarType componentType,
    uint32_t maxSize,
    bool usedForTrainingOp)
{
    if (componentType == SLANG_SCALAR_TYPE_NONE)
        return;

    slang::CooperativeVectorTypeUsageInfo key = {};
    key.componentType = componentType;

    Index insertIndex =
        lowerBound(list, key, _compareCooperativeVectorTypeUsageInfoByComponentType);
    if (insertIndex < list.getCount() &&
        _compareCooperativeVectorTypeUsageInfoByComponentType(list[insertIndex], key) == 0)
    {
        auto& existing = list[insertIndex];
        if (existing.maxSize < maxSize)
            existing.maxSize = maxSize;
        if (usedForTrainingOp)
            existing.usedForTrainingOp = true;
    }
    else
    {
        key.maxSize = maxSize;
        key.usedForTrainingOp = usedForTrainingOp;
        list.insert(insertIndex, key);
    }
}

static bool isValidInputInterpretation(
    IRIntegerValue inputInterpretation,
    IRIntegerValue packingFactor)
{
    switch ((int32_t)inputInterpretation)
    {
    case SLANG_SCALAR_TYPE_INT8:
    case SLANG_SCALAR_TYPE_UINT8:
        return packingFactor == 1 || packingFactor == 4;

    default:
        return packingFactor == 1;
    }
}

static bool isValidCoopVecDataOperand(IRInst* operand)
{
    auto type = operand->getDataType();
    if (auto rateQualifiedType = as<IRRateQualifiedType>(type))
        type = rateQualifiedType->getValueType();

    type = unwrapArray(type);
    return as<IRPtrTypeBase>(type) || as<IRHLSLStructuredBufferTypeBase>(type) ||
           as<IRByteAddressBufferTypeBase>(type);
}

static bool isValidCoopVecAccumulationOperand(IRInst* operand)
{
    auto type = operand->getDataType();
    if (auto rateQualifiedType = as<IRRateQualifiedType>(type))
        type = rateQualifiedType->getValueType();

    type = unwrapArray(type);

    if (as<IRPtrTypeBase>(type))
        return true;

    switch (type->getOp())
    {
    case kIROp_HLSLRWStructuredBufferType:
    case kIROp_HLSLRasterizerOrderedStructuredBufferType:
    case kIROp_HLSLRWByteAddressBufferType:
    case kIROp_HLSLRasterizerOrderedByteAddressBufferType:
        return true;
    default:
        return false;
    }
}

// Validates operands of `IRCoopMatMulAdd` and, when valid, emits a metadata entry describing
// the matrix multiply-accumulate combination it performs. Diagnoses malformed operands on
// `sink`; no metadata is recorded for instructions that fail validation.
static void collectMetadataFromCooperativeMatrixCombination(
    IRInst* inst,
    DiagnosticSink* sink,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto matMulAdd = as<IRCoopMatMulAdd>(inst);
    if (!matMulAdd)
        return;

    auto emitDiagnostic = [=](const char* message)
    {
        sink->diagnose(Diagnostics::IrValidationFailed{
            .message = message,
            .location = inst->sourceLoc,
        });
    };

    if (inst->getOperandCount() != 4)
    {
        emitDiagnostic("Malformed CoopMatMulAdd operand list");
        return;
    }

    auto aType = as<IRCoopMatrixType>(matMulAdd->getMatA()->getDataType());
    auto bType = as<IRCoopMatrixType>(matMulAdd->getMatB()->getDataType());
    auto cType = as<IRCoopMatrixType>(matMulAdd->getMatC()->getDataType());
    auto resultType = as<IRCoopMatrixType>(inst->getDataType());
    if (!aType || !bType || !cType || !resultType)
    {
        emitDiagnostic(
            "CoopMatMulAdd input and result operands must have cooperative matrix types");
        return;
    }

    auto saturate = as<IRBoolLit>(matMulAdd->getSaturatingAccumulation());
    if (!saturate)
    {
        emitDiagnostic("CoopMatMulAdd saturatingAccumulation operand must be a bool literal");
        return;
    }

    auto aUse = as<IRIntLit>(aType->getMatrixUse());
    auto bUse = as<IRIntLit>(bType->getMatrixUse());
    auto cUse = as<IRIntLit>(cType->getMatrixUse());
    auto resultUse = as<IRIntLit>(resultType->getMatrixUse());
    if (!aUse || !bUse || !cUse || !resultUse)
    {
        emitDiagnostic("CoopMatMulAdd cooperative matrix uses must be integer literals");
        return;
    }

    if (aUse->getValue() != SLANG_COOPERATIVE_MATRIX_USE_A ||
        bUse->getValue() != SLANG_COOPERATIVE_MATRIX_USE_B ||
        cUse->getValue() != SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR ||
        resultUse->getValue() != SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR)
    {
        emitDiagnostic("CoopMatMulAdd requires MatrixA, MatrixB, and accumulator matrix operands");
        return;
    }

    auto aScope = as<IRIntLit>(aType->getScope());
    auto bScope = as<IRIntLit>(bType->getScope());
    auto cScope = as<IRIntLit>(cType->getScope());
    auto resultScope = as<IRIntLit>(resultType->getScope());
    if (!aScope || !bScope || !cScope || !resultScope)
    {
        emitDiagnostic("CoopMatMulAdd scopes must be integer literals");
        return;
    }

    if (aScope->getValue() != bScope->getValue() || aScope->getValue() != cScope->getValue() ||
        aScope->getValue() != resultScope->getValue())
    {
        emitDiagnostic(
            "CoopMatMulAdd requires all cooperative matrix operands to use the same scope");
        return;
    }

    auto aRows = as<IRIntLit>(aType->getRowCount());
    auto aCols = as<IRIntLit>(aType->getColumnCount());
    auto bRows = as<IRIntLit>(bType->getRowCount());
    auto bCols = as<IRIntLit>(bType->getColumnCount());
    auto cRows = as<IRIntLit>(cType->getRowCount());
    auto cCols = as<IRIntLit>(cType->getColumnCount());
    auto resultRows = as<IRIntLit>(resultType->getRowCount());
    auto resultCols = as<IRIntLit>(resultType->getColumnCount());
    if (!aRows || !aCols || !bRows || !bCols || !cRows || !cCols || !resultRows || !resultCols)
    {
        emitDiagnostic("CoopMatMulAdd row and column counts must be integer literals");
        return;
    }

    if (aRows->getValue() != cRows->getValue() || aRows->getValue() != resultRows->getValue() ||
        aCols->getValue() != bRows->getValue() || bCols->getValue() != cCols->getValue() ||
        bCols->getValue() != resultCols->getValue())
    {
        emitDiagnostic(
            "CoopMatMulAdd operand dimensions must satisfy A(MxK), B(KxN), and C/result(MxN)");
        return;
    }

    // All literal and consistency checks have passed. Extract structured metadata for the
    // operands. `_getCooperativeMatrixType` may still return an empty struct if a scope value
    // does not map to a public `SlangScope`; in that case we silently skip this combination.
    slang::CooperativeMatrixType typeA = _getCooperativeMatrixType(aType);
    slang::CooperativeMatrixType typeB = _getCooperativeMatrixType(bType);
    slang::CooperativeMatrixType typeC = _getCooperativeMatrixType(cType);
    slang::CooperativeMatrixType typeResult = _getCooperativeMatrixType(resultType);
    if (!typeA.componentType || !typeB.componentType || !typeC.componentType ||
        !typeResult.componentType)
    {
        return;
    }

    slang::CooperativeMatrixCombination combination = {};
    combination.m = typeA.rowCount;
    combination.n = typeB.columnCount;
    combination.k = typeA.columnCount;
    combination.componentTypeA = typeA.componentType;
    combination.componentTypeB = typeB.componentType;
    combination.componentTypeC = typeC.componentType;
    combination.componentTypeResult = typeResult.componentType;
    combination.saturate = saturate->getValue();
    combination.scope = typeA.scope;

    _insertSortedUnique(
        outMetadata.m_cooperativeMatrixCombinations,
        combination,
        _compareCooperativeMatrixCombination);
}

// Validates operands of `IRCoopVecMatMulAdd` and, when valid, emits a metadata entry for the
// cooperative-vector matrix-multiply combination. Diagnoses malformed operands on `sink`.
static void collectMetadataFromCooperativeVectorCombination(
    IRInst* inst,
    DiagnosticSink* sink,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto matMulAdd = as<IRCoopVecMatMulAdd>(inst);
    if (!matMulAdd)
        return;

    auto emitDiagnostic = [=](const char* message)
    {
        sink->diagnose(Diagnostics::IrValidationFailed{
            .message = message,
            .location = inst->sourceLoc,
        });
    };

    if (inst->getOperandCount() != 10 && inst->getOperandCount() != 13)
    {
        emitDiagnostic("Malformed CoopVecMatMulAdd operand list");
        return;
    }

    const bool hasBias = inst->getOperandCount() == 13;

    auto resultType = as<IRCoopVectorType>(inst->getDataType());
    auto inputType = as<IRCoopVectorType>(matMulAdd->getInput()->getDataType());
    if (!resultType || !inputType)
    {
        emitDiagnostic(
            "CoopVecMatMulAdd input and result operands must have cooperative vector types");
        return;
    }

    auto inputInterpretation = as<IRIntLit>(matMulAdd->getInputInterpretation());
    if (!inputInterpretation)
    {
        emitDiagnostic("CoopVecMatMulAdd inputInterpretation operand must be an integer literal");
        return;
    }

    auto matrixInterpretation = as<IRIntLit>(matMulAdd->getMatrixInterpretation());
    if (!matrixInterpretation)
    {
        emitDiagnostic("CoopVecMatMulAdd matrixInterpretation operand must be an integer literal");
        return;
    }

    if (!isValidCoopVecDataOperand(matMulAdd->getMatrixPtr()))
    {
        emitDiagnostic("CoopVecMatMulAdd matrix operand must be a pointer, ByteAddressBuffer, or "
                       "StructuredBuffer");
        return;
    }

    IRInst* biasInterpretationOp = nullptr;
    if (hasBias)
    {
        biasInterpretationOp = matMulAdd->getBiasInterpretation();
        if (!as<IRIntLit>(biasInterpretationOp))
        {
            emitDiagnostic(
                "CoopVecMatMulAdd biasInterpretation operand must be an integer literal");
            return;
        }

        if (!isValidCoopVecDataOperand(matMulAdd->getBiasPtr()))
        {
            emitDiagnostic("CoopVecMatMulAdd bias operand must be a pointer, ByteAddressBuffer, or "
                           "StructuredBuffer");
            return;
        }
    }

    if (!as<IRIntLit>(matMulAdd->getMemoryLayout()))
    {
        emitDiagnostic("CoopVecMatMulAdd memoryLayout operand must be an integer literal");
        return;
    }

    auto transpose = as<IRBoolLit>(matMulAdd->getTranspose());
    if (!transpose)
    {
        emitDiagnostic("CoopVecMatMulAdd transpose operand must be a bool literal");
        return;
    }

    auto k = as<IRIntLit>(matMulAdd->getK());
    if (!k)
    {
        emitDiagnostic("CoopVecMatMulAdd k operand must be an integer literal");
        return;
    }

    auto packingFactor = as<IRIntLit>(matMulAdd->getInputInterpretationPackingFactor());
    if (!packingFactor)
    {
        emitDiagnostic(
            "CoopVecMatMulAdd inputInterpretationPackingFactor operand must be an integer literal");
        return;
    }

    auto inputElementCount = as<IRIntLit>(inputType->getElementCount());
    if (!inputElementCount)
    {
        emitDiagnostic("CoopVecMatMulAdd input element count must be known at compile time");
        return;
    }

    if (!isValidInputInterpretation(inputInterpretation->getValue(), packingFactor->getValue()))
    {
        emitDiagnostic(
            "CoopVecMatMulAdd input interpretation is invalid for the specified packing factor");
        return;
    }

    if (packingFactor->getValue() == 1)
    {
        if (k->getValue() != inputElementCount->getValue())
        {
            emitDiagnostic("CoopVecMatMulAdd k operand must match input vector element count for "
                           "non-packed input interpretations");
            return;
        }
    }
    else
    {
        if (k->getValue() > packingFactor->getValue() * inputElementCount->getValue())
        {
            emitDiagnostic(
                "CoopVecMatMulAdd k operand must be less than or equal to the input vector element "
                "count times the packing factor for packed input interpretations");
            return;
        }
    }

    if (!std::in_range<uint32_t>(packingFactor->getValue()))
        return;

    slang::CooperativeVectorCombination combination = {};
    combination.inputType = _getScalarTypeFromIRType(inputType->getElementType());
    combination.inputInterpretation = _getCooperativeVectorInterpretation(inputInterpretation);
    combination.inputPackingFactor = uint32_t(packingFactor->getValue());
    combination.matrixInterpretation = _getCooperativeVectorInterpretation(matrixInterpretation);
    combination.biasInterpretation = SLANG_SCALAR_TYPE_NONE;
    if (biasInterpretationOp)
        combination.biasInterpretation = _getCooperativeVectorInterpretation(biasInterpretationOp);
    combination.resultType = _getScalarTypeFromIRType(resultType->getElementType());
    combination.transpose = transpose->getValue();

    if (!combination.inputType || !combination.inputInterpretation ||
        !combination.matrixInterpretation || !combination.resultType)
    {
        return;
    }

    _insertSortedUnique(
        outMetadata.m_cooperativeVectorCombinations,
        combination,
        _compareCooperativeVectorCombination);
}

// Validates operands of cooperative-vector training ops (`IRCoopVecOuterProductAccumulate` and
// `IRCoopVecReduceSumAccumulate`) and records the training-specific component type in the
// cooperative-vector type-usage list. Diagnoses malformed operands on `sink`.
static void collectMetadataFromCooperativeVectorTrainingUsage(
    IRInst* inst,
    DiagnosticSink* sink,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto emitDiagnostic = [=](const char* message)
    {
        sink->diagnose(Diagnostics::IrValidationFailed{
            .message = message,
            .location = inst->sourceLoc,
        });
    };

    SlangScalarType trainingType = SLANG_SCALAR_TYPE_NONE;
    uint32_t maxSize = 0;

    if (auto outerProduct = as<IRCoopVecOuterProductAccumulate>(inst))
    {
        if (inst->getOperandCount() != 7)
        {
            emitDiagnostic("Malformed CoopVecOuterProductAccumulate operand list");
            return;
        }

        if (!as<IRCoopVectorType>(outerProduct->getA()->getDataType()) ||
            !as<IRCoopVectorType>(outerProduct->getB()->getDataType()))
        {
            emitDiagnostic("CoopVecOuterProductAccumulate a and b operands must have cooperative "
                           "vector types");
            return;
        }

        if (!as<IRIntLit>(outerProduct->getMemoryLayout()))
        {
            emitDiagnostic(
                "CoopVecOuterProductAccumulate memoryLayout operand must be an integer literal");
            return;
        }

        auto matrixInterp = as<IRIntLit>(outerProduct->getMatrixInterpretation());
        if (!matrixInterp)
        {
            emitDiagnostic("CoopVecOuterProductAccumulate matrixInterpretation operand must be an "
                           "integer literal");
            return;
        }

        if (!isValidCoopVecAccumulationOperand(outerProduct->getMatrixPtr()))
        {
            emitDiagnostic("CoopVecOuterProductAccumulate matrix operand must be a writable "
                           "pointer, RWByteAddressBuffer, or RWStructuredBuffer");
            return;
        }

        if (inst->getDataType()->getOp() != kIROp_VoidType)
        {
            emitDiagnostic("CoopVecOuterProductAccumulate result type must be void");
            return;
        }

        // For outer-product accumulation, the training-specific type we want to surface is the
        // matrix accumulation/storage interpretation. That usage does not correspond to a
        // cooperative vector width in the public Vulkan API, so keep `maxSize` at zero instead of
        // inheriting the operand vector sizes.
        trainingType = _getCooperativeVectorInterpretation(matrixInterp);
    }
    else if (auto reduceSum = as<IRCoopVecReduceSumAccumulate>(inst))
    {
        if (inst->getOperandCount() != 3)
        {
            emitDiagnostic("Malformed CoopVecReduceSumAccumulate operand list");
            return;
        }

        if (!as<IRCoopVectorType>(reduceSum->getValue()->getDataType()))
        {
            emitDiagnostic(
                "CoopVecReduceSumAccumulate value operand must have a cooperative vector type");
            return;
        }

        if (!isValidCoopVecAccumulationOperand(reduceSum->getBufferPtr()))
        {
            emitDiagnostic("CoopVecReduceSumAccumulate buffer operand must be a writable pointer, "
                           "RWByteAddressBuffer, or RWStructuredBuffer");
            return;
        }

        if (inst->getDataType()->getOp() != kIROp_VoidType)
        {
            emitDiagnostic("CoopVecReduceSumAccumulate result type must be void");
            return;
        }

        auto vectorType = _getCooperativeVectorTypeUsageInfo(reduceSum->getValue()->getDataType());
        trainingType = vectorType.componentType;
        maxSize = vectorType.maxSize;
    }
    else
    {
        return;
    }

    _insertOrUpdateCooperativeVectorTypeUsageInfo(
        outMetadata.m_cooperativeVectorTypes,
        trainingType,
        maxSize,
        true);
}

static void collectCooperativeMetadataFromInst(
    IRInst* inst,
    DiagnosticSink* sink,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto resolved = getResolvedInstForDecorations(inst);

    if (as<IRCoopMatrixType>(resolved))
    {
        auto matrixType = _getCooperativeMatrixType(resolved);
        if (matrixType.componentType)
            _insertSortedUnique(
                outMetadata.m_cooperativeMatrixTypes,
                matrixType,
                _compareCooperativeMatrixType);
    }
    else if (as<IRCoopVectorType>(resolved))
    {
        auto vectorType = _getCooperativeVectorTypeUsageInfo(resolved);
        _insertOrUpdateCooperativeVectorTypeUsageInfo(
            outMetadata.m_cooperativeVectorTypes,
            vectorType.componentType,
            vectorType.maxSize,
            false);
    }

    collectMetadataFromCooperativeMatrixCombination(inst, sink, outMetadata);
    collectMetadataFromCooperativeVectorCombination(inst, sink, outMetadata);
    collectMetadataFromCooperativeVectorTrainingUsage(inst, sink, outMetadata);
}

void collectCooperativeMetadata(
    const IRModule* irModule,
    DiagnosticSink* sink,
    ArtifactPostEmitMetadata& outMetadata)
{
    List<IRInst*> insts;
    findAllInstsBreadthFirst(irModule->getModuleInst(), insts);

    for (auto inst : insts)
    {
        if (as<IRDecoration>(inst))
            continue;
        collectCooperativeMetadataFromInst(inst, sink, outMetadata);
    }
}

} // namespace Slang
