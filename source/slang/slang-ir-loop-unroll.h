// slang-ir-loop-unroll.h
#pragma once

namespace Slang
{
    struct IRLoop;
    struct IRGlobalValueWithCode;
    struct SharedIRBuilder;
    class DiagnosticSink;
    struct IRModule;

    // Return true if successfull, false if errors occurred.
    bool unrollLoopsInFunc(SharedIRBuilder* sharedBuilder, IRGlobalValueWithCode* func, DiagnosticSink* sink);

    bool unrollLoopsInModule(SharedIRBuilder* sharedBuilder, IRModule* module, DiagnosticSink* sink);
}
