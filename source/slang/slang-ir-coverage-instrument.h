#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;
struct IRVarLayout;
class DiagnosticSink;
class TargetRequest;
class ArtifactPostEmitMetadata;

// Default per-slot byte width for the synthesized `__slang_coverage`
// buffer when the user does not opt down via
// `-trace-coverage-counter-width` (CLI) or
// `CompilerOptionName::TraceCoverageCounterByteWidth` (API). `8` =
// `uint64_t`, which effectively cannot wrap within any practical run.
static constexpr int kDefaultCoverageCounterByteWidth = 8;

// Shader coverage instrumentation pass.
//
// When `enabled` is true, the pass synthesizes a fresh
// `RWStructuredBuffer<uint64_t> __slang_coverage` `IRGlobalParam`
// (or `RWStructuredBuffer<uint>` if the caller opts down via
// `counterByteWidth`; see below) directly in the linked IR module,
// attaches a target-appropriate layout decoration (UAV register for
// D3D, descriptor binding for Khronos), extends the program-scope var
// layout so the buffer participates in
// `collectGlobalUniformParameters` packaging on targets that need it
// (CPU, CUDA), and rewrites coverage marker ops into atomic adds. The
// line producer coalesces markers that provably execute together onto
// one counter slot and one runtime probe, so counter count is never
// larger than entry count, and smaller whenever a straight-line region
// is coalesced; function and branch producers keep one dedicated slot
// per marker, so the two counts are equal when only those modes are
// enabled. Marker kind selects the emitted source-entry metadata:
// line, function, branch, and later region coverage all share this
// path. The pass writes the resulting source coverage entries and the
// chosen buffer binding into
// `outMetadata` so hosts can query them via
// `ICoverageTracingMetadata` and `ISyntheticResourceMetadata`.
//
// `explicitBinding` / `explicitSpace` are the values supplied by
// `-trace-coverage-binding`; pass `-1` for either to request auto-
// allocation.
// `reservedSpaces` is the optional list supplied by
// `-trace-coverage-reserved-space`; auto-allocation treats each space
// as externally occupied even if no shader-visible resource uses it.
//
// `globalScopeVarLayout` is taken by reference: when the pass extends
// the program-scope layout to include the synthesized buffer, it
// updates the caller's pointer so the subsequent
// `collectGlobalUniformParameters` pass sees the extended layout.
//
// `counterByteWidth` selects the per-slot element width of the
// synthesized buffer: `8` for `RWStructuredBuffer<uint64_t>` (the
// default; effectively immune to counter wrap), or `4` for
// `RWStructuredBuffer<uint>` (wraps at 2^32 hits per slot — only used
// when the runtime driver lacks 64-bit shader atomic add). Both
// entry paths (the `-trace-coverage-counter-width` CLI parser and
// the `TraceCoverageCounterByteWidth` API option) validate the value
// to `{4, 8}` before calling in, so any other value is a
// compiler-internal contract violation; the pass asserts rather than
// silently coercing.
//
// When `enabled` is false the pass is a no-op: any stray marker ops
// from cached modules are dropped so the backend never sees them, no
// buffer is synthesized, and `outMetadata` and `globalScopeVarLayout`
// are left untouched.
//
// `booleanMode` opts in to boolean recording (`CoverageCounterMode::Boolean`):
// each counter is written with a plain non-atomic store of `1` instead of
// an atomic add, so it records whether the entry executed (0 / non-zero)
// rather than an exact count. This removes all atomic contention. Off by
// default.
// `bindlessIndex` is the value supplied by
// `-trace-coverage-bindless-index`; pass `-1` for the ordinary
// single-buffer form. When it is >= 0 the synthesized global becomes an
// UNBOUNDED ARRAY of structured buffers rather than one buffer, and
// every counter access indexes through it:
// `__slang_coverage[bindlessIndex][slot]`. Many separately compiled
// shaders sharing one pipeline then occupy a single descriptor binding
// instead of one binding each — at the cost of requiring descriptor
// indexing. A caller must therefore only pass `bindlessIndex >= 0` for a
// Khronos target: the user-facing rejection belongs in `linkAndOptimizeIR`,
// which validates it before this pass is gated on the module having any
// coverage markers, so that a module with nothing instrumentable still
// diagnoses. This pass asserts the invariant rather than re-reporting it.
//
// The index is a compile-time constant and therefore part of
// the compiled artifact: a host that keys a shader cache on that
// artifact must derive the index from a stable shader identity rather
// than from load order, or an unchanged shader recompiles whenever the
// order shifts. Supplying the index at pipeline creation instead would
// avoid that entirely; see issue #12541.
void instrumentCoverage(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    int explicitBinding,
    int explicitSpace,
    const int* reservedSpaces,
    int reservedSpaceCount,
    int counterByteWidth,
    bool booleanMode,
    int bindlessIndex,
    TargetRequest* targetRequest,
    IRVarLayout*& globalScopeVarLayout,
    ArtifactPostEmitMetadata& outMetadata);

// Finalize coverage-related synthetic resource metadata after global
// and entry-point uniform packing has run. This updates CPU/CUDA
// uniform-marshaling fields that can only be determined from the
// final post-packing IR layout.
void finalizeCoverageInstrumentationMetadata(
    IRModule* module,
    DiagnosticSink* sink,
    bool enabled,
    IRVarLayout* globalScopeVarLayout,
    TargetRequest* targetRequest,
    ArtifactPostEmitMetadata& outMetadata);

// Assign a counter slot to every collected marker op, coalescing line
// markers that provably execute together. This is the coalescing core of
// `instrumentCoverage`; it is declared here (rather than kept file-local) so
// `slang-static-unit-test` can drive it directly on hand-built IR and assert
// on the slot assignment — which is the only way to observe some of its
// guarantees (notably which marker of a coalesced region emits the probe;
// see `outEmitsProbe` below).
//
// `markerOps` must be in the order `collectCoverageMarkerOps` produces:
// grouped by function, then by block, then by instruction position within a
// block, with the markers of any one block contiguous. Coalescing scans
// forward from the previous marker to the current one, so an out-of-order or
// interleaved list would violate that precondition (asserted in debug).
//
// Line markers in the same basic block, with nothing between them that can
// abandon the invocation, all execute exactly the same number of times, so
// they can share one counter and one runtime probe. That sharing is what
// shrinks emitted shader code: probe count, not counter width, is what
// scales SPIR-V size.
//
// `outSlots[i]` is the counter index assigned to `markerOps[i]`.
// `outEmitsProbe[i]` selects the single marker per group that emits the
// runtime counter update; it is placed at the *last* marker of the group so
// that reaching it proves every earlier marker in the group executed
// (placing it first would over-report a group entered but abandoned
// partway). `outCounterCount` is the number of distinct slots assigned.
// Function and branch markers always take a dedicated slot.
void assignCoverageCounterSlots(
    List<IRInst*> const& markerOps,
    List<UInt>& outSlots,
    List<bool>& outEmitsProbe,
    UInt& outCounterCount);

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
