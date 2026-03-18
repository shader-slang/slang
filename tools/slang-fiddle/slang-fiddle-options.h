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
                inputPathPrefix = expectArg(argCursor, argEnd);
            }
            else if (arg == UnownedTerminatedStringSlice("-o"))
            {
                outputPathPrefix = expectArg(argCursor, argEnd);
            }
            else if (
                arg == UnownedTerminatedStringSlice("-ignore-abort-msg") ||
                arg == UnownedTerminatedStringSlice("--ignore-abort-msg"))
            {
#ifdef _MSC_VER
                // Suppress the modal abort() dialog in unattended/LLM-driven builds.
                _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
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
};
} // namespace fiddle
