#ifndef SLANG_TEST_TOOL_UTIL_H
#define SLANG_TEST_TOOL_UTIL_H

#include "slang-std-writers.h"

#include "slang-render-api-util.h"

namespace Slang {

#ifdef SLANG_SHARED_LIBRARY_TOOL
#   define SLANG_TEST_TOOL_API SLANG_EXTERN_C SLANG_DLL_EXPORT 
#else
#   define SLANG_TEST_TOOL_API
#endif

/* Enumeration of the different kinds of compiler backends slang needs. */
enum class BackendType
{
    Unknown = -1,
    Dxc,
    Fxc,
    Glslang,
    CountOf,
};

typedef uint32_t BackendFlags;
struct BackendFlag
{
    enum Enum : uint32_t
    {
        Dxc = 1 << int(BackendType::Dxc),
        Fxc = 1 << int(BackendType::Fxc),
        Glslang = 1 << int(BackendType::Glslang),
    };
};

/// Structure that describes requirements needs to run - such as rendering APIs or
/// back-end availability 
struct TestRequirements
{
    TestRequirements& addUsed(BackendType type)
    {
        if (type != BackendType::Unknown)
        {
            usedBackendFlags |= BackendFlags(1) << int(type);
        }
        return *this;
    }
    TestRequirements& addUsed(Slang::RenderApiType type)
    {
        using namespace Slang;
        if (type != RenderApiType::Unknown)
        {
            usedRenderApiFlags |= RenderApiFlags(1) << int(type);
        }
        return *this;
    }
    TestRequirements& addUsedBackends(BackendFlags flags)
    {
        usedBackendFlags |= flags;
        return *this;
    }
    TestRequirements& addUsedRenderApis(Slang::RenderApiFlags flags)
    {
        usedRenderApiFlags |= flags;
        return *this;
    }
    /// True if has this render api as used
    bool isUsed(Slang::RenderApiType apiType) const
    {
        return (apiType != Slang::RenderApiType::Unknown) && ((usedRenderApiFlags & (Slang::RenderApiFlags(1) << int(apiType))) != 0);
    }

    Slang::RenderApiType explicitRenderApi = Slang::RenderApiType::Unknown;     ///< The render api explicitly specified 
    BackendFlags usedBackendFlags = 0;                                          ///< Used backends
    Slang::RenderApiFlags usedRenderApiFlags = 0;                               ///< Used render api flags (some might be implied)
};

/* An interface to communicate between a test tool and the test runner */
struct ITestTool : public ISlangUnknown
{
public:
        /** Run the test using the session and the specified args.
        @param stdWriters Writes used for output
        @param session A slang session
        @param argc The number of arguments
        @param argv The arguments */
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL run(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv) = 0;
        /** Parse the specified command line parameters.
        @param stdWriters Writers used for output
        @param argc The number of arguments
        @param argv The arguments
        @param outRequirements The requirements of this test */
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcTestRequirements(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv, TestRequirements* ioRequirements ) = 0;
};

#define SLANG_UUID_Slang_ITestTool { 0x1023b36c, 0xda3f, 0x40fb, { 0xa0, 0xb1, 0xb5, 0xcd, 0xc9, 0x61, 0x96, 0xd1 } };
    
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
    typedef SlangResult(*getTestToolFunc)(const Guid& intf, ISlangUnknown** toolOut);

        /// If the test failed to run or was ignored then we are done
    static bool isDone(ToolReturnCode code) { return int(code) >= int(ToolReturnCodeSpan::FirstIsDone) && int(code) <= int(ToolReturnCodeSpan::LastIsDone); }

        /// Given a list of args extracts an args value specified by name. Arg name must have - prefix, and the result is the next arg.
    static SlangResult extractArg(const char*const* args, int argsCount, const char* argName, const char** outValue);

        /// Returns true if the option, specified as argName, is found in the list of args
    static bool hasOption(const char*const* args, int argsCount, const char* argName);

        /// Given a string returns BackendType or BackendType::Unknown if not found
    static BackendType toBackendType(const UnownedStringSlice& slice);

        /// Given a compilation target returns what backends are required as flags
    static BackendFlags getBackendFlagsForTarget(SlangCompileTarget target);

        /// For given pass through type returns the associated backend type
    static BackendType toBackendTypeFromPassThroughType(SlangPassThrough passThru);

        /// Given a string returns as the associated SlangCompileTarget or Unknown if not found
    static SlangCompileTarget toCompileTarget(const UnownedStringSlice& name);

        /// Convert from an int
    static ToolReturnCode getReturnCodeFromInt(int code);

        /// Given a slang result, returns a return code that can be returned from an executable
    static ToolReturnCode getReturnCode(SlangResult res);
};

} // namespace Slang

#endif // SLANG_TEST_TOOL_H
