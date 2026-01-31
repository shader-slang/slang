// slang-ir-operator-shift-overflow.cpp
#include "slang-ir-mesh-output-reads.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir.h"
#include "slang.h"

namespace Slang
{

class DiagnosticSink;
struct IRModule;

void checkForMeshOutputReadsRecursive(IRInst* inst, DiagnosticSink* sink)
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
                    if (as<IRMeshOutputRef>(as<IRLoad>(opInst)->getPtr()))
                    {
                        sink->diagnose(opInst, Diagnostics::attemptToReadFromMeshShaderOutput);
                    }
                    break;
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
