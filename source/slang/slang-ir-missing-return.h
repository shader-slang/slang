// slang-ir-missing-return.h
#pragma once

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    void checkForMissingReturns(
        IRModule*       module,
        DiagnosticSink* sink);
}
