#include "slang-ir-insts.h"

namespace Slang
{
void IRDeduplicationContext::init(IRModule* module)
{
    m_module = module;
    m_session = module->getSession();

    m_globalValueNumberingMap.clear();
    m_scopedGlobalValueNumberingMap.clear();
    m_scopedGlobalValueNumberingScopes.clear();
    m_constantMap.clear();
}

bool IRDeduplicationContext::isInstVisibleFromGenericScope(IRInst* inst, IRInst* genericScope)
{
    auto instGeneric = findOuterGeneric(inst);
    return !instGeneric || instGeneric == genericScope ||
           (genericScope && hasDescendent(instGeneric, genericScope));
}

IRInst* IRDeduplicationContext::findVisibleScopedGlobalNumberingEntry(
    IRInstKey const& key,
    IRInst* genericScope)
{
    IRInst* scopedInst = nullptr;
    for (auto generic = genericScope; generic; generic = findOuterGeneric(generic))
    {
        if (m_scopedGlobalValueNumberingMap.tryGetValue(IRScopedInstKey{key, generic}, scopedInst))
            return scopedInst;
    }
    if (m_scopedGlobalValueNumberingMap.tryGetValue(IRScopedInstKey{key, nullptr}, scopedInst))
        return scopedInst;
    return nullptr;
}

IRInst* IRDeduplicationContext::findVisibleGlobalNumberingEntry(
    IRInstKey const& key,
    IRInst* genericScope)
{
    IRInst* existingVal = nullptr;
    if (!m_globalValueNumberingMap.tryGetValue(key, existingVal))
        return nullptr;

    if (isInstVisibleFromGenericScope(existingVal, genericScope))
        return existingVal;

    return findVisibleScopedGlobalNumberingEntry(key, genericScope);
}

void IRDeduplicationContext::_addScopedGlobalNumberingEntry(IRInst* inst)
{
    auto scope = findOuterGeneric(inst);
    m_scopedGlobalValueNumberingMap[IRScopedInstKey{IRInstKey{inst}, scope}] = inst;
    m_scopedGlobalValueNumberingScopes[inst] = scope;
}

void IRDeduplicationContext::_removeScopedGlobalNumberingEntry(IRInst* inst)
{
    IRInst* scope = nullptr;
    if (!m_scopedGlobalValueNumberingScopes.tryGetValue(inst, scope))
        return;

    IRInst* value = nullptr;
    IRScopedInstKey scopedKey{IRInstKey{inst}, scope};
    if (m_scopedGlobalValueNumberingMap.tryGetValue(scopedKey, value) && value == inst)
        m_scopedGlobalValueNumberingMap.remove(scopedKey);

    m_scopedGlobalValueNumberingScopes.remove(inst);
}

void IRDeduplicationContext::removeHoistableInstFromGlobalNumberingMap(IRInst* instToRemove)
{
    InstHashSet userWorkListSet(instToRemove->getModule());
    InstWorkList userWorkList(instToRemove->getModule());
    auto addToWorkList = [&](IRInst* i)
    {
        if (userWorkListSet.add(i))
            userWorkList.add(i);
    };
    addToWorkList(instToRemove);
    for (Index i = 0; i < userWorkList.getCount(); i++)
    {
        auto inst = userWorkList[i];
        if (getIROpInfo(inst->getOp()).isHoistable())
        {
            _removeGlobalNumberingEntry(inst);
            for (auto use = inst->firstUse; use; use = use->nextUse)
            {
                addToWorkList(use->getUser());
            }
        }
    }
}

void IRDeduplicationContext::removeInstFromConstantMap(IRInst* inst)
{
    // m_constantMap stores an IRConstantKey{constInst} -> constInst.
    // If we are about to remove `inst` from this map, we need to make sure
    // inst is actually the value stored in the map, and not some temp/duplicate
    // constant val that hashes to the same key.
    //
    IRConstant* constInst = as<IRConstant>(inst);
    if (!constInst)
        return;
    IRConstantKey key;
    key.inst = constInst;
    IRConstant* existing = nullptr;
    if (m_constantMap.tryGetValue(key, existing))
    {
        // Only remove if it's the same instance.
        if (existing == constInst)
        {
            m_constantMap.remove(key);
        }
    }
}

void IRDeduplicationContext::tryHoistInst(IRInst* inst)
{
    auto module = inst->getModule();
    SLANG_ASSERT(module);
    InstWorkList workList(module);

    // Note: It is possible to see the same inst multiple times since it could
    // reference a hoistable inst through multiple paths. For now, we will essentially
    // re-hoist it each time, though we could be more efficient here.
    //
    workList.add(inst);
    IRBuilder builder(module);

    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto item = workList[i];

        // Does inst no longer depend on anything defined locally?
        // If so we should hoist it.
        bool shouldHoist = (item->getOperandCount() > 0);

        if (item->getFullType())
        {
            auto typeParent = item->getFullType()->getParent();
            if (typeParent == item->getParent())
                shouldHoist = false;
        }

        for (UInt a = 0; a < item->getOperandCount(); a++)
        {
            auto opParent = item->getOperand(a)->getParent();
            if (opParent == item->getParent())
            {
                shouldHoist = false;
                break;
            }
        }

        // Hoisting this inst
        if (shouldHoist)
        {
            item->removeFromParent();
            addHoistableInst(&builder, item);

            // Continue to consider all users for hoisting.
            for (auto use = item->firstUse; use; use = use->nextUse)
            {
                if (getIROpInfo(use->getUser()->getOp()).isHoistable())
                {
                    workList.add(use->getUser());
                }
            }
        }
    }
}
} // namespace Slang
