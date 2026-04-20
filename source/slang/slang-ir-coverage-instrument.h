#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

namespace Slang
{
struct IRModule;
class DiagnosticSink;

// Prototype shader coverage instrumentation pass.
//
// When `enabled` is true, the pass ensures there is a module-scope
// `RWStructuredBuffer<uint> __slang_coverage` — either one declared by
// the user or a synthesized one bound at a reserved space/binding.
// For every basic block whose first source-bearing instruction has an
// IRDebugLine or IRDebugLocationDecoration, the pass injects an
// increment into a counter slot indexed by a deduplicated (file, line)
// key.
//
// When `enabled` is false the pass still preserves the pre-existing
// zero-config contract: if the user has declared `__slang_coverage`
// explicitly, instrument it; otherwise no-op.
//
// A counter->source manifest is printed to stderr when the
// `SLANG_COVERAGE_DUMP_MANIFEST` environment variable is set, in the
// form:
//     slang-coverage-manifest: <index>,<file>,<line>
void instrumentCoverage(IRModule* module, DiagnosticSink* sink, bool enabled);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
