// slang-ir-loop-unroll.h
#pragma once

#include "../core/slang-list.h"

namespace Slang
{
struct IRLoop;
struct IRGlobalValueWithCode;
class DiagnosticSink;
struct IRModule;
struct IRBlock;
class TargetProgram;

// Unroll every `[ForceUnroll]` loop in `func`. Returns true if successful,
// false if a loop could not be unrolled (an error is diagnosed to `sink`).
// When `outChanged` is non-null, it is set to true if any loop was unrolled
// (it is never reset to false, so one flag can accumulate across calls).
bool unrollLoopsInFunc(
    TargetProgram* target,
    IRModule* module,
    IRGlobalValueWithCode* func,
    DiagnosticSink* sink,
    bool* outChanged = nullptr);

bool unrollLoopsInModule(
    IRModule* module,
    TargetProgram* target,
    DiagnosticSink* sink,
    bool* outChanged = nullptr);

// Turn a loop with continue block into a loop with only back jumps and breaks.
// Each iteration will be wrapped in a breakable region, where everything before `continue`
// is within the breakable region, and everything after `continue` is outside the breakable
// region. A `continue` then becomes a `break` in the inner breakable region, and a `break`
// becomes a multi-level break out of the parent loop.
void eliminateContinueBlocks(IRModule* module, IRLoop* loopInst);
void eliminateContinueBlocksInFunc(IRModule* module, IRGlobalValueWithCode* func);

} // namespace Slang
