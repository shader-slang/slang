// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

#include <utility>

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
    if (a.inputPackingFactor != b.inputPackingFactor)
        return a.inputPackingFactor < b.inputPackingFactor;
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

static slang::CooperativeVectorType _getCooperativeVectorType(IRInst* inst)
{
    slang::CooperativeVectorType type = {};

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

template<typename T, typename T2>
static Index lowerBound(const List<T>& list, const T2& value)
{
    return lowerBound(
        list,
        value,
        [](const T& currentValue, const T2& searchValue) -> int
        {
            if (currentValue < searchValue)
                return -1;
            if (currentValue == searchValue)
                return 0;
            return +1;
        });
}

template<typename T>
static void _insertSortedUnique(List<T>& list, const T& value)
{
    Index insertIndex = lowerBound(list, value);
    if (insertIndex >= list.getCount() || !(list[insertIndex] == value))
        list.insert(insertIndex, value);
}

static void _insertOrUpdateCooperativeVectorType(
    List<slang::CooperativeVectorType>& list,
    SlangScalarType componentType,
    uint32_t maxSize,
    bool usedForTrainingOp)
{
    if (componentType == SLANG_SCALAR_TYPE_NONE)
        return;

    slang::CooperativeVectorType key = {};
    key.componentType = componentType;

    // Custom compare function since different maxSize and usedForTrainingOp
    // will be collected together into one entry.
    auto compareByComponentType = [](const slang::CooperativeVectorType& a,
                                     const slang::CooperativeVectorType& b) -> int
    {
        if (a.componentType < b.componentType)
            return -1;
        if (a.componentType > b.componentType)
            return 1;
        return 0;
    };

    Index insertIndex = lowerBound(list, key, compareByComponentType);
    if (insertIndex < list.getCount() && compareByComponentType(list[insertIndex], key) == 0)
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

// Operand literal types are guaranteed by validateCooperativeOperations which runs
// before this pass, so cast<> is used instead of as<> + null-check.
static void collectMetadataFromCooperativeVectorCombination(
    IRInst* inst,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto matMulAdd = as<IRCoopVecMatMulAdd>(inst);
    if (!matMulAdd)
        return;

    slang::CooperativeVectorCombination combination = {};

    combination.inputType =
        _getCooperativeVectorType(matMulAdd->getInput()->getDataType()).componentType;

    combination.inputInterpretation =
        _getCooperativeVectorInterpretation(matMulAdd->getInputInterpretation());

    IRIntegerValue packingFactorValue =
        cast<IRIntLit>(matMulAdd->getInputInterpretationPackingFactor())->getValue();
    if (!std::in_range<uint32_t>(packingFactorValue))
        return;
    combination.inputPackingFactor = uint32_t(packingFactorValue);

    combination.matrixInterpretation =
        _getCooperativeVectorInterpretation(matMulAdd->getMatrixInterpretation());

    combination.biasInterpretation = SLANG_SCALAR_TYPE_NONE;
    if (auto biasInterpretation = matMulAdd->getBiasInterpretation())
        combination.biasInterpretation = _getCooperativeVectorInterpretation(biasInterpretation);

    combination.resultType = _getCooperativeVectorType(matMulAdd->getDataType()).componentType;

    combination.transpose = cast<IRBoolLit>(matMulAdd->getTranspose())->getValue();

    if (!combination.inputType || !combination.inputInterpretation ||
        !combination.matrixInterpretation || !combination.resultType)
    {
        return;
    }

    _insertSortedUnique(outMetadata.m_cooperativeVectorCombinations, combination);
}

static void collectMetadataFromCooperativeVectorTrainingUsage(
    IRInst* inst,
    ArtifactPostEmitMetadata& outMetadata)
{
    SlangScalarType trainingType = SLANG_SCALAR_TYPE_NONE;
    uint32_t maxSize = 0;

    if (auto outerProduct = as<IRCoopVecOuterProductAccumulate>(inst))
    {
        // For outer-product accumulation, the training-specific type we want to surface is the
        // matrix accumulation/storage interpretation. That type does not correspond to a
        // cooperative vector width in the public Vulkan API, so keep `maxSize` at zero instead of
        // inheriting the operand vector sizes.
        trainingType = _getCooperativeVectorInterpretation(outerProduct->getMatrixInterpretation());
    }
    else if (auto reduceSum = as<IRCoopVecReduceSumAccumulate>(inst))
    {
        auto vectorType = _getCooperativeVectorType(reduceSum->getValue()->getDataType());
        trainingType = vectorType.componentType;
        maxSize = vectorType.maxSize;
    }
    else
    {
        return;
    }

    _insertOrUpdateCooperativeVectorType(
        outMetadata.m_cooperativeVectorTypes,
        trainingType,
        maxSize,
        true);
}

// Operand literal types are guaranteed by validateCooperativeOperations which runs
// before this pass, so cast<> is used instead of as<> + null-check.
static void collectMetadataFromCooperativeMatrixCombination(
    IRInst* inst,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto matMulAdd = as<IRCoopMatMulAdd>(inst);
    if (!matMulAdd)
        return;

    slang::CooperativeMatrixType typeA =
        _getCooperativeMatrixType(matMulAdd->getMatA()->getDataType());
    slang::CooperativeMatrixType typeB =
        _getCooperativeMatrixType(matMulAdd->getMatB()->getDataType());
    slang::CooperativeMatrixType typeC =
        _getCooperativeMatrixType(matMulAdd->getMatC()->getDataType());
    slang::CooperativeMatrixType typeResult = _getCooperativeMatrixType(inst->getDataType());

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
    combination.saturate = cast<IRBoolLit>(matMulAdd->getSaturatingAccumulation())->getValue();
    // All four matrices are required by validateCoopMatMulAdd to have the same scope.
    combination.scope = typeA.scope;

    _insertSortedUnique(outMetadata.m_cooperativeMatrixCombinations, combination);
}

static void collectCooperativeMetadataFromInst(IRInst* inst, ArtifactPostEmitMetadata& outMetadata)
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
        auto vectorType = _getCooperativeVectorType(resolved);
        _insertOrUpdateCooperativeVectorType(
            outMetadata.m_cooperativeVectorTypes,
            vectorType.componentType,
            vectorType.maxSize,
            false);
    }

    collectMetadataFromCooperativeMatrixCombination(inst, outMetadata);
    collectMetadataFromCooperativeVectorCombination(inst, outMetadata);
    collectMetadataFromCooperativeVectorTrainingUsage(inst, outMetadata);
}

void collectCooperativeMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata)
{
    List<IRInst*> insts;
    findAllInstsBreadthFirst(irModule->getModuleInst(), insts);

    for (auto inst : insts)
    {
        if (as<IRDecoration>(inst))
            continue;
        collectCooperativeMetadataFromInst(inst, outMetadata);
    }
}

} // namespace Slang
