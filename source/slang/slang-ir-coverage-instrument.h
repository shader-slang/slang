#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

namespace Slang
{
struct IRModule;
struct IRVarLayout;
class DiagnosticSink;
class TargetRequest;
class ArtifactPostEmitMetadata;

// Shader coverage instrumentation pass.
//
// When `enabled` is true, the pass synthesizes a fresh
// `RWStructuredBuffer<uint> __slang_coverage` `IRGlobalParam` directly
// in the linked IR module, attaches a target-appropriate layout
// decoration (UAV register for D3D, descriptor binding for Khronos),
// extends the program-scope var layout so the buffer participates in
// `collectGlobalUniformParameters` packaging on targets that need it
// (CPU, CUDA), assigns one counter slot per `IncrementCoverageCounter`
// op, and rewrites each op into an atomic add on its slot. The pass
// writes the resulting `(slot → file, line)` mapping and the chosen
// buffer binding into `outMetadata` so hosts can query it via
// `ICoverageTracingMetadata`.
//
// `explicitBinding` / `explicitSpace` are the values supplied by
// `-trace-coverage-binding`; pass `-1` for either to request auto-
// allocation.
//
// `globalScopeVarLayout` is taken by reference: when the pass extends
// the program-scope layout to include the synthesized buffer, it
// updates the caller's pointer so the subsequent
// `collectGlobalUniformParameters` pass sees the extended layout.
//
// When `enabled` is false the pass is a no-op: any stray counter ops
// from cached modules are dropped so the backend never sees them, no
// buffer is synthesized, and `outMetadata` and `globalScopeVarLayout`
// are left untouched.
void instrumentCoverage(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    int explicitBinding,
    int explicitSpace,
    TargetRequest* targetRequest,
    IRVarLayout*& globalScopeVarLayout,
    ArtifactPostEmitMetadata& outMetadata);

// Defensive invariant: after `[ForceInline]` inlining has run, the
// synthesized `__slang_coverage_hit` thunk must have no surviving
// call sites. Each backend used by Slang honors `[ForceInline]` and
// folds the thunk's atomic-add body back into each call site at emit
// time, preserving the "byte-identical to pre-thunk emit" contract
// documented in `docs/design/shader-coverage.md`. If a backend ever
// stops honoring the decoration (or a target-specific pass strips
// it after instrumentation), an extra per-coverage-hit function-call
// frame would appear in emitted code; this check catches that
// regression in IR rather than as a per-target emit-output check.
//
// No-op when the coverage pass did not run (no thunk in the module).
void verifyCoverageThunkInlined(IRModule* module);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
