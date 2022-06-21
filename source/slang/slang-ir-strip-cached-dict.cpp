// slang-ir-strip-cached-dict.cpp
#include "slang-ir-strip-cached-dict.h"
#include "slang-ir-insts.h"

namespace Slang
{

void stripCachedDictionaries(IRModule* module)
{
    for (auto inst : module->getGlobalInsts())
    {
        switch (inst->getOp())
        {
        case kIROp_GenericSpecializationDictionary:
        case kIROp_ExistentialFuncSpecializationDictionary:
        case kIROp_ExistentialTypeSpecializationDictionary:
            inst->removeAndDeallocate();
            break;
        default:
            continue;
        }
    }
}

}
