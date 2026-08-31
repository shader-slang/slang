// slang-ir-early-raytracing-intrinsic-simplification.cpp
#include "slang-ir-early-raytracing-intrinsic-simplification.h"

#include "core/slang-performance-profiler.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
bool isRayTracingLocationOperand(IROp op)
{
    switch (op)
    {
    case kIROp_SPIRVAsmOperandRayPayloadFromLocation:
    case kIROp_SPIRVAsmOperandRayAttributeFromLocation:
    case kIROp_SPIRVAsmOperandRayCallableFromLocation:
        return true;
    default:
        return false;
    }
}

/// Indexes the global ray-tracing objects and functions needed by the location-operand rewrite.
///
/// The maps are keyed by the integer location encoded in a Vulkan decoration. Keeping separate
/// maps for payloads, attributes, and callable data prevents equal numeric locations in different
/// SPIR-V operand roles from being treated as interchangeable.
struct CacheOfDataToReplaceOps
{
    IRModule* module;
    DiagnosticSink* sink;

    Dictionary<int, IRInst*> m_RayLocationToPayloads;
    Dictionary<int, IRInst*> m_RayLocationToAttributes;
    Dictionary<int, IRInst*> m_RayLocationToCallables;

    List<IRInst*> funcsToSearch;

    /// Resolve one location operand to the global object with the corresponding role and location.
    ///
    /// Invalid locations are diagnosed and return an integer recovery value so callers can replace
    /// the malformed operand and allow compilation to continue reporting errors.
    IRInst* getRayVariableFromLocation(IRInst* payloadVariable, Slang::IROp op)
    {
        SLANG_RELEASE_ASSERT(isRayTracingLocationOperand(op));

        IRBuilder builder(payloadVariable);
        IRInst** varLayoutPointsTo = nullptr;
        int intLitValue = -1;
        IRIntLit* intLit = as<IRIntLit>(payloadVariable);
        if (intLit)
        {
            intLitValue = int(intLit->getValue());
            if (kIROp_SPIRVAsmOperandRayPayloadFromLocation == op)
            {
                varLayoutPointsTo = m_RayLocationToPayloads.tryGetValue(intLitValue);
            }
            else if (kIROp_SPIRVAsmOperandRayAttributeFromLocation == op)
            {
                varLayoutPointsTo = m_RayLocationToAttributes.tryGetValue(intLitValue);
            }
            else
            {
                SLANG_RELEASE_ASSERT(kIROp_SPIRVAsmOperandRayCallableFromLocation == op);
                varLayoutPointsTo = m_RayLocationToCallables.tryGetValue(intLitValue);
            }
        }
        else
        {
            sink->diagnose(Diagnostics::ExpectedIntegerConstantNotConstant{
                .location = payloadVariable->sourceLoc});
        }

        IRInst* resultVariable;
        if (!varLayoutPointsTo)
        {
            // if somehow the location tied variable is missing and an error was not thrown by the
            // compiler
            resultVariable = builder.getIntValue(builder.getIntType(), 0);
            sink->diagnose(Diagnostics::ExpectedRayTracingPayloadObjectAtLocationButMissing{
                .payloadLocation = intLitValue,
                .location = payloadVariable->sourceLoc});
        }
        else
        {
            resultVariable = *varLayoutPointsTo;
        }
        return resultVariable;
    }

    /// Build the role-specific location indexes and collect functions that may contain operands.
    void searchForGlobalsDataNeededInPass()
    {
        for (auto i : module->getGlobalInsts())
        {
            switch (i->getOp())
            {
            case kIROp_GlobalParam:
            case kIROp_GlobalVar:
                {
                    for (auto decoration : i->getDecorations())
                    {
                        auto op = decoration->getOp();
                        if (op == kIROp_VulkanRayPayloadDecoration)
                        {
                            m_RayLocationToPayloads.set(
                                int(getIntVal(decoration->getOperand(0))),
                                i);
                        }
                        else if (op == kIROp_VulkanRayPayloadInDecoration)
                        {
                            m_RayLocationToPayloads.set(
                                int(getIntVal(decoration->getOperand(0))),
                                i);
                        }
                        else if (op == kIROp_VulkanHitObjectAttributesDecoration)
                        {
                            m_RayLocationToAttributes.set(
                                int(getIntVal(decoration->getOperand(0))),
                                i);
                        }
                        else if (op == kIROp_VulkanCallablePayloadDecoration)
                        {
                            m_RayLocationToCallables.set(
                                int(getIntVal(decoration->getOperand(0))),
                                i);
                        }
                        else if (op == kIROp_VulkanCallablePayloadInDecoration)
                        {
                            m_RayLocationToCallables.set(
                                int(getIntVal(decoration->getOperand(0))),
                                i);
                        }
                    }
                    break;
                }
            case kIROp_Func:
                {
                    funcsToSearch.add(i);
                    break;
                }
            }
        }
    }

    CacheOfDataToReplaceOps(IRModule* module, DiagnosticSink* sink)
    {
        this->module = module;
        this->sink = sink;
    }
};

/// Replace location operands nested under SPIR-V assembly blocks in one function subtree.
void recurseInFuncForOpsToReplace(IRInst* parent, CacheOfDataToReplaceOps* cache)
{

    if (as<IRSPIRVAsm>(parent))
    {
        for (auto i : parent->getChildren())
        {
            if (isRayTracingLocationOperand(i->getOp()))
            {
                auto op = i->getOperand(0);
                IRInst* globalVar = cache->getRayVariableFromLocation(op, i->getOp());
                auto builder = IRBuilder(i);
                builder.setInsertBefore(i);
                auto spirvASM = builder.emitSPIRVAsmOperandInst(globalVar);
                i->replaceUsesWith(spirvASM);
                i->removeAndDeallocate();
            }
        }
    }

    for (auto i : parent->getChildren())
        recurseInFuncForOpsToReplace(i, cache);
}

/// Apply the location-operand rewrite to every function collected from the module.
void recurseAllOpsToReplace(CacheOfDataToReplaceOps* cache)
{
    for (auto func : cache->funcsToSearch)
    {
        recurseInFuncForOpsToReplace(func, cache);
    }
}

void replaceLocationIntrinsicsWithRaytracingObject(IRModule* module, DiagnosticSink* sink)
{
    // The GLSL-specific SPIR-V assembly operands and Vulkan decorations in the IR are the source
    // of truth for whether this rewrite applies. The caller's required-pass set avoids invoking
    // this traversal when the module contains no location operands to replace.
    CacheOfDataToReplaceOps cache = CacheOfDataToReplaceOps(module, sink);
    cache.searchForGlobalsDataNeededInPass();
    recurseAllOpsToReplace(&cache);
}
} // namespace Slang
