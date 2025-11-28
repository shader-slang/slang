// slang-pass-wrapper.h

#ifndef SLANG_PASS_WRAPPER_H
#define SLANG_PASS_WRAPPER_H

#include "../core/slang-performance-profiler.h"
#include "slang-compiler-options.h"

#include <optional>
#include <type_traits>

namespace Slang
{

struct IRModule;
struct CodeGenContext;

// Standalone hook functions
void prePassHooks(CodeGenContext* codeGenContext, IRModule* irModule);
void postPassHooks(CodeGenContext* codeGenContext, IRModule* irModule);

template<typename PassFunc, typename... Args>
auto wrapPass(
    CodeGenContext* codeGenContext,
    const char* passName,
    PassFunc&& passFunc,
    IRModule* irModule,
    Args&&... args) -> decltype(passFunc(irModule, std::forward<Args>(args)...))
{
    auto targetRequest = codeGenContext->getTargetReq();
    auto targetCompilerOptions = targetRequest->getOptionSet();

    std::optional<PerformanceProfilerFuncRAIIContext> perfContext;
    if (targetCompilerOptions.getBoolOption(CompilerOptionName::ReportDetailedPerfBenchmark))
    {
        perfContext.emplace(passName);
    }

    prePassHooks(codeGenContext, irModule);

    if constexpr (std::is_void_v<decltype(passFunc(irModule, std::forward<Args>(args)...))>)
    {
        passFunc(irModule, std::forward<Args>(args)...);
        postPassHooks(codeGenContext, irModule);
    }
    else
    {
        auto result = passFunc(irModule, std::forward<Args>(args)...);
        postPassHooks(codeGenContext, irModule);
        return result;
    }
}

} // namespace Slang

#endif // SLANG_PASS_WRAPPER_H