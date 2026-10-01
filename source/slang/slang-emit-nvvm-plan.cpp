#include "slang-emit-nvvm-plan.h"

namespace Slang
{

namespace
{

template<typename T>
void _indexOperations(const List<T>& operations, Dictionary<IRInst*, Index>& outIndices)
{
    for (Index i = 0; i < operations.getCount(); ++i)
    {
        SLANG_RELEASE_ASSERT(operations[i].source);
        SLANG_RELEASE_ASSERT(!outIndices.containsKey(operations[i].source));
        outIndices[operations[i].source] = i;
    }
}

template<typename T>
const T* _findOperation(
    const List<T>& operations,
    const Dictionary<IRInst*, Index>& indices,
    IRInst* source)
{
    const Index* index = indices.tryGetValue(source);
    return index ? &operations[*index] : nullptr;
}

} // namespace

void NVVMAddressPlan::addFieldAddress(const NVVMPlannedFieldAddress& address)
{
    SLANG_RELEASE_ASSERT(address.source && !m_fieldAddressIndices.containsKey(address.source));
    SLANG_RELEASE_ASSERT(address.root && address.selection.field);
    m_fieldAddressIndices[address.source] = m_fieldAddresses.getCount();
    m_fieldAddresses.add(address);
}

void NVVMAddressPlan::addElementAddress(const NVVMPlannedElementAddress& address)
{
    SLANG_RELEASE_ASSERT(address.source && !m_elementAddressIndices.containsKey(address.source));
    SLANG_RELEASE_ASSERT(address.root && address.kind != NVVMElementAddressKind::Pending);
    m_elementAddressIndices[address.source] = m_elementAddresses.getCount();
    m_elementAddresses.add(address);
}

const NVVMPlannedFieldAddress* NVVMAddressPlan::findFieldAddress(IRInst* source) const
{
    return _findOperation(m_fieldAddresses, m_fieldAddressIndices, source);
}

void NVVMAddressPlan::addDataPointer(const NVVMRawBufferDataPointer& address)
{
    SLANG_RELEASE_ASSERT(address.source && !m_dataPointerIndices.containsKey(address.source));
    m_dataPointerIndices[address.source] = m_dataPointers.getCount();
    m_dataPointers.add(address);
}

void NVVMAddressPlan::addStructuredElement(const NVVMStructuredBufferElementPointer& address)
{
    SLANG_RELEASE_ASSERT(address.source && !m_structuredElementIndices.containsKey(address.source));
    m_structuredElementIndices[address.source] = m_structuredElements.getCount();
    m_structuredElements.add(address);
}

const NVVMRawBufferDataPointer* NVVMAddressPlan::findDataPointer(IRInst* source) const
{
    return _findOperation(m_dataPointers, m_dataPointerIndices, source);
}

const NVVMStructuredBufferElementPointer* NVVMAddressPlan::findStructuredElement(
    IRInst* source) const
{
    return _findOperation(m_structuredElements, m_structuredElementIndices, source);
}

IRInst* NVVMAddressPlan::getRoot(IRInst* source) const
{
    if (auto field = findFieldAddress(source))
    {
        SLANG_RELEASE_ASSERT(field->root);
        return field->root;
    }
    if (auto element = findElementAddress(source))
    {
        SLANG_RELEASE_ASSERT(element->root);
        return element->root;
    }
    SLANG_RELEASE_ASSERT(
        source && source->getOp() != kIROp_FieldAddress && source->getOp() != kIROp_GetElementPtr);
    return source;
}

const NVVMRawBufferType* NVVMAddressPlan::findRootBuffer(IRInst* source) const
{
    IRInst* root = getRoot(source);
    if (auto element = findStructuredElement(root))
        return &element->bufferType;
    if (auto data = findDataPointer(root))
        return &data->bufferType;
    return nullptr;
}

const NVVMPlannedElementAddress* NVVMAddressPlan::findElementAddress(IRInst* source) const
{
    return _findOperation(m_elementAddresses, m_elementAddressIndices, source);
}

const NVVMPlannedFieldAddress* NVVMEmissionPlanIndex::findFieldAddress(IRInst* source) const
{
    SLANG_RELEASE_ASSERT(m_plan);
    return m_plan->addresses.findFieldAddress(source);
}

const NVVMPlannedElementAddress* NVVMEmissionPlanIndex::findElementAddress(IRInst* source) const
{
    SLANG_RELEASE_ASSERT(m_plan);
    return m_plan->addresses.findElementAddress(source);
}

void NVVMEmissionPlanIndex::initialize(const NVVMEmissionPlan& plan)
{
    SLANG_RELEASE_ASSERT(!m_plan);
    m_plan = &plan;
    _indexOperations(plan.localStorage, m_localStorage);
    _indexOperations(plan.loads, m_loads);
    _indexOperations(plan.stores, m_stores);
    _indexOperations(plan.structuredLoads, m_structuredLoads);
    _indexOperations(plan.aggregateStorageConstructions, m_aggregateStorageConstructions);
    _indexOperations(plan.valueOperations, m_valueOperations);
    _indexOperations(plan.namedIntrinsics, m_namedIntrinsics);
    _indexOperations(plan.traceRays, m_traceRays);
    _indexOperations(plan.uint64WordConstructions, m_uint64WordConstructions);
    _indexOperations(plan.numericTruthinessOperations, m_numericTruthinessOperations);
    _indexOperations(plan.floatingRemainderOperations, m_floatingRemainderOperations);
    _indexOperations(plan.bitfieldOperations, m_bitfieldOperations);
    _indexOperations(plan.resourceBitCasts, m_resourceBitCasts);
    _indexOperations(plan.defaultResourceValues, m_defaultResourceValues);
    _indexOperations(plan.ephemeralValues, m_ephemeralValues);
    _indexOperations(plan.surfaceOperations, m_surfaceOperations);
    _indexOperations(plan.atomicOperations, m_atomicOperations);
}

#define SLANG_NVVM_DEFINE_PLAN_FIND(NAME, TYPE, MEMBER, INDEX_MEMBER) \
    const TYPE* NVVMEmissionPlanIndex::NAME(IRInst* source) const     \
    {                                                                 \
        SLANG_RELEASE_ASSERT(m_plan);                                 \
        return _findOperation(m_plan->MEMBER, INDEX_MEMBER, source);  \
    }

SLANG_NVVM_DEFINE_PLAN_FIND(findLocalStorage, NVVMPlannedLocalStorage, localStorage, m_localStorage)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findNamedIntrinsic,
    NVVMPlannedNamedIntrinsic,
    namedIntrinsics,
    m_namedIntrinsics)
