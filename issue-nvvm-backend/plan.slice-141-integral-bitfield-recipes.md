# Lower integer truthiness and bitfield IR through generic recipes

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires this experimental branch's slice plan to be committed with its implementation, which is
an exception to the repository's default active-plan lifetime policy.

## Purpose and Observable Result

After this slice, the direct NVVM path lowers the first unsupported shape across the 11-workload
healthy-MVP ordinary-numeric/bit-operation cluster as two canonical IR families: scalar integer-to-Boolean
`intCast`, and selected integer scalar/vector `bitfieldExtract`/`bitfieldInsert`. The compiler
expresses both through revision-29 generic typed and structural builder operations. It does not add
a provider callback or reinterpret source syntax.

The shortest observable result is that both ordinary bitfield fixtures, the AnyValue layout
fixtures, and the scalar truthiness workloads that do not encounter a later unrelated blocker
compile and compare correctly through direct O0 and O3. A bounded cluster probe and the fixed
452-workload census record exact gains, later blockers, and zero previously correct regressions
before promotion.

## Progress

- [x] (2026-08-30) Recomputed the healthy-MVP Pareto after Slice 140: helper ABI/type contract
  blocks 16, aggregate/pointer/layout transport blocks 14, and ordinary numeric/bit operations
  block 11 at both modes.
- [x] (2026-08-30) Inventoried all 11 numeric rows. Four first stop at integer-to-Boolean
  `intCast`; two first stop at `bitfieldExtract`; five first stop at `bitfieldInsert`.
- [x] (2026-08-30) Traced final IR. `core.meta.slang` and AnyValue marshalling produce ordinary
  canonical `kIROp_BitfieldExtract`/`kIROp_BitfieldInsert`; integer truthiness remains a canonical
  scalar `kIROp_IntCast` from selected integer to Boolean.
- [x] (2026-08-30) Added bounded truthiness/bitfield recipe descriptors, shared
  preflight/emission construction, focused fake/negative/differential/ptxas tests, and the one
  required generic catalog widening for selected integer vector bitwise-not.
- [x] (2026-08-30) Probed all 11 workloads. Eight become correct at both modes; three advance to
  independent later blockers (`LoadFromUninitializedMemory`, `castFloatToInt`, and `makeUInt64`).
  Promoted seven files representing eight workload identities and regenerated census/Pareto and
  representative metrics.
- [x] (2026-08-30) Completed self-review, durable documentation, final validation, and the Slice
  141 commit preparation.

## Surprises and Discoveries

- `kIROp_IntCast` already maps to the generic integer-convert family, but that family intentionally
  returns integer values. Integer-to-Boolean casts are truthiness comparisons against typed zero,
  not width/sign conversions. The producer shape is valid; the direct resolver is missing the
  semantic recipe.
- The final bitfield fixtures include signed and unsigned scalar values plus two-, three-, and
  four-lane vectors. Offset and count are scalar UInt32 even for vector data. The provider's
  generic integer operations already support selected integer widths and scalar broadcast, while
  generic aggregate construction can materialize exact vector count/constant splats.
- Initial signed extraction must shift logically before optional sign extension. Reusing the
  signed `SHIFT_RIGHT` descriptor for that first step would silently change semantics. The recipe
  must operate on an unsigned mirror and bit-reinterpret only at the signed-extension boundary.
- Generic bitwise-not was catalogued only for scalar selected integers even though the provider's
  generic implementation already handles selected integer vectors. Canonical vector insertion
  needs a vector mask complement, so this slice widens that one shared legality row; it does not
  add a callback or duplicate the operation in the emitter.
- Resolving the first unsupported shape does not imply that every member of a census cluster is
  complete. The three non-promoted rows now expose distinct later canonical failures and move to
  their corresponding Pareto clusters instead of attracting speculative work here.

## Decision Log

- Decision: Take the 11-row numeric cluster before the larger but heterogeneous 16-row helper
  cluster.
  Rationale: All 11 numeric failures share two ordinary canonical IR families and can be removed
  with one bounded reusable representation. The helper rows split across references, resources,
  substandard floats, tuples, and pointer address spaces and need separate producer audits.
  Date/author: 2026-08-30, Codex.
