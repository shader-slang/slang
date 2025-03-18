// ir-missing-return.cpp
#include "slang-ir-missing-return.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

class DiagnosticSink;
struct IRModule;

// Returns false if compilation target does not allow and errors out(i.e. during downstream
// compilation) on missing returns.
static bool doesTargetAllowMissingReturns(CodeGenTarget target)
{
    if (isKhronosTarget(target) || isWGPUTarget(target))
    {
        return false;
    }

    return true;
}

static void diagnoseMissingReturnForTarget(DiagnosticSink* sink, CodeGenTarget target) {}

void checkForMissingReturnsRec(IRInst* inst, DiagnosticSink* sink, CodeGenTarget target)
{
    if (auto code = as<IRGlobalValueWithCode>(inst))
    {
        for (auto block : code->getBlocks())
        {
            auto terminator = block->getTerminator();

            if (auto missingReturn = as<IRMissingReturn>(terminator))
            {
                sink->diagnose(missingReturn, Diagnostics::missingReturn);
            }
        }
    }

    for (auto childInst : inst->getDecorationsAndChildren())
    {
        checkForMissingReturnsRec(childInst, sink, target);
    }
}

void checkForMissingReturns(IRModule* module, DiagnosticSink* sink, CodeGenTarget target)
{
    // Look for any `missingReturn` instructions
    checkForMissingReturnsRec(module->getModuleInst(), sink, target);
}

} // namespace Slang
