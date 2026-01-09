#ifndef SLANG_COMPILER_CORE_TEST_PROTOCOL_H
#define SLANG_COMPILER_CORE_TEST_PROTOCOL_H

#include "../core/slang-rtti-info.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-json-value.h"
#include "slang.h"

// WARNING: ABI Compatibility
// These structs are part of the binary interface between test-server and pre-built binaries
// (e.g., VK-GL-CTS). Adding or removing fields will change struct layout and break
// compatibility, causing memory corruption or crashes.
//
// To safely add new fields:
// 1. First update and release new versions of all pre-built binaries that use this protocol
// 2. Then add the new fields here
//
// The VK-GL-CTS nightly CI workflow will fail if this ABI is broken.

namespace TestServerProtocol
{

using namespace Slang;

struct ExecuteUnitTestArgs
{
    String moduleName;
    String testName;
    uint32_t enabledApis;
    bool enableDebugLayers;

    static const UnownedStringSlice g_methodName;
    static const StructRttiInfo g_rttiInfo;
};

struct ExecuteToolTestArgs
{
    String toolName;    ///< The name of the tool (will be a shared library typically - like
                        ///< render-test). Doesn't need -tool suffix.
    List<String> args;  ///< Arguments passed to the tool during exectution
    String testCommand; ///< Full test command for diagnostic logging (Optional)

    static const UnownedStringSlice g_methodName;
    static const StructRttiInfo g_rttiInfo;
};

struct QuitArgs
{
    static const UnownedStringSlice g_methodName;
};

struct ExecutionResult
{
    String stdOut;
    String stdError;
    String debugLayer;
    int32_t result = SLANG_OK;
    int32_t returnCode = 0;     ///< As returned if invoked as command line
    double executionTimeMs = 0; ///< Execution time in milliseconds (Optional)

    static const StructRttiInfo g_rttiInfo;
};

} // namespace TestServerProtocol

#endif // SLANG_COMPILER_CORE_TEST_PROTOCOL_H
