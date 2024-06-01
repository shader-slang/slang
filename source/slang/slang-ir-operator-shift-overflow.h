// slang-ir-operator-shift-overflow.h
#pragma once

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    void checkForOperatorShiftOverflow(
        IRModule* module,
        DiagnosticSink* sink);
}
