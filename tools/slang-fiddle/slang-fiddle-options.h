// slang-fiddle-options.h
#pragma once

#include "slang-fiddle-diagnostics.h"

#ifdef _MSC_VER
#include <stdlib.h>
#endif

namespace fiddle
{
using namespace Slang;

//

enum class FiddleMode
{
    Default, // Default: Generate .fiddle files for C++ code
    TestGen  // Generate test files from .slang templates
};

struct Options
{
public:
    static const char* expectArg(char const* const*& cursor, char const* const* end)
    {
        if (cursor != end)
            return *cursor++;
        return nullptr;
    }

    void parse(DiagnosticSink& sink, int argc, char const* const* argv)
    {
        auto argCursor = argv++;
        auto argEnd = argCursor + argc;

        if (argCursor != argEnd)
        {
            appName = *argCursor++;
        }

        while (argCursor != argEnd)
        {
            UnownedTerminatedStringSlice arg = *argCursor++;
            if (arg[0] != '-')
            {
                inputPaths.add(String(arg));
                continue;
            }

            if (arg == UnownedTerminatedStringSlice("-i"))
            {
                auto val = expectArg(argCursor, argEnd);
                if (!val)
                    sink.diagnose(SourceLoc(), Diagnostics::unknownOption, arg);
                else
                    inputPathPrefix = val;
            }
            else if (arg == UnownedTerminatedStringSlice("-o"))
            {
                auto val = expectArg(argCursor, argEnd);
                if (!val)
                    sink.diagnose(SourceLoc(), Diagnostics::unknownOption, arg);
                else
                    outputPathPrefix = val;
            }
            else if (arg == UnownedTerminatedStringSlice("-m"))
            {
                auto modeStr = expectArg(argCursor, argEnd);
                if (!modeStr)
                {
                    sink.diagnose(SourceLoc(), Diagnostics::unknownOption, arg);
                }
                else if (strcmp(modeStr, "default") == 0)
                {
                    fiddleMode = FiddleMode::Default;
                }
                else if (strcmp(modeStr, "test-gen") == 0)
                {
                    fiddleMode = FiddleMode::TestGen;
                }
                else
                {
                    sink.diagnose(
                        SourceLoc(),
                        Diagnostics::unknownOption,
                        String("-m ") + modeStr);
                }
            }
            else
            {
                sink.diagnose(SourceLoc(), Diagnostics::unknownOption, arg);
            }
        }
    }

    String appName = "slang-fiddle";
    String inputPathPrefix = "";
    String outputPathPrefix = "";
    List<String> inputPaths;

    FiddleMode fiddleMode = FiddleMode::Default;
};
} // namespace fiddle
