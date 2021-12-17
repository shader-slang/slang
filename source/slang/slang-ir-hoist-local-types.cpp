#include "slang-ir-hoist-local-types.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
struct HoistLocalTypesContext
{
    IRModule* module;
    DiagnosticSink* sink;

    SharedIRBuilder sharedBuilderStorage;

    List<IRInst*> workList;
    HashSet<IRInst*> workListSet;

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.Contains(inst))
            return;

        workList.add(inst);
        workListSet.Add(inst);
    }

    void processInst(IRInst* inst)
    {
        auto sharedBuilder = &sharedBuilderStorage;
        if (!as<IRType>(inst))
            return;
        if (inst->getParent() == module->getModuleInst())
            return;
        IRInstKey key = {inst};
        if (auto value = sharedBuilder->getGlobalValueNumberingMap().TryGetValue(key))
        {
            inst->replaceUsesWith(*value);
            inst->removeAndDeallocate();
            return;
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
        }
    }

    void processModule()
    {
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);

        // Deduplicate equivalent types and build numbering map for global types.
        sharedBuilder->deduplicateAndRebuildGlobalNumberingMap();

        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.Remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }
    }
};

void hoistLocalTypes(IRModule* module, DiagnosticSink* sink)
{
    HoistLocalTypesContext context;
    context.module = module;
    context.sink = sink;
    context.processModule();
}

} // namespace Slang
