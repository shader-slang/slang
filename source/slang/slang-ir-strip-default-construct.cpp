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

        // TODO: Consider splitting this into a dedicated re-materialization
        // pass, separate from the pure "strip raw default constructors"
        // behavior above.
        IRBuilder builder(module);
        for (auto defaultConstruct : defaultConstructsToReEmit)
        {
            builder.setInsertBefore(defaultConstruct);

            // Re-emit the default construct in materialized form (e.g.
            // MakeStruct/MakeArray/constant) when possible. For types that
            // cannot yet be materialized at this stage, this may still fall
            // back to raw `DefaultConstruct`, matching existing behavior.
            IRInst* replacement =
                builder.emitDefaultConstruct(defaultConstruct->getDataType());
            if (replacement)
            {
                defaultConstruct->replaceUsesWith(replacement);
                defaultConstruct->removeAndDeallocate();
            }
        }
    }
};

void removeRawDefaultConstructors(IRModule* module)
{
    RemoveDefaultConstructInsts(module).processModule();
}

} // namespace Slang
