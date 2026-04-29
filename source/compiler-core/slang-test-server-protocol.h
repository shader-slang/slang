#ifndef SLANG_COMPILER_CORE_TEST_PROTOCOL_H
#define SLANG_COMPILER_CORE_TEST_PROTOCOL_H

#include "../core/slang-rtti-info.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-json-value.h"
#include "slang.h"

// WARNING: Protocol/layout compatibility
// These structs define the serialized test-server protocol used between test-server and
// external clients/pre-built binaries (for example, VK-GL-CTS).
//
// These headers are also compiled into some external/pre-built clients, so the in-memory C++
// layout is part of the effective binary contract for those clients. Do not add, remove, rename,
// reorder, or change existing fields without first updating and releasing all pre-built clients
// that include this header.
//
// Marking a field Optional in RTTI is not enough when an older binary was built with the previous
// struct layout.
//
// The VK-GL-CTS nightly CI workflow will fail if this protocol compatibility is broken.

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
    String toolName;   ///< The name of the tool (will be a shared library typically - like
                       ///< render-test). Doesn't need -tool suffix.
    List<String> args; ///< Arguments passed to the tool during exectution

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
    int32_t returnCode = 0; ///< As returned if invoked as command line

    static const StructRttiInfo g_rttiInfo;
};

} // namespace TestServerProtocol

#endif // SLANG_COMPILER_CORE_TEST_PROTOCOL_H
