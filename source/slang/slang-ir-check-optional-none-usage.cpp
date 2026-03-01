// slang-ir-check-optional-none-usage.cpp
#include "slang-ir-check-optional-none-usage.h"

#include "slang-ir-util.h"

namespace Slang
{

void checkForOptionalNoneUsage(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(globalInst))
        {
            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    if (inst->getOp() == kIROp_GetOptionalValue &&
                        inst->getOperand(0)->getOp() == kIROp_MakeOptionalNone)
                    {
                        sink->diagnose(
                            inst->sourceLoc,
                            Diagnostics::accessingValueOfNoneOptional,
                            inst->getDataType());
                    }
                }
            }
        }
    }
}

} // namespace Slang