- Decision: Compose existing typed operations in the compiler and retain provider ABI revision 29.
  Rationale: comparison, conversion, reinterpretation, shifts, masks, Boolean results, constants,
  and vector construction are all expressible through the current generic interface. A new
  bitfield callback would duplicate compiler-known semantics without closing an interface gap.
  Date/author: 2026-08-30, Codex.
- Decision: Accept only selected integer scalar/vector data with scalar UInt32 offset and count.
  Rationale: This is the exact canonical producer contract seen in the corpus and intrinsic
  declaration. Adjacent floating, Boolean, aggregate, mismatched, or vector-count shapes do not
  establish ownership for widening.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The bounded 11-row probe yields eight both-mode gains and no loss. Seven files representing eight
workload identities are promoted; `wave-active-count-bits.slang` contributes two native CUDA
workload identities. The three remaining rows advance to `LoadFromUninitializedMemory`,
floating-point `castFloatToInt`, and `makeUInt64` and are deliberately left to their owning
clusters.

The fixed 452-workload census reaches 338 correct at O0 and 343 at O3, up eight in each mode with
zero previously correct regressions. Against 427 healthy MVP references, O0/O3/both correctness is
336/340/336 (78.7%/79.6%/78.7%). The ordinary-numeric/bit-operation cluster is eliminated because
all residual rows now have later, more precise root causes. The selected prefix passes 413/413.

The recipe representation is useful for future compound ordinary IR operations: it keeps exact
canonical classification, capability closure, and emission in one descriptor while composing the
existing generic provider boundary. Provider ABI revision 29 remains sufficient.

## Context and Current Pipeline

Consider:

    uint r = bitfieldInsert(base, value, offset, count);
    int s = bitfieldExtract(signedValue, offset, count);
    bool active = intValue;

`source/slang/core.meta.slang` declares the first two as `kIROp_BitfieldInsert` and
`kIROp_BitfieldExtract`. `IRBuilder::emitBitfieldInsert`/`emitBitfieldExtract` also produce the same
ops for AnyValue packing and unpacking in `slang-ir-any-value-marshalling.cpp`. Checked integer
truthiness produces `kIROp_IntCast`. These ordinary instructions survive target legalization into
the final linked compute functions.

`_validateNVVMFunction` currently routes ordinary typed operations through
`_resolveNVVMValueOperation`. Integer-to-integer conversions resolve, but an integer-to-Bool
`intCast` does not. Bitfield ops have no validation or emission case and stop with deterministic
E52017 diagnostics. The canonical representations are valid and shared with the ordinary LLVM
emitter; the direct NVVM consumer is incomplete.

The new compiler-owned resolvers will validate complete result/operand types and build the exact
typed operation closure used by both preflight and emission. Integer truthiness becomes `value !=
zero`. Bitfield insert constructs a shifted mask, clears the base, masks the shifted insert, and
combines the two. Bitfield extract logically shifts an unsigned mirror, then either masks it or
performs explicit signed extension with left/arithmetic-right shifts. Scalar UInt32 offset/count
values are converted to the data element type and structurally splatted only when the data is a
vector.

## Scope and Non-Goals

In scope:

- selected 8-, 16-, 32-, and 64-bit signed/unsigned scalar and one-to-four-lane vector data;
- selected scalar integer-to-Boolean `kIROp_IntCast` truthiness;
- canonical `kIROp_BitfieldExtract(value, uintOffset, uintCount)`;
- canonical `kIROp_BitfieldInsert(base, insert, uintOffset, uintCount)`;
- signed extraction, vector data with scalar counts, deterministic adjacent-shape rejection;
- fixed-corpus promotion and coverage/metric refresh.

Out of scope:

- resource, pointer, floating-point, Boolean, matrix, or aggregate bitfield data;
- vector offset/count operands, clamping malformed ranges, or defining out-of-contract shifts;
- source-prelude rewriting, upstream syntax reconstruction, fixture-name checks, compatibility
  fallbacks, provider callbacks, or ABI revision 30;
- the separate helper ABI and aggregate/pointer/layout clusters.

## Architecture and Invariants

