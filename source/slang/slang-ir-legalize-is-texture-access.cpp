#include "slang-ir-legalize-is-texture-access.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-parameter-binding.h"
#include "slang-ir-legalize-image-subscript.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-sccp.h"

namespace Slang
{
    IRImageSubscript* getTextureAccess(IRInst* inst)
    {
        return as<IRImageSubscript>(getRootAddr(inst->getOperand(0)));
    }

    void legalizeIsTextureAccess(IRModule* module, DiagnosticSink* sink)
    {
        HashSet<IRFunc*> functionsToSCCP;
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
                        if (getTextureAccess(inst))
                            inst->replaceUsesWith(builder.getBoolValue(true));
                        else
                            inst->replaceUsesWith(builder.getBoolValue(false));
                        inst->removeAndDeallocate();
                        functionsToSCCP.add(func);
                        continue;
                    case kIROp_IsTextureArrayAccess:
                    {
                        auto textureAccess = getTextureAccess(inst);
                        if (textureAccess && as<IRTextureType>(textureAccess->getImage()->getDataType())->isArray())
                            inst->replaceUsesWith(builder.getBoolValue(true));
                        else
                            inst->replaceUsesWith(builder.getBoolValue(false));
                        inst->removeAndDeallocate();
                        functionsToSCCP.add(func);
                        continue;
                    }
                    case kIROp_IsTextureScalarAccess:
                    {
                        auto textureAccess = getTextureAccess(inst);
                        if (textureAccess && !as<IRVectorType>(as<IRTextureType>(textureAccess->getImage()->getDataType())->getElementType()))
                            inst->replaceUsesWith(builder.getBoolValue(true));
                        else
                            inst->replaceUsesWith(builder.getBoolValue(false));
                        inst->removeAndDeallocate();
                        functionsToSCCP.add(func);
                        continue;
                    }
                    }
                }   
            }
        }
        // Requires a SCCP to ensure Slang does not evaluate 'IRTextureType' code path
        // and unresolved 'isTextureAccess' operations for when 'inst' is not a
        // 'IRTextureType'/`TextureAccessor`
        for (auto func : functionsToSCCP)
            applySparseConditionalConstantPropagation(func, sink);
    }
}

