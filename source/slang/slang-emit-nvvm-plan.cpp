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

void NVVMEmissionPlanIndex::initialize(const NVVMEmissionPlan& plan)
{
    SLANG_RELEASE_ASSERT(!m_plan);
    m_plan = &plan;
    _indexOperations(plan.valueOperations, m_valueOperations);
    _indexOperations(plan.uint64WordConstructions, m_uint64WordConstructions);
    _indexOperations(plan.numericTruthinessOperations, m_numericTruthinessOperations);
    _indexOperations(plan.floatingRemainderOperations, m_floatingRemainderOperations);
    _indexOperations(plan.bitfieldOperations, m_bitfieldOperations);
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
