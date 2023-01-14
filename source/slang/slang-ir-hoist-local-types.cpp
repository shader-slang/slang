#include "slang-ir-hoist-local-types.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
struct HoistLocalTypesContext
{
    IRModule* module;

    SharedIRBuilder sharedBuilderStorage;

    List<IRInst*> workList;
    HashSet<IRInst*> workListSet;

    void addToWorkList(IRInst* inst)
    {
        if (workListSet.Contains(inst))
            return;

        workList.add(inst);
        workListSet.Add(inst);
    }

    bool processInst(IRInst* inst)
    {
        auto sharedBuilder = &sharedBuilderStorage;
        if (!as<IRType>(inst))
            return false;
        if (inst->getParent() == module->getModuleInst())
            return false;
        switch (inst->getOp())
        {
        case kIROp_InterfaceType:
        case kIROp_StructType:
        case kIROp_ClassType:
            return false;
        default:
            break;
        }
        IRInstKey key = {inst};
        if (auto value = sharedBuilder->getGlobalValueNumberingMap().TryGetValue(key))
        {
            inst->replaceUsesWith(*value);
            inst->removeAndDeallocate();
            return true;
        }
        IRBuilder builder(sharedBuilder);
        builder.setInsertInto(module->getModuleInst());
        bool hoistable = true;
        ShortList<IRInst*> mappedOperands;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            IRInstKey opKey = {inst->getOperand(i)};
            if (auto value = sharedBuilder->getGlobalValueNumberingMap().TryGetValue(opKey))
            {
                mappedOperands.add(*value);
            }
            else
            {
                hoistable = false;
                break;
            }
        }
        if (hoistable)
        {
            auto newType = builder.getType(
                inst->getOp(), mappedOperands.getCount(), mappedOperands.getArrayView().getBuffer());
            inst->transferDecorationsTo(newType);
            inst->replaceUsesWith(newType);
            inst->removeAndDeallocate();
            return true;
        }
        return false;
    }

    void processModule()
    {
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);

        for (;;)
        {
            bool changed = false;
            // Deduplicate equivalent types and build numbering map for global types.
            sharedBuilder->deduplicateAndRebuildGlobalNumberingMap();

            addToWorkList(module->getModuleInst());

            while (workList.getCount() != 0)
            {
                IRInst* inst = workList.getLast();

                workList.removeLast();
                workListSet.Remove(inst);

                changed |= processInst(inst);

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
            }

            if (!changed)
                break;
        }
    }
};

void hoistLocalTypes(IRModule* module)
{
    HoistLocalTypesContext context;
    context.module = module;
    context.processModule();
}

} // namespace Slang
