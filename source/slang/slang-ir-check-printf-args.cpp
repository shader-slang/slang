// slang-ir-check-printf-args.cpp
#include "slang-ir-check-printf-args.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static void checkPrintfArgumentTypes(IRFunc* func, DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() != kIROp_Printf)
                continue;

            ShortList<IRInst*> payloadArgs;
            collectFlattenedVariadicOperands(inst, 1, payloadArgs);
            for (auto arg : payloadArgs)
            {
                IRType* opaqueType = nullptr;
                if (isOpaqueType(arg->getDataType(), &opaqueType))
                {
                    sink->diagnose(Diagnostics::PrintfArgumentTypeNotSupported{
                        .type = opaqueType,
                        .location = inst->sourceLoc});
                }
            }
        }
    }
}

void checkPrintfArgumentTypes(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(globalInst))
            checkPrintfArgumentTypes(func, sink);
    }
}

} // namespace Slang
