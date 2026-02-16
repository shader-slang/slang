// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

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

static SlangScope _mapIRMemoryScopeToMetadataScope(IRInst* scopeInst)
{
    // IR cooperative matrix types carry `MemoryScope` values, while the public metadata API uses
    // the narrower `SlangScope` enum.
    switch ((MemoryScope)getIntVal(scopeInst))
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

static slang::CooperativeMatrixType _getCooperativeMatrixType(IRInst* inst)
{
    slang::CooperativeMatrixType type = {};

    auto coopMatType = as<IRCoopMatrixType>(getResolvedInstForDecorations(inst));
    if (!coopMatType)
        return type;

    type.componentType = _getComponentTypeFromIRType(coopMatType->getElementType());
    if (type.componentType == SLANG_COOPERATIVE_COMPONENT_TYPE_NONE)
        return slang::CooperativeMatrixType{};

    type.scope = _mapIRMemoryScopeToMetadataScope(coopMatType->getScope());
    type.rowCount = (uint32_t)getIntVal(coopMatType->getRowCount());
    type.columnCount = (uint32_t)getIntVal(coopMatType->getColumnCount());
    type.use = SlangCooperativeMatrixUse(getIntVal(coopMatType->getMatrixUse()));
    return type;
}

static slang::CooperativeVectorType _getCooperativeVectorType(IRInst* inst)
{
    slang::CooperativeVectorType type = {};

    auto coopVecType = as<IRCoopVectorType>(getResolvedInstForDecorations(inst));
    if (!coopVecType)
        return type;

    type.componentType = _getComponentTypeFromIRType(coopVecType->getElementType());
    type.maxSize = (uint32_t)getIntVal(coopVecType->getElementCount());
    return type;
}

template<typename T>
static void _insertSortedUnique(List<T>& list, const T& value)
{
    Index searchResult = list.binarySearch(value);
    if (searchResult < 0)
        list.insert(~searchResult, value);
}

static void _insertOrUpdateCooperativeVectorType(
    List<slang::CooperativeVectorType>& list,
    SlangCooperativeComponentType componentType,
    uint32_t maxSize,
    bool usedForTrainingOp)
{
    if (componentType == SLANG_COOPERATIVE_COMPONENT_TYPE_NONE)
        return;

    // Search by component type only, because training usage is accumulated onto the same
    // metadata entry instead of creating a separate entry.
    Index searchResult = list.binarySearch(
        componentType,
        [](slang::CooperativeVectorType& curObj,
           const SlangCooperativeComponentType& thatType) -> int
        {
            if (curObj.componentType < thatType)
                return -1;
            else if (curObj.componentType == thatType)
                return 0;
            else
                return 1;
        });

    if (searchResult < 0)
    {
        slang::CooperativeVectorType vectorType = {};
        vectorType.componentType = componentType;
        vectorType.maxSize = maxSize;
        vectorType.usedForTrainingOp = usedForTrainingOp;
        list.insert(~searchResult, vectorType);
    }
    else
    {
        if (list[searchResult].maxSize < maxSize)
            list[searchResult].maxSize = maxSize;
        if (usedForTrainingOp)
            list[searchResult].usedForTrainingOp = true;
    }
}

static void collectMetadataFromCooperativeVectorCombination(
    IRInst* inst,
    ArtifactPostEmitMetadata& outMetadata)
{
    slang::CooperativeVectorCombination combination = {};

    if (auto matMulAdd = as<IRCoopVecMatMulAdd>(inst))
    {
        combination.inputType =
            _getCooperativeVectorType(matMulAdd->getInput()->getDataType()).componentType;
        combination.inputInterpretation =
            SlangCooperativeComponentType(getIntVal(matMulAdd->getInputInterpretation()));
        combination.matrixInterpretation =
            SlangCooperativeComponentType(getIntVal(matMulAdd->getMatrixInterpretation()));
        combination.biasInterpretation =
            SlangCooperativeComponentType(getIntVal(matMulAdd->getBiasInterpretation()));
        combination.resultType = _getCooperativeVectorType(matMulAdd->getDataType()).componentType;

        auto transpose = as<IRBoolLit>(matMulAdd->getTranspose());
        if (!transpose)
            return;
        combination.transpose = transpose->getValue();

        if (!combination.inputType || !combination.inputInterpretation ||
            !combination.matrixInterpretation || !combination.biasInterpretation ||
            !combination.resultType)
        {
            return;
        }
    }
    else if (auto matMul = as<IRCoopVecMatMul>(inst))
    {
        combination.inputType =
            _getCooperativeVectorType(matMul->getInput()->getDataType()).componentType;
        combination.inputInterpretation =
            SlangCooperativeComponentType(getIntVal(matMul->getInputInterpretation()));
        combination.matrixInterpretation =
            SlangCooperativeComponentType(getIntVal(matMul->getMatrixInterpretation()));
        combination.biasInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
        combination.resultType = _getCooperativeVectorType(matMul->getDataType()).componentType;

        auto transpose = as<IRBoolLit>(matMul->getTranspose());
        if (!transpose)
            return;
        combination.transpose = transpose->getValue();

        if (!combination.inputType || !combination.inputInterpretation ||
            !combination.matrixInterpretation || !combination.resultType)
        {
            return;
        }
    }
    else
    {
        return;
    }

    _insertSortedUnique(outMetadata.m_cooperativeVectorCombinations, combination);
}

static void collectMetadataFromCooperativeVectorTrainingUsage(
    IRInst* inst,
    ArtifactPostEmitMetadata& outMetadata)
{
    SlangCooperativeComponentType trainingType = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
    uint32_t maxSize = 0;

    if (auto outerProduct = as<IRCoopVecOuterProductAccumulate>(inst))
    {
        // For outer-product accumulation, the training-specific type we want to surface is the
        // matrix accumulation/storage interpretation. That type does not correspond to a
        // cooperative vector width in the public Vulkan API, so keep `maxSize` at zero instead of
        // inheriting the operand vector sizes.
        trainingType =
            SlangCooperativeComponentType(getIntVal(outerProduct->getMatrixInterpretation()));
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

static void collectMetadataFromCooperativeMatrixCombination(
    IRInst* inst,
    ArtifactPostEmitMetadata& outMetadata)
{
    auto matMulAdd = as<IRCoopMatMulAdd>(inst);
    if (!matMulAdd)
        return;

    auto argA = matMulAdd->getMatA();
    auto argB = matMulAdd->getMatB();
    auto argC = matMulAdd->getMatC();
    auto saturate = as<IRBoolLit>(matMulAdd->getSaturatingAccumulation());
    if (!argA || !argB || !argC || !saturate)
        return;

    slang::CooperativeMatrixType typeA = _getCooperativeMatrixType(argA->getDataType());
    slang::CooperativeMatrixType typeB = _getCooperativeMatrixType(argB->getDataType());
    slang::CooperativeMatrixType typeC = _getCooperativeMatrixType(argC->getDataType());
    slang::CooperativeMatrixType typeResult = _getCooperativeMatrixType(inst->getDataType());

    slang::CooperativeMatrixCombination combination = {};
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

    combination.saturate = saturate->getValue();
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
