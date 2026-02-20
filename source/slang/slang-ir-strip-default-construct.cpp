// slang-ir-strip-default-construct.cpp
#include "slang-ir-strip-default-construct.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct RemoveDefaultConstructInsts : InstPassBase
{
    RemoveDefaultConstructInsts(IRModule* module)
        : InstPassBase(module)
    {
    }
    void processModule()
    {
        List<IRInst*> structDefaultConstructs;
        processInstsOfType<IRDefaultConstruct>(
            kIROp_DefaultConstruct,
            [&](IRDefaultConstruct* defaultConstruct)
            {
                List<IRInst*> instsToRemove;
                for (auto use = defaultConstruct->firstUse; use; use = use->nextUse)
                {
                    if (as<IRStore>(use->getUser()))
                        instsToRemove.add(use->getUser());
                    else
                    {
                        if (as<IRStructType>(defaultConstruct->getDataType()))
                            structDefaultConstructs.add(defaultConstruct);
                        return; // Ignore this inst if there are non-store
                                // uses.
                    }
                }

                for (auto inst : instsToRemove)
                    inst->removeAndDeallocate();

                defaultConstruct->removeAndDeallocate();
            });

        // TODO: clean this up (should either rename the pass, or put this in its own pass)
        // or preferably make sure that when intermediate context types are turned into structs,
        // they have the defualt construct re-emitted.
        //
        IRBuilder builder(module);
        for (auto structDefaultConstruct : structDefaultConstructs)
        {
            builder.setInsertBefore(structDefaultConstruct);

            // Re-emit the default construct, which will create a MakeStruct instead.
            structDefaultConstruct->replaceUsesWith(
                builder.emitDefaultConstruct(structDefaultConstruct->getDataType()));
        }
    }
};

void removeRawDefaultConstructors(IRModule* module)
{
    RemoveDefaultConstructInsts(module).processModule();
}

} // namespace Slang
