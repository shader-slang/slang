#include "slang-ir-remove-unused-generic-param.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
struct RemoveUnusedGenericParamContext : InstPassBase
{
    RemoveUnusedGenericParamContext(IRModule* inModule)
        : InstPassBase(inModule)
    {}

    bool processModule()
    {
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);
        sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
        IRBuilder builder(sharedBuilder);
        bool changed = false;
        for (auto inst : module->getModuleInst()->getChildren())
        {
            if (auto genInst = as<IRGeneric>(inst))
            {
                List<UInt> paramToPreserve;
                UInt id = 0;
                for (auto param : genInst->getParams())
                {
                    if (param->hasUses())
                    {
                        paramToPreserve.add(id);
                    }
                    id++;
                }
                if (paramToPreserve.getCount() == (Index)id)
                    continue;
                changed = true;
                if (paramToPreserve.getCount() == 0)
                {
                    // Special case: the generic return value is not dependent on the generic param,
                    // we can hoist to global scope safely.
                    IRInst* returnVal = nullptr;
                    for (auto child = genInst->getFirstBlock()->getFirstOrdinaryInst(); child; )
                    {
                        auto next = child->getNextInst();
                        if (child->getOp() == kIROp_Return)
                        {
                            returnVal = child->getOperand(0);
                            break;
                        }
                        child->insertBefore(genInst);
                        child = next;
                    }
                    SLANG_ASSERT(returnVal);
                    List<IRUse*> uses;
                    for (auto use = genInst->firstUse; use; use = use->nextUse)
                        uses.add(use);
                    for (auto use : uses)
                    {
                        if (use->getUser()->getOp() == kIROp_Specialize &&
                            use == use->getUser()->getOperands())
                        {
                            use->getUser()->replaceUsesWith(returnVal);
                        }
                    }
                    genInst->replaceUsesWith(returnVal);
                    genInst->removeAndDeallocate();
                }
                else
                {
                    // General case: remove unnecessary specialization arguments.
                    List<IRUse*> uses;
                    for (auto use = genInst->firstUse; use; use = use->nextUse)
                        uses.add(use);
                    for (auto use : uses)
                    {
                        if (use->getUser()->getOp() == kIROp_Specialize &&
                            use == use->getUser()->getOperands())
                        {
                            auto specialize = as<IRSpecialize>(use->getUser());
                            builder.setInsertBefore(specialize);
                            List<IRInst*> newArgs;
                            for (auto i : paramToPreserve)
                                newArgs.add(specialize->getArg(i));
                            auto newSpecialize = builder.emitSpecializeInst(
                                specialize->getFullType(),
                                specialize->getBase(),
                                newArgs.getCount(),
                                newArgs.getBuffer());
                            specialize->transferDecorationsTo(newSpecialize);
                            specialize->replaceUsesWith(newSpecialize);
                            specialize->removeAndDeallocate();
                        }
                    }
                }
            }
        }
        return changed;
    }
};

bool removeUnusedGenericParam(IRModule* module)
{
    RemoveUnusedGenericParamContext context = RemoveUnusedGenericParamContext(module);
    return context.processModule();
}

} // namespace Slang
