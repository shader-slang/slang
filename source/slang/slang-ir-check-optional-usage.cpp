// slang-ir-check-optional-usage.cpp
#include "slang-ir-check-optional-usage.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static void checkForInvalidOptionalUsage(
    IRFunc* func,
    DiagnosticSink* sink,
    bool runNonEssentialValidation)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (runNonEssentialValidation && inst->getOp() == kIROp_GetOptionalValue &&
                inst->getOperand(0)->getOp() == kIROp_MakeOptionalNone)
            {
                sink->diagnose(Diagnostics::AccessingValueOfNoneOptional{
                    .type = inst->getDataType(),
                    .location = inst->sourceLoc,
                });
            }
            else if (auto noneInst = as<IRMakeOptionalNone>(inst))
            {
                if (auto optType = as<IROptionalType>(noneInst->getDataType()))
                {
                    if (isOpaqueType(optType->getValueType(), nullptr))
                    {
                        sink->diagnose(Diagnostics::OptionalCannotWrapResourceTypeIr{
                            .type = optType->getValueType(),
                            .location = noneInst->sourceLoc,
                        });
                    }
                }
            }
        }
    }
}

void checkForInvalidOptionalUsage(
    IRModule* module,
    DiagnosticSink* sink,
    bool runNonEssentialValidation)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_Func:
            checkForInvalidOptionalUsage(as<IRFunc>(globalInst), sink, runNonEssentialValidation);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkForInvalidOptionalUsage(innerFunc, sink, runNonEssentialValidation);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
