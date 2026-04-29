#ifndef SLANG_COMPILER_CORE_TEST_PROTOCOL_H
#define SLANG_COMPILER_CORE_TEST_PROTOCOL_H

#include "../core/slang-rtti-info.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-json-value.h"
#include "slang.h"

// WARNING: Protocol compatibility
// These structs define the serialized test-server protocol used between test-server and
// external clients/pre-built binaries (for example, VK-GL-CTS).
//
// Compatibility concerns here are about the wire format/schema, not the in-memory C++ struct
// layout. Do not remove, rename, or change the meaning/type of existing fields without
// coordinating updates to both sides of the protocol.
//
// To evolve this protocol compatibly, add new fields only as Optional in RTTI so older readers can
// ignore fields they do not know about and newer readers can tolerate those fields being absent in
// older messages.
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
