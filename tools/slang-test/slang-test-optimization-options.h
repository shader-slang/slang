// slang-test-optimization-options.h
#ifndef SLANG_TEST_OPTIMIZATION_OPTIONS_H
#define SLANG_TEST_OPTIMIZATION_OPTIONS_H

#include "core/slang-command-line.h"
#include "core/slang-render-api-util.h"

namespace Slang
{
namespace SlangTest
{

static constexpr const char* kTestOptimizationOption = "-O0";

/// Returns true when an argument is a slangc optimization-level option.
///
/// This uses the exact spellings recognized by slangc so unrelated options with a `-O` prefix do
/// not accidentally opt a test out of the slang-test default.
inline bool isSlangOptimizationArg(const String& arg)
{
    return arg == "-O" || arg == "-O0" || arg == "-Onone" || arg == "-O1" || arg == "-Odefault" ||
           arg == "-O2" || arg == "-Ohigh" || arg == "-O3" || arg == "-Omaximal";
}

/// Returns true when a compiler command line already specifies an optimization level.
inline bool hasSlangOptimizationArg(const List<String>& args)
{
    for (const auto& arg : args)
    {
        if (isSlangOptimizationArg(arg))
            return true;
    }
    return false;
}

/// Adds the slang-test default optimization level unless the test already specifies one.
///
/// Most compiler-based tests do not need optimized output, and keeping them at `-O0` avoids
/// optimizer time and SPIR-V optimizer output churn.
inline void addDefaultSlangOptimization(CommandLine& ioCmdLine)
{
    if (!hasSlangOptimizationArg(ioCmdLine.m_args))
    {
        ioCmdLine.addArg(kTestOptimizationOption);
    }
}

/// Returns true when a render-test command forwards an optimization level to slangc.
///
/// Render-test accepts several forwarding forms, including single-argument forwarding and
/// `-Xslang...` blocks, so slang-test checks each forwarded slangc argument before adding its
/// default.
inline bool hasRenderTestSlangOptimizationArg(const List<String>& args)
{
    for (Index i = 0; i < args.getCount(); ++i)
    {
        const auto& arg = args[i];
        if ((arg == "-compile-arg" || arg == "-xslang" || arg == "-Xslang") &&
            i + 1 < args.getCount() && isSlangOptimizationArg(args[i + 1]))
        {
            return true;
        }
        if (arg == "-Xslang...")
        {
            for (++i; i < args.getCount() && args[i] != "-X."; ++i)
            {
                if (isSlangOptimizationArg(args[i]))
                    return true;
            }
        }
    }
    return false;
}

/// Returns true when a render-test command explicitly selects the given API.
inline bool hasRenderTestRenderApiArg(const List<String>& args, RenderApiType apiType)
{
    for (const auto& arg : args)
    {
        if (arg.getLength() <= 1 || arg[0] != '-')
            continue;

        UnownedStringSlice name(arg.getUnownedSlice().begin() + 1, arg.getUnownedSlice().end());
        if (RenderApiUtil::findApiTypeByName(name) == apiType)
            return true;
    }

    return false;
}

/// Adds the slang-test default optimization level to render-test commands.
///
/// The option is forwarded with `-Xslang` because render-test options are not slangc options.
inline void addDefaultRenderTestSlangOptimization(CommandLine& ioCmdLine)
{
    if (hasRenderTestSlangOptimizationArg(ioCmdLine.m_args))
        return;

    // Metal render-test paths do not invoke spv-opt, and many generated Metal CI variants rely on
    // the optimized output that render-test/slangc uses by default.
    if (hasRenderTestRenderApiArg(ioCmdLine.m_args, RenderApiType::Metal))
        return;

    ioCmdLine.addArg("-Xslang");
    ioCmdLine.addArg(kTestOptimizationOption);
}

} // namespace SlangTest
} // namespace Slang

#endif
