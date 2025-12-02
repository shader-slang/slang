// slang-pass-wrapper.cpp

#include "slang-pass-wrapper.h"

#include "../core/slang-dictionary.h"
#include "../core/slang-writer.h"
#include "slang-ir-validate.h"
#include "slang-ir.h"

namespace Slang
{

// Forward declaration for dumpIR function
void dumpIR(
    IRModule* module,
    IRDumpOptions const& options,
    char const* label,
    SourceManager* sourceManager,
    ISlangWriter* writer);

// Helper function to check if a pass name is in a hash set of pass names
static bool isPassNameInSet(const HashSet<String>& passNamesSet, const char* passName)
{
    return passName && passNamesSet.contains(String(passName));
}

// Helper function to dump IR for specific passes (doesn't check shouldDumpIR)
static void dumpIRForPass(CodeGenContext* codeGenContext, IRModule* irModule, const char* label)
{
    DiagnosticSinkWriter writer(codeGenContext->getSink());
    dumpIR(
        irModule,
        codeGenContext->getIRDumpOptions(),
        label,
        codeGenContext->getSourceManager(),
        &writer);
}

void prePassHooks(CodeGenContext* codeGenContext, IRModule* irModule, const char* passName)
{
    auto targetRequest = codeGenContext->getTargetReq();
    auto targetCompilerOptions = targetRequest->getOptionSet();

    // Check if we should dump IR before this pass
    if (auto dumpBeforeOptions =
            targetCompilerOptions.options.tryGetValue(CompilerOptionName::DumpIRBefore))
    {
        HashSet<String> dumpBeforeSet;
        for (const auto& option : *dumpBeforeOptions)
        {
            dumpBeforeSet.add(option.stringValue);
        }

        if (isPassNameInSet(dumpBeforeSet, passName))
        {
            String label = String("BEFORE ") + passName;
            dumpIRForPass(codeGenContext, irModule, label.getBuffer());
        }
    }
}

void postPassHooks(CodeGenContext* codeGenContext, IRModule* irModule, const char* passName)
{
    auto targetRequest = codeGenContext->getTargetReq();
    auto targetCompilerOptions = targetRequest->getOptionSet();

    // Check if we should perform detailed IR validation
    if (targetCompilerOptions.getBoolOption(CompilerOptionName::ValidateIRDetailed))
    {
        validateIRModule(irModule, codeGenContext->getSink());
    }

    // Check if we should dump IR after this pass
    bool shouldDumpForThisPass = false;
    String dumpLabel;

    // Dump IR after every pass if -dump-ir is enabled
    if (targetCompilerOptions.getBoolOption(CompilerOptionName::DumpIr))
    {
        shouldDumpForThisPass = true;
        dumpLabel = String("AFTER ") + passName;
    }
    // Otherwise check if we should dump IR after this specific pass
    else if (
        auto dumpAfterOptions =
            targetCompilerOptions.options.tryGetValue(CompilerOptionName::DumpIRAfter))
    {
        HashSet<String> dumpAfterSet;
        for (const auto& option : *dumpAfterOptions)
        {
            dumpAfterSet.add(option.stringValue);
        }

        if (isPassNameInSet(dumpAfterSet, passName))
        {
            shouldDumpForThisPass = true;
            dumpLabel = String("AFTER ") + passName;
        }
    }

    if (shouldDumpForThisPass)
    {
        dumpIRForPass(codeGenContext, irModule, dumpLabel.getBuffer());
    }
}

} // namespace Slang