#include "slang-ir-legalize-image-subscript.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-parameter-binding.h"
#include "slang-ir-legalize-varying-params.h"

namespace Slang
{
    void legalizeIsTextureAccess(IRModule* module)
    {
        IRBuilder builder(module);
        for (auto globalInst : module->getModuleInst()->getChildren())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func)
                continue;
            for (auto block : func->getBlocks())
            {
                auto inst = block->getFirstInst();
                IRInst* next;
                for ( ; inst; inst = next)
                {
                    next = inst->getNextInst();
                    switch (inst->getOp())
                    {
                    case kIROp_IsTextureAccess:
                        if (as<IRImageSubscript>(inst->getOperand(0)))
                            inst->replaceUsesWith(builder.getBoolValue(true));
                        else
                            inst->replaceUsesWith(builder.getBoolValue(false));
                        inst->removeAndDeallocate();
                        continue;
                    }
                }   
            }
        }
    }
}

