#include "slang-ir-augment-make-existential.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
struct AugmentMakeExistentialContext
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

    void processMakeExistential(IRMakeExistential* inst)
    {
        IRBuilder builderStorage;
        auto builder = &builderStorage;
        builder->sharedBuilder = &sharedBuilderStorage;
        builder->setInsertBefore(inst);

        auto augInst = builder->emitMakeExistentialWithRTTI(
            inst->getFullType(),
            inst->getWrappedValue(),
            inst->getWitnessTable(),
            inst->getWrappedValue()->getDataType());
        inst->replaceUsesWith(augInst);
        inst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_MakeExistential:
            processMakeExistential((IRMakeExistential*)inst);
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->module = module;
        sharedBuilder->session = module->session;

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

void augmentMakeExistentialInsts(IRModule* module)
{
    AugmentMakeExistentialContext context;
    context.module = module;
    context.processModule();
}
} // namespace Slang
