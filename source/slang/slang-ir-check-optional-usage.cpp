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
                // A `none` of an opaque-payload Optional cannot be lowered (E30902's
                // front-end guard misses it when the payload is a still-abstract generic
                // parameter). This runs unconditionally to prevent an unlowerable
                // `defaultConstruct` from reaching the backend as an internal error.
                if (auto optType = as<IROptionalType>(noneInst->getDataType()))
                {
                    IRType* opaqueLeaf = nullptr;
                    if (isOpaqueType(optType->getValueType(), &opaqueLeaf))
                    {
                        sink->diagnose(Diagnostics::OptionalCannotWrapResourceTypeIr{
                            .type = opaqueLeaf,
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
