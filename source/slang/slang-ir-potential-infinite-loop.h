// slang-ir-potential-infinite-loop.h
#pragma once

namespace Slang
{
class DiagnosticSink;
struct IRModule;

/// Check for potential infinite loops in IR by detecting constant loop conditions
/// This pass analyzes loop structures to identify cases where loop variables
/// remain constant, indicating missing increment operations that cause infinite loops.
void checkForPotentialInfiniteLoops(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
