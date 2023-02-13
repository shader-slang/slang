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


    // Turn a loop with continue block into a loop with only back jumps and breaks.
    // Each iteration will be wrapped in a breakable region, where everything before `continue`
    // is within the breakable region, and everything after `continue` is outside the breakable
    // region. A `continue` then becomes a `break` in the inner breakable region, and a `break`
    // becomes a multi-level break out of the parent loop.
    void eliminateContinueBlocks(SharedIRBuilder* sharedBuilder, IRLoop* loopInst);
    void eliminateContinueBlocksInFunc(SharedIRBuilder* sharedBuilder, IRGlobalValueWithCode* func);

}
