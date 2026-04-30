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
        List<IRInst*> defaultConstructsToReEmit;
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
                        // Re-emit any `DefaultConstruct` that has a non-store
                        // use, regardless of its data type. For struct/array
                        // types this expands to `MakeStruct`/`MakeArray`; for
                        // primitive types it produces an `IRConstant` zero.
                        // Either way, downstream codegen can handle the result
                        // without requiring native `DefaultConstruct` support.
                        defaultConstructsToReEmit.add(defaultConstruct);
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
        for (auto defaultConstruct : defaultConstructsToReEmit)
        {
            builder.setInsertBefore(defaultConstruct);

            // Re-emit the default construct, which will create a MakeStruct or
            // primitive constant value instead.
            defaultConstruct->replaceUsesWith(
                builder.emitDefaultConstruct(defaultConstruct->getDataType()));
        }
    }
};

void removeRawDefaultConstructors(IRModule* module)
{
    RemoveDefaultConstructInsts(module).processModule();
}

} // namespace Slang
