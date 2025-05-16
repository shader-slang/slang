// slang-ir-potential-infinite-loop.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;
struct IRInst;
struct IRLoop;
struct IRBlock;
enum class CodeGenTarget;

/// This IR check pass analyzes loops in the IR to detect potential infinite loops.
/// It looks for two key indicators of infinite loops:
/// 1. Loops with constant conditions (always evaluating to true)
/// 2. Loops with no exit paths (no branches to the break block)
///
/// When both conditions are met, it warns about a potential infinite loop.
void checkForPotentialInfiniteLoops(IRModule* module, DiagnosticSink* sink, bool diagnoseWarning);
} // namespace Slang
