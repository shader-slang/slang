// slang-test-diagnostics.h
// Diagnostic control for slang-test debugging
// Enabled via SLANG_TEST_DIAGNOSTICS environment variable

#ifndef SLANG_TEST_DIAGNOSTICS_H
#define SLANG_TEST_DIAGNOSTICS_H

#include "slang-platform.h"
#include "slang-string-util.h"
#include "slang-string.h"

#include <functional>
#include <stddef.h>
#include <thread>

namespace Slang
{

struct TestDiagnosticConfig
{
    bool all = false;
    bool timing = false;
    bool timingPhases = false;
    bool rpc = false;
    bool fd = false;
    bool pipe = false;
};

inline TestDiagnosticConfig parseTestDiagnosticConfig(UnownedStringSlice diagnostics)
{
    TestDiagnosticConfig result;

    List<UnownedStringSlice> categories;
    StringUtil::split(diagnostics, ',', categories);

    for (const auto& rawCategory : categories)
    {
        const UnownedStringSlice category = rawCategory.trim();
        if (category.caseInsensitiveEquals(UnownedStringSlice("all")) ||
            category.caseInsensitiveEquals(UnownedStringSlice("1")))
        {
            result.all = true;
        }
        else if (category.caseInsensitiveEquals(UnownedStringSlice("timing")))
        {
            result.timing = true;
        }
        else if (category.caseInsensitiveEquals(UnownedStringSlice("timing-phases")))
        {
            result.timing = true;
            result.timingPhases = true;
        }
        else if (category.caseInsensitiveEquals(UnownedStringSlice("rpc")))
        {
            result.rpc = true;
        }
        else if (category.caseInsensitiveEquals(UnownedStringSlice("fd")))
        {
            result.fd = true;
        }
        else if (category.caseInsensitiveEquals(UnownedStringSlice("pipe")))
        {
            result.pipe = true;
        }
    }

    return result;
}

inline const TestDiagnosticConfig& getTestDiagnosticConfig()
{
    static TestDiagnosticConfig config = []() -> TestDiagnosticConfig
    {
        StringBuilder envValue;
        if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(
                UnownedStringSlice("SLANG_TEST_DIAGNOSTICS"),
                envValue)))
        {
            return TestDiagnosticConfig();
        }

        const String diagnostics = envValue.toString();
        return parseTestDiagnosticConfig(diagnostics.getUnownedSlice());
    }();
    return config;
}

// Check if a diagnostic category is enabled via SLANG_TEST_DIAGNOSTICS env var.
// Categories: timing, timing-phases, rpc, fd, pipe, all
// Thread-safe: reads and parses the env var once. Prefer the typed helpers below for hot paths.
inline bool isTestTimingDiagnosticEnabled()
{
    const TestDiagnosticConfig& config = getTestDiagnosticConfig();
    return config.all || config.timing;
}

inline bool isTestTimingPhaseDiagnosticEnabled()
{
    const TestDiagnosticConfig& config = getTestDiagnosticConfig();
    return config.all || config.timingPhases;
}

inline bool isTestRpcDiagnosticEnabled()
{
    const TestDiagnosticConfig& config = getTestDiagnosticConfig();
    return config.all || config.rpc;
}

inline bool isTestFileDescriptorDiagnosticEnabled()
{
    const TestDiagnosticConfig& config = getTestDiagnosticConfig();
    return config.all || config.fd;
}

inline bool isTestPipeDiagnosticEnabled()
{
    const TestDiagnosticConfig& config = getTestDiagnosticConfig();
    return config.all || config.pipe;
}

inline bool isDiagnosticEnabled(const char* category)
{
    const TestDiagnosticConfig& config = getTestDiagnosticConfig();
    if (config.all)
        return true;

    const UnownedStringSlice categorySlice(category);
    if (categorySlice.caseInsensitiveEquals(UnownedStringSlice("timing")))
        return isTestTimingDiagnosticEnabled();
    if (categorySlice.caseInsensitiveEquals(UnownedStringSlice("timing-phases")))
        return isTestTimingPhaseDiagnosticEnabled();
    if (categorySlice.caseInsensitiveEquals(UnownedStringSlice("rpc")))
        return isTestRpcDiagnosticEnabled();
    if (categorySlice.caseInsensitiveEquals(UnownedStringSlice("fd")))
        return isTestFileDescriptorDiagnosticEnabled();
    if (categorySlice.caseInsensitiveEquals(UnownedStringSlice("pipe")))
        return isTestPipeDiagnosticEnabled();
    if (categorySlice.caseInsensitiveEquals(UnownedStringSlice("all")) ||
        categorySlice.caseInsensitiveEquals(UnownedStringSlice("1")))
        return config.all;

    return false;
}

// Opaque thread value for correlating diagnostic log lines. This is not an OS thread id.
inline size_t getCurrentThreadDiagnosticId()
{
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

} // namespace Slang

#endif // SLANG_TEST_DIAGNOSTICS_H
