// slang-ir-init-local-var.cpp
#include "slang-ir-init-local-var.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

void initializeLocalVariables(SharedIRBuilder* sharedBuilder, IRGlobalValueWithCode* func)
{
    IRBuilder builder(sharedBuilder);
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_Var)
            {
                auto firstUse = inst->firstUse;
                bool initialized =
                    (firstUse && firstUse->getUser()->getOp() == kIROp_Store &&
                        firstUse->getUser()->getParent() == inst->getParent());
                if (initialized)
                    continue;
                builder.setInsertAfter(inst);
                builder.emitStore(
                    inst,
                    builder.emitDefaultConstruct(
                        as<IRPtrTypeBase>(inst->getFullType())->getValueType()));
            }
        }
    }
}

} // namespace Slang
