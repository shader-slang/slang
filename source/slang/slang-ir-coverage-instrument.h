#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

namespace Slang
{
struct IRModule;
struct IRVarLayout;
class DiagnosticSink;
class TargetRequest;
class ArtifactPostEmitMetadata;

// Early shader coverage preparation pass.
//
// When `enabled` is true, the pass synthesizes a fresh
// `RWStructuredBuffer<uint> __slang_coverage` `IRGlobalParam` directly
// in the linked IR module, attaches a target-appropriate layout
// decoration (UAV register for D3D, descriptor binding for Khronos),
// extends the program-scope var layout so the buffer participates in
// `collectGlobalUniformParameters` packaging on targets that need it
// (CPU, CUDA), assigns one counter slot per `IncrementCoverageCounter`
// op, and records that slot on the marker as an IR decoration. The
// final rewrite into runtime coverage operations is deferred to the
// later `materializeCoverageInstrumentation` pass. The preparation pass
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
void prepareCoverageInstrumentation(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    int explicitBinding,
    int explicitSpace,
    TargetRequest* targetRequest,
    IRVarLayout*& globalScopeVarLayout,
    ArtifactPostEmitMetadata& outMetadata);

// Finalize coverage-related synthetic resource metadata after global
// uniform packing has run. This updates any backend-independent
// marshaling fields (for example CPU/CUDA uniform offsets) that can
// only be determined from the post-packing IR layout.
void finalizeCoverageInstrumentationMetadata(
    IRModule* module,
    bool enabled,
    TargetRequest* targetRequest,
    ArtifactPostEmitMetadata& outMetadata);

// Preserve the synthesized coverage binding while coverage remains in
// compact marker form. This should run after
// `collectGlobalUniformParameters`, before simplification/DCE can drop
// an otherwise-unused hidden resource on targets where coverage may be
// the only global parameter.
void preserveCoverageBindingForMaterialization(IRModule* module, bool enabled);

// Late shader coverage materialization pass.
//
// Lowers any surviving `IncrementCoverageCounter` ops into runtime
// coverage operations using the slot assigned by
// `prepareCoverageInstrumentation`. This pass should run after the
// major specialization/cloning-heavy transformations, but still before
// backend lowering stages that need to see the resulting resource
// accesses.
//
// When `enabled` is false, or when there are no remaining counter ops,
// the pass simply drops any stray markers.
void materializeCoverageInstrumentation(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
