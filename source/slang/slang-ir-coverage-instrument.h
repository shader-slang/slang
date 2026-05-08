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
// (CPU, CUDA), assigns counter slots deduplicated by `(file, line)`
// — multiple `IncrementCoverageCounter` ops on the same source line
// share one slot — and rewrites each op into a `Call` to a synthesized
// `[ForceInline]` `__slang_coverage_hit` thunk that performs the
// atomic add on its slot. The pass writes the resulting
// `(slot → file, line)` mapping and the chosen buffer binding into
// `outMetadata` so hosts can query it via `ICoverageTracingMetadata`.
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

// Defensive invariant + cleanup: after `[ForceInline]` inlining has
// run, the synthesized `__slang_coverage_hit` thunk must have no
// surviving call sites. Each backend used by Slang honors
// `[ForceInline]` and folds the thunk's atomic-add body back into
// each call site at emit time, preserving the "byte-identical to
// pre-thunk emit" contract documented in
// `docs/design/shader-coverage.md`. If a backend ever stops honoring
// the decoration (or a target-specific pass strips it after
// instrumentation), an extra per-coverage-hit function-call frame
// would appear in emitted code; this check catches that regression
// in IR rather than as a per-target emit-output check.
//
// On success the now-zero-use thunk is also removed from the module
// so its dead body cannot leak into emitted source on backends that
// do not (or no longer) run a downstream global-DCE pass.
//
// The thunk is identified by `IRCoverageThunkDecoration`, not by
// name hint — user code is free to declare a function with the same
// reserved-looking name without colliding with this verifier.
//
// No-op when the coverage pass did not run (no thunk in the module).
void verifyAndRemoveCoverageThunk(IRModule* module);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
