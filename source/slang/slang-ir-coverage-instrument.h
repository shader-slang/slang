#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

namespace Slang
{
struct IRModule;
class DiagnosticSink;

// Shader coverage instrumentation pass.
//
// When `enabled` is true, the pass synthesizes a module-scope
// `RWStructuredBuffer<uint> __slang_coverage` at a reserved binding
// and rewrites every IncrementCoverageCounter op into an atomic add on
// a counter slot. Slots are assigned by deduplicating (file, line)
// keys read from each op's built-in `sourceLoc`, sorted
// lexicographically for stability across unrelated source edits.
//
// When `enabled` is false the pass is a no-op: any stray counter ops
// from cached modules are dropped so the backend never sees them.
void instrumentCoverage(IRModule* module, DiagnosticSink* sink, bool enabled);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
