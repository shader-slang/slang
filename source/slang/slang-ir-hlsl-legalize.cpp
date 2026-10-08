// slang-ir-hlsl-legalize.cpp
#include "slang-ir-hlsl-legalize.h"

#include "slang-ir-insts.h"
#include "slang-ir-util-hlsl.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static String getBarrierFlagValueString(uint32_t flagVal)
{
    StringBuilder sb;
    sb << "0x" << String(flagVal, 16);
    return sb.produceString();
}

static void validateBarrierFlagsForHLSLInst(IRInst* inst, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_GetEnumBarrierMemoryTypeFlags:
        {
            auto intLit = cast<IRIntLit>(getBarrierFlagValueInst(inst->getOperand(0)));
            auto rawFlagVal = getIntVal(intLit);
            auto flagVal = (uint32_t)rawFlagVal;
            if (!isValidBarrierMemoryTypeFlags(flagVal))
            {
                sink->diagnose(Diagnostics::InvalidBarrierMemoryTypeFlagsValue{
                    .value = getBarrierFlagValueString(flagVal),
                    .location = inst->sourceLoc});
            }
            break;
        }
    case kIROp_GetEnumBarrierSemanticFlags:
        {
            auto intLit = cast<IRIntLit>(getBarrierFlagValueInst(inst->getOperand(0)));
            auto rawFlagVal = getIntVal(intLit);
            auto flagVal = (uint32_t)rawFlagVal;
            if (!isValidBarrierSemanticFlags(flagVal))
            {
                sink->diagnose(Diagnostics::InvalidBarrierSemanticFlagsValue{
                    .value = getBarrierFlagValueString(flagVal),
                    .location = inst->sourceLoc});
            }
            break;
        }
    default:
        break;
    }

    for (auto child : inst->getChildren())
        validateBarrierFlagsForHLSLInst(child, sink);
}

static void validateBarrierFlagsForHLSLFunc(IRFunc* func, DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
            validateBarrierFlagsForHLSLInst(inst, sink);
    }
}

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_GetEnumBarrierMemoryTypeFlags:
        case kIROp_GetEnumBarrierSemanticFlags:
            validateBarrierFlagsForHLSLInst(globalInst, sink);
            break;
        case kIROp_Func:
            validateBarrierFlagsForHLSLFunc(as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            if (auto innerFunc = as<IRFunc>(findGenericReturnVal(as<IRGeneric>(globalInst))))
                validateBarrierFlagsForHLSLFunc(innerFunc, sink);
            break;
        default:
            break;
        }
    }
}

} // namespace Slang
