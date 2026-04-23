// slang-ir-mesh-output-reads.cpp
#include "slang-ir-mesh-output-reads.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang.h"

namespace Slang
{

/// Returns true if `inst` is an IRMeshOutputRef or is derived from one via
/// chains of GetElementPtr or FieldAddress instructions.
static bool isDerivedFromMeshOutputRef(IRInst* inst)
{
    for (;;)
    {
        if (as<IRMeshOutputRef>(inst))
            return true;
        if (auto gep = as<IRGetElementPtr>(inst))
            inst = gep->getBase();
        else if (auto fa = as<IRFieldAddress>(inst))
            inst = fa->getBase();
        else
            return false;
    }
}

static void checkForMeshOutputReadsRecursive(IRInst* inst, DiagnosticSink* sink)
{
    if (auto code = as<IRGlobalValueWithCode>(inst))
    {
        for (auto block : code->getBlocks())
        {
            for (auto opInst : block->getChildren())
            {
                switch (opInst->getOp())
                {
                case kIROp_Load:
                    if (isDerivedFromMeshOutputRef(as<IRLoad>(opInst)->getPtr()))
                    {
                        sink->diagnose(
                            Diagnostics::AttemptToReadFromMeshShaderOutput{.inst = opInst});
                    }
                    break;

                case kIROp_Call:
                    {
                        // Check for IRMeshOutputRef-derived pointers being passed as
                        // non-output reference parameters (which implies a read).
                        auto call = as<IRCall>(opInst);
                        auto calleeType = as<IRFuncType>(call->getCallee()->getFullType());
                        if (!calleeType)
                            break;
                        UInt argCount = call->getArgCount();
                        for (UInt i = 0; i < argCount && i < calleeType->getParamCount(); ++i)
                        {
                            // Out-only parameters won't read from the argument.
                            if (calleeType->getParamType(i)->getOp() == kIROp_OutParamType)
                                continue;
                            if (isDerivedFromMeshOutputRef(call->getArg(i)))
                            {
                                sink->diagnose(
                                    Diagnostics::InvalidParameterPassingModeForWriteOnlyReference{
                                        .location = opInst->sourceLoc});
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    for (auto childInst : inst->getChildren())
    {
        checkForMeshOutputReadsRecursive(childInst, sink);
    }
}

void checkForMeshOutputReads(IRModule* module, DiagnosticSink* sink)
{
    checkForMeshOutputReadsRecursive(module->getModuleInst(), sink);
}

} // namespace Slang
