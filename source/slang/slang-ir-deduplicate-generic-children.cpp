#include "slang-ir-deduplicate-generic-children.h"

#include "slang-ir-clone.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

bool deduplicateGenericChildren(IRGeneric* genericInst)
{
    bool changed = false;
    GenericChildrenMigrationContext ctx;
    ctx.init(genericInst, genericInst, nullptr);
    List<IRInst*> instsToRemove;
    for (auto inst = genericInst->getFirstBlock()->getFirstInst(); inst; inst = inst->getNextInst())
    {
        auto deduped = ctx.deduplicate(inst);
        if (deduped != inst)
        {
            inst->replaceUsesWith(deduped);
            instsToRemove.add(inst);
            changed = true;
        }
    }
    for (auto inst : instsToRemove)
        inst->removeAndDeallocate();
    return changed;
}

bool deduplicateGenericChildren(IRModule* module)
{
    bool changed = false;
    for (auto gen : module->getGenerics())
    {
        changed |= deduplicateGenericChildren(as<IRGeneric>(gen));
    }
    return changed;
}

} // namespace Slang
