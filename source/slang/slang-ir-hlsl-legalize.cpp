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

// A `groupshared` parameter that reaches this point belongs to a function the inliner could not
// remove, so the function keeps a call boundary. We check the same functions the HLSL emitter
// would write out: definitions that are not target intrinsics. A mesh-shader input payload shares
// the group-shared rate but crosses the boundary legitimately through HLSL's `in payload` syntax,
// so it is excluded.
void validateGroupSharedParamsForHLSL(
    IRModule* module,
    CapabilitySet const& targetCaps,
    DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func || !func->isDefinition())
            continue;
        UnownedStringSlice intrinsicDef;
        IRInst* intrinsicInst;
        if (findTargetIntrinsicDefinition(func, targetCaps, intrinsicDef, intrinsicInst))
            continue;
        for (auto param : func->getParams())
        {
            if (as<IRGroupSharedRate>(param->getRate()) &&
                !param->findDecoration<IRHLSLMeshPayloadDecoration>())
            {
                sink->diagnose(Diagnostics::GroupsharedParameterNotAllowedOnHlslWithBoundary{
                    .location = param->sourceLoc});
            }
        }
    }
}

} // namespace Slang
