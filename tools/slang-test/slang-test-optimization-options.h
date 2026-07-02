// slang-test-optimization-options.h
#ifndef SLANG_TEST_OPTIMIZATION_OPTIONS_H
#define SLANG_TEST_OPTIMIZATION_OPTIONS_H

#include "core/slang-command-line.h"
#include "core/slang-render-api-util.h"
#include "core/slang-type-text-util.h"

namespace Slang
{
namespace SlangTest
{

static constexpr const char* kTestOptimizationOption = "-O0";

/// The default optimization level for Metal render tests.
///
/// Metal render tests compile through the downstream `metal` toolchain to produce a metallib,
/// and macOS CI showed mass flaky failures when that toolchain runs at `-O0` (the generated MSL
/// itself is identical at every level). `-O1` maps to the toolchain's previous default flags, so
/// Metal render tests keep the behavior that passed CI while still receiving an explicit
/// slang-test-controlled level like every other target.
static constexpr const char* kMetalRenderTestOptimizationOption = "-O1";

/// Returns true when an argument is a slangc optimization-level option.
///
/// This uses slangc's optimization-level names so unrelated options with a `-O` prefix do not
/// accidentally opt a test out of the slang-test default.
inline bool isSlangOptimizationArg(const String& arg)
{
    const auto argSlice = arg.getUnownedSlice();
    const auto optimizationPrefix = UnownedStringSlice::fromLiteral("-O");
    if (!argSlice.startsWith(optimizationPrefix))
        return false;

    const auto levelSlice = argSlice.tail(optimizationPrefix.getLength());
    if (levelSlice.getLength() == 0)
        return true;

    return NameValueUtil::findValue(
               TypeTextUtil::getOptimizationLevelInfos(),
               levelSlice,
               ValueInt(-1)) != ValueInt(-1);
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
/// Metal render tests receive `-O1` instead of `-O0`; see
/// `kMetalRenderTestOptimizationOption` for why.
inline void addDefaultRenderTestSlangOptimization(CommandLine& ioCmdLine)
{
    if (hasRenderTestSlangOptimizationArg(ioCmdLine.m_args))
        return;

    const bool isMetal = hasRenderTestRenderApiArg(ioCmdLine.m_args, RenderApiType::Metal);

    ioCmdLine.addArg("-Xslang");
    ioCmdLine.addArg(isMetal ? kMetalRenderTestOptimizationOption : kTestOptimizationOption);
}

} // namespace SlangTest
} // namespace Slang

#endif
