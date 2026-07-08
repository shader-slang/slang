// slang-ir-check-optional-usage.cpp
#include "slang-ir-check-optional-usage.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static void checkForInvalidOptionalUsage(IRFunc* func, DiagnosticSink* sink)
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
            else if (auto noneInst = as<IRMakeOptionalNone>(inst))
            {
                // A `none` whose (possibly nested) Optional payload is opaque cannot be
                // lowered; the front-end guard E30902 misses it when the payload is a
                // still-abstract generic parameter, so diagnose here post-specialization.
                if (auto optType = as<IROptionalType>(noneInst->getDataType()))
                {
                    IRType* valueType = optType->getValueType();
                    while (auto innerOptional = as<IROptionalType>(valueType))
                        valueType = innerOptional->getValueType();
                    if (isOpaqueType(valueType, nullptr))
                    {
                        sink->diagnose(Diagnostics::OptionalCannotWrapResourceTypeIr{
                            .type = valueType,
                            .location = noneInst->sourceLoc,
                        });
                    }
                }
            }
        }
    }
}

void checkForInvalidOptionalUsage(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_Func:
            checkForInvalidOptionalUsage(as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkForInvalidOptionalUsage(innerFunc, sink);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
