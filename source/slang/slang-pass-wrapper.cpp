// slang-pass-wrapper.cpp

#include "slang-pass-wrapper.h"

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

// Helper function to check if we should dump IR for a pass
static bool shouldDumpPass(
    CompilerOptionSet& options,
    CompilerOptionName optionName,
    const char* passName)
{
    if (!passName)
        return false;

    // For after dumps, -dump-ir enables dumping after every pass
    if (optionName == CompilerOptionName::DumpIRAfter &&
        options.getBoolOption(CompilerOptionName::DumpIr))
        return true;

    // Check if this specific pass is in the list
    if (auto passOptions = options.options.tryGetValue(optionName))
    {
        return passOptions->findFirstIndex([passName](const CompilerOptionValue& option)
                                           { return option.stringValue == passName; }) != -1;
    }

    return false;
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
    if (shouldDumpPass(targetCompilerOptions, CompilerOptionName::DumpIRBefore, passName))
    {
        String label = String("BEFORE ") + passName;
        dumpIRForPass(codeGenContext, irModule, label.getBuffer());
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
    if (shouldDumpPass(targetCompilerOptions, CompilerOptionName::DumpIRAfter, passName))
    {
        String label = String("AFTER ") + passName;
        dumpIRForPass(codeGenContext, irModule, label.getBuffer());
    }
}

} // namespace Slang