#ifndef SLANG_CORE_TEST_TOOL_UTIL_H
#define SLANG_CORE_TEST_TOOL_UTIL_H

#include "slang-std-writers.h"

namespace Slang {

#ifdef SLANG_SHARED_LIBRARY_TOOL
#   define SLANG_TEST_TOOL_API SLANG_EXTERN_C SLANG_DLL_EXPORT 
#else
#   define SLANG_TEST_TOOL_API
#endif

/* When a tool is run as an executable the return code is the code returned from
the last return of main. On unix this can be up to 8 bits. 
By normal command line tool conventions returning 0 means success. */
enum class ToolReturnCode
{
    CompilationFailed = -1,     ///< Compilation failure (-1 to maintain compatibility). This may still produce output and may mean a test was successful.
    Success = 0,                ///< Tool ran normally
    Failed,                     ///< Tool failed 
    Ignored,                    ///< The run was ignored because it couldn't be run (because some optional feature was not present for example)
    FailedToRun,                ///< Could not even run the test
};

enum class ToolReturnCodeSpan
{
    // Span of all valid values
    First = int(ToolReturnCode::CompilationFailed),
    Last = int(ToolReturnCode::FailedToRun),
    // Span of all values that indicate the test is 'done'
    FirstIsDone = int(ToolReturnCode::Ignored),
    LastIsDone = int(ToolReturnCode::FailedToRun)
};

/* Utility functions for 'test tools' */
struct TestToolUtil
{
    struct PreludeInfo
    {
        const char* exePath = nullptr;
        const char* nvAPIPath = nullptr;
    };

    typedef SlangResult(*InnerMainFunc)(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv);

        /// If the test failed to run or was ignored then we are done
    static bool isDone(ToolReturnCode code) { return int(code) >= int(ToolReturnCodeSpan::FirstIsDone) && int(code) <= int(ToolReturnCodeSpan::LastIsDone); }

        /// Convert from an int
    static ToolReturnCode getReturnCodeFromInt(int code);

        /// Given a slang result, returns a return code that can be returned from an executable
    static ToolReturnCode getReturnCode(SlangResult res);

        /// Sets the default preludes on the session based on the executable path
    static SlangResult setSessionDefaultPrelude(const PreludeInfo& preludeInfo, slang::IGlobalSession* session);

    static SlangResult setSessionDefaultPrelude(const char* exePath, slang::IGlobalSession* session);
};

} // namespace Slang

#endif // SLANG_TEST_TOOL_H
