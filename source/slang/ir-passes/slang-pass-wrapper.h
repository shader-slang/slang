// slang-pass-wrapper.h

#ifndef SLANG_PASS_WRAPPER_H
#define SLANG_PASS_WRAPPER_H

#include "core/slang-crash-breadcrumbs.h"
#include "core/slang-performance-profiler.h"
#include "slang/slang-code-gen.h"
#include "slang/slang-compiler-options.h"

#include <optional>
#include <type_traits>

namespace Slang
{

struct IRModule;
struct CodeGenContext;

// Standalone hook functions
void prePassHooks(CodeGenContext* codeGenContext, IRModule* irModule, const char* passName);
void postPassHooks(CodeGenContext* codeGenContext, IRModule* irModule, const char* passName);

// RAII helper for pass hooks and performance profiling
struct PassHooksRAII
{
    CodeGenContext* codeGenContext;
    IRModule* irModule;
    const char* passName;
    std::optional<PerformanceProfilerFuncRAIIContext> perfContext;

    // A crash report names the pass that was running when the compiler failed.
    CrashBreadcrumbScope passBreadcrumb;

    PassHooksRAII(CodeGenContext* ctx, IRModule* module, const char* name)
        : codeGenContext(ctx)
        , irModule(module)
        , passName(name)
        , passBreadcrumb(CrashBreadcrumbField::Pass, name)
    {
        prePassHooks(codeGenContext, irModule, passName);

        auto targetRequest = codeGenContext->getTargetReq();
        auto targetCompilerOptions = targetRequest->getOptionSet();
        if (targetCompilerOptions.getBoolOption(CompilerOptionName::ReportDetailedPerfBenchmark))
        {
            perfContext.emplace(passName);
        }
    }

    ~PassHooksRAII()
    {
        perfContext.reset(); // End profiler timing before post hooks
        postPassHooks(codeGenContext, irModule, passName);
    }
};

template<typename PassFunc, typename... Args>
auto wrapPass(
    CodeGenContext* codeGenContext,
    const char* passName,
    PassFunc&& passFunc,
    IRModule* irModule,
    Args&&... args)
{
    PassHooksRAII passHooks(codeGenContext, irModule, passName);
    return passFunc(irModule, std::forward<Args>(args)...);
}

} // namespace Slang

#endif // SLANG_PASS_WRAPPER_H