The resolver owns one complete canonical instruction contract. Result and data operands have the
same selected integer scalar/vector type. Offset and count are scalar UInt32. Integer truthiness
requires a selected integer scalar operand and scalar Boolean result. Every typed step is resolved
through `NVVMSemantics`; requirement collection runs before provider discovery.

Bitfield recipes preserve physical bits explicitly:

- insert is unsigned bit algebra regardless of source signedness, then reinterprets to the exact
  result type;
- extract performs its initial right shift on the unsigned mirror;
- unsigned extraction returns masked low bits;
- signed extraction places the sign bit at the physical high bit and uses one final signed
  arithmetic shift;
- vector constants and converted counts are exact structural splats, not provider-side implicit
  type guesses;
- preflight and emission use the same descriptor and type-derived step construction.

No recipe repairs malformed IR. Out-of-range offsets/counts remain subject to the established
front-end intrinsic contract; this slice does not add silent clamping or default values.

## Interfaces and Dependencies

No public API or provider ABI change is planned. `source/slang/slang-emit-nvvm.cpp` gains internal
integer truthiness and bitfield descriptors, resolvers, requirement collectors, vector-splat
materialization, and emitters. Existing `NVVMIRBuilder` integer constants, aggregate construction,
and typed value operations remain the provider boundary.

Focused fake sources and negative cases live in `tools/slang-unit-test/unit-test-nvvm-support.h`;
fake graph tests live in `unit-test-nvvm-emitter.cpp`; real differential and ptxas coverage live in
`unit-test-nvvm-integration.cpp`. Existing corpus fixtures receive direct lanes only after both
optimization modes compare correctly.

## Milestones

1. Add exact integer-truthiness resolution and emit compare-to-zero through the existing generic
   value-operation family. Prove scalar admission plus adjacent floating-point rejection.
2. Add one complete bitfield descriptor for signed/unsigned scalar/vector insert/extract, typed
   closure collection, vector splat construction, and generic emission.
3. Build and run focused fake/real differential/ptxas tests, then probe all 11 cluster workloads at
   O0 and O3. Record later blockers without speculative widening.
4. Promote every exact success, run the fixed census and representative metrics/SM70/80/90
   assembly, update durable design/capability evidence, self-review, format, validate, and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox. On this Windows machine use:

    cmake.exe --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release
    cmake.exe --build build --config Release --target slang-unit-test
    $env:SLANG_NVVM_BUILDER_PATH =
      'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
    .\build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm

Acceptance requires focused positive/negative fake-provider tests; signed, unsigned, scalar, and
vector real differential evidence; real `ptxas`; all newly promoted direct lanes; an exact
11-workload cluster probe; the fixed 452-workload census with zero old-correct regression;
representative metrics and SM70/80/90 assembly; clean pinned formatting and
`git.exe diff --check`; and the complete selected prefix.

## Failure and Recovery

The change is additive. If a selected type/lane shape fails, leave that exact shape outside the
resolver and record its producer and diagnostic rather than adding a fixture condition. If a
multi-step recipe mismatches NVRTC, compare the unsigned mirror, mask, count conversion, and final
signed-extension boundary against `LLVMBuilder::emitBitfieldExtract/Insert`; fix the shared recipe
invariant rather than patching a fixture.

Builds, probes, census, and metrics are safe to rerun. Removing the two new resolver/emitter paths
restores deterministic E52017 preflight without changing the provider binary or ABI negotiation.

## Self-Review

Inventory the descriptor, resolvers, requirements collectors, splat materializer, and emitters.
For each branch, record the exact canonical input shape and which corpus/focused test fails without
it. Reject any path that infers type semantics from an instruction consumer or rebuilds syntax.
Perform a revert drill on the shared bitfield resolver and integer-truthiness branch: focused
sources must return to deterministic preflight rather than another fallback.

## Artifacts and Hand-Off

Keep probe output under `build/nvvm-census/slice141-*`. Retain the fixed Slice 141 census TSV,
cluster JSON, five-part report, and this explicitly committed plan under `issue-nvvm-backend/`.
Distill stable recipe architecture into `docs/design/nvvm-backend.md` and exact measured status
into `docs/design/nvvm-backend-capability-ledger.md`. The next slice should re-rank the helper ABI,
aggregate/pointer/layout, residual marker, and atomic/wave clusters from the new census.
