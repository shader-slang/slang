// slang-ir-use-uninitialized-out-param.h
#pragma once

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    void checkForUsingUninitializedOutParams(
        IRModule*       module,
        DiagnosticSink* sink);
}
