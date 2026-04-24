#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class ArtifactPostEmitMetadata;

// Shader coverage instrumentation pass.
//
// When `enabled` is true, the pass locates the module-scope
// `RWStructuredBuffer<uint> __slang_coverage` parameter that was
// synthesized earlier at semantic-check time, assigns one counter
// slot per `IncrementCoverageCounter` op, and rewrites each op into
// an atomic add on its slot. The pass writes the resulting
// `(slot → file, line)` mapping and the chosen buffer binding into
// `outMetadata` so hosts can query it via
// `ICoverageTracingMetadata` on the artifact.
//
// When `enabled` is false the pass is a no-op: any stray counter ops
// from cached modules are dropped so the backend never sees them,
// and `outMetadata` is left untouched.
void instrumentCoverage(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    ArtifactPostEmitMetadata& outMetadata);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
