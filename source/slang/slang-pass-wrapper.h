// slang-pass-wrapper.h

#ifndef SLANG_PASS_WRAPPER_H
#define SLANG_PASS_WRAPPER_H

#include "../core/slang-performance-profiler.h"
#include "slang-code-gen.h"
#include "slang-compiler-options.h"

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

    PassHooksRAII(CodeGenContext* ctx, IRModule* module, const char* name)
        : codeGenContext(ctx), irModule(module), passName(name)
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
    extern void dumpIRIfEnabled(CodeGenContext* codeGenContext, IRModule* irModule, char const* label);
    static int dumpPostPass = -1;
    if (dumpPostPass == -1)
    {
        dumpPostPass = (getenv("SLANG_IRDUMP_POST_PASS") ? atoi(getenv("SLANG_IRDUMP_POST_PASS")) : 0);
    }
    //dumpIRIfEnabled(codeGenContext, irModule, (String("PRE_") + passName).getBuffer());
    using ReturnType = decltype(passFunc(irModule, std::forward<Args>(args)...));
    if constexpr (std::is_void_v<ReturnType>)
    {
        passFunc(irModule, std::forward<Args>(args)...);
        if (dumpPostPass > 0)
            dumpIRIfEnabled(codeGenContext, irModule, (String("POST_") + passName).getBuffer());
    }
    else
    {
        auto result = passFunc(irModule, std::forward<Args>(args)...);
        if (dumpPostPass > 0)
            dumpIRIfEnabled(codeGenContext, irModule, (String("POST_") + passName).getBuffer());
        return result;
    }
}

} // namespace Slang

#endif // SLANG_PASS_WRAPPER_H
