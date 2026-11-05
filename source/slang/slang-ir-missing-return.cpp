// ir-missing-return.cpp
#include "slang-ir-missing-return.h"

#include "core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-target-switch.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

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

static void diagnoseMissingReturnForTarget(
    IRMissingReturn* missingReturn,
    DiagnosticSink* sink,
    SlangLanguageVersion languageVersion,
    CodeGenTarget target,
    bool diagnoseWarning)
{
    // A `missingReturn` standing for a `__target_switch` with no case for the target is reported
    // as E41037 rather than as a missing return; see `specializeTargetSwitch` for where.
    if (auto noTargetCase = missingReturn->findDecoration<IRNoTargetCaseDecoration>())
    {
        if (!doesTargetAllowMissingReturns(target))
            diagnoseNoTargetCase(noTargetCase, target, sink);
        return;
    }

    if (languageVersion >= SlangLanguageVersion::SLANG_LANGUAGE_VERSION_202C)
    {
        sink->diagnose(
            Diagnostics::MissingReturnNotAllowedInSlang202c{.location = missingReturn->sourceLoc});
    }
    else if (doesTargetAllowMissingReturns(target))
    {
        if (diagnoseWarning)
        {
            sink->diagnose(Diagnostics::MissingReturn{.location = missingReturn->sourceLoc});
        }
    }
    else
    {
        sink->diagnose(Diagnostics::MissingReturnError{
            .targetName = TypeTextUtil::getCompileTargetName(SlangCompileTarget(target)),
            .location = missingReturn->sourceLoc,
        });
    }
}

void checkForMissingReturnsRec(
    IRInst* inst,
    DiagnosticSink* sink,
    SlangLanguageVersion languageVersion,
    CodeGenTarget target,
    bool diagnoseWarning)
{
    if (auto code = as<IRGlobalValueWithCode>(inst))
    {
        for (auto block : code->getBlocks())
        {
            auto terminator = block->getTerminator();

            if (auto missingReturn = as<IRMissingReturn>(terminator))
            {
                diagnoseMissingReturnForTarget(
                    missingReturn,
                    sink,
                    languageVersion,
                    target,
                    diagnoseWarning);
            }
        }
    }

    for (auto childInst : inst->getDecorationsAndChildren())
    {
        checkForMissingReturnsRec(childInst, sink, languageVersion, target, diagnoseWarning);
    }
}

void checkForMissingReturns(
    IRModule* module,
    DiagnosticSink* sink,
    SlangLanguageVersion languageVersion,
    CodeGenTarget target,
    bool diagnoseWarning)
{
    // Look for any `missingReturn` instructions
    checkForMissingReturnsRec(
        module->getModuleInst(),
        sink,
        languageVersion,
        target,
        diagnoseWarning);
}

} // namespace Slang
