// slang-ir-check-optional-none-usage.cpp
#include "slang-ir-check-optional-none-usage.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static void checkForOptionalNoneUsage(IRFunc* func, DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_GetOptionalValue &&
                inst->getOperand(0)->getOp() == kIROp_MakeOptionalNone)
            {
                sink->diagnose(Diagnostics::AccessingValueOfNoneOptional{
                    .type = inst->getDataType(),
                    .location = inst->sourceLoc,
                });
            }
        }
    }
}

void checkForOptionalNoneUsage(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_Func:
            checkForOptionalNoneUsage(as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkForOptionalNoneUsage(innerFunc, sink);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