SLANG_NVVM_DEFINE_PLAN_FIND(findTraceRay, NVVMPlannedTraceRay, traceRays, m_traceRays)
SLANG_NVVM_DEFINE_PLAN_FIND(findLoad, NVVMPlannedLoad, loads, m_loads)
SLANG_NVVM_DEFINE_PLAN_FIND(findStore, NVVMPlannedStore, stores, m_stores)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findStructuredLoad,
    NVVMPlannedStructuredLoad,
    structuredLoads,
    m_structuredLoads)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findAggregateStorageConstruction,
    NVVMPlannedAggregateStorageConstruction,
    aggregateStorageConstructions,
    m_aggregateStorageConstructions)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findValueOperation,
    NVVMPlannedValueOperation,
    valueOperations,
    m_valueOperations)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findUInt64WordConstruction,
    NVVMPlannedUInt64WordConstruction,
    uint64WordConstructions,
    m_uint64WordConstructions)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findNumericTruthiness,
    NVVMPlannedNumericTruthiness,
    numericTruthinessOperations,
    m_numericTruthinessOperations)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findFloatingRemainder,
    NVVMPlannedFloatingRemainder,
    floatingRemainderOperations,
    m_floatingRemainderOperations)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findBitfieldOperation,
    NVVMPlannedBitfieldOperation,
    bitfieldOperations,
    m_bitfieldOperations)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findResourceBitCast,
    NVVMPlannedResourceBitCast,
    resourceBitCasts,
    m_resourceBitCasts)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findDefaultResourceValue,
    NVVMPlannedDefaultResourceValue,
    defaultResourceValues,
    m_defaultResourceValues)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findEphemeralValue,
    NVVMPlannedEphemeralValue,
    ephemeralValues,
    m_ephemeralValues)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findSurfaceOperation,
    NVVMPlannedSurfaceOperation,
    surfaceOperations,
    m_surfaceOperations)
SLANG_NVVM_DEFINE_PLAN_FIND(
    findAtomicOperation,
    NVVMPlannedAtomicOperation,
    atomicOperations,
    m_atomicOperations)

#undef SLANG_NVVM_DEFINE_PLAN_FIND

} // namespace Slang
