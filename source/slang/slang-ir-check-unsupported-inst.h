#pragma once

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;

    void checkUnsupportedInst(IRModule* module, DiagnosticSink* sink);
}
