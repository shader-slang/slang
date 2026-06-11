#ifndef SLANG_IR_COVERAGE_INSTRUMENT_H
#define SLANG_IR_COVERAGE_INSTRUMENT_H

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
// current line/function/branch producers assign one direct counter slot
// per marker op; the metadata keeps entry count and counter count
// separate so later source-region coverage can use shared or derived
// counters. Marker kind selects the emitted source-entry metadata:
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

} // namespace Slang

#endif // SLANG_IR_COVERAGE_INSTRUMENT_H
