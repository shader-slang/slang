# Legalize canonical Half scalar math through Float32 semantics

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the canonical CUDA helpers used by ordinary scalar Half math no longer stop at
one unsupported `IRGenericAsm` operation at a time. The compiler recognizes the complete exact
Half helper signatures reached by `tests/hlsl-intrinsic/scalar-half.slang`, promotes Half inputs to
Float32, evaluates the corresponding already-typed Float32 semantic, and narrows a floating result
back to Half. The native CUDA/NVRTC result and direct NVVM O0/O3 results must agree before the
fixture receives permanent direct lanes.

The slice establishes one reusable invariant rather than a fixture exception: CUDA implements
scalar Half library math through Float32 evaluation because every finite Half value is represented
exactly by Float32. Exact helper text plus the complete specialized signature remains the semantic
key. Any operation that cannot be represented exactly with the revision-32 generic provider
interface must be proved as a concrete gap before the ABI is revised.

## Progress

- [x] (2026-09-01) Resumed from clean Slice 176 commit `c9607cf0d` and confirmed the only unrelated
  worktree item is the pre-existing untracked `external/slang-binaries/` directory.
- [x] (2026-09-01) Traced the first blocker to the canonical one-block `$P_min($0, $1)` helper with
  signature `half(half, half)` and confirmed the existing catalog deliberately admits libdevice
  minimum/maximum only for Float32 and Float64.
- [x] (2026-09-01) Selected the whole bounded scalar-Half math chain in `scalar-half` rather than a
  single minimum spelling.
- [x] (2026-09-01) Followed the first-failure chain through minimum, hyperbolic functions, FMA,
  `frexp`, and `modf`, then recorded their exact value and out-parameter topologies.
- [x] (2026-09-01) Implemented one homogeneous scalar-Half promotion recipe plus explicit Half
  `frexp`/`modf` recipes; added only the six typed provider semantics proven absent.
- [x] (2026-09-01) Passed native/O0/O3 differential execution, promoted two stable lanes, replayed
  both corpora, ran permanent and selected regressions, assembled SM70/80/90 measurements,
  formatted with clang-format 17, and completed the producer-side self-review.

## Surprises and Discoveries

- The resolver already maps `$P_min` and `$P_max` to generic semantic IDs. Rejection occurs because
  `resolveValueOperationFamily` restricts libdevice minimum/maximum to scalar Float32/64, not
  because the canonical producer or provider callback is missing.
- `scalar-half` deliberately combines NaN, infinity, signed-zero, transcendental, decomposition,
  and fused-operation behavior. Passing it will test the promotion invariant much more strongly
  than a synthetic one-operation fixture.
- The exact diagnostic progression was `$P_min half(half,half)`, `$P_sinh half(half)`, `$P_fma
  half(half,half,half)`, `$P_frexp half(half,OutParam<int>)`, and `$P_modf
  half(half,OutParam<half>)`. Each newly exposed stop belonged to the same bounded producer family.
- LLVM's FMA intrinsic did not serialize to the legacy NVVM dialect accepted by this toolchain.
  The exact libdevice `__nv_fmaf`/`__nv_fma` calls preserve fusion and serialize in both provider
  assembly modes.
- Discovery requires the immutable Slice-146 corpus-v1 artifact for its overlap/denominator guard.
  Passing a newer result artifact correctly fails because its correctness fields have advanced.

## Decision Log

- Decision: make Slice 177 a bounded vertical slice for the scalar-Half math chain in
  `scalar-half`, not a one-operation minimum patch.
  Rationale: all relevant helpers share the same CUDA intrinsic producer, exact one-block helper
  representation, and Half-to-Float32 evaluation contract. Treating each spelling separately
  would repeat the same type legalization and validation boundary.
  Date/author: 2026-09-01, Codex.
- Decision: keep Half promotion in compiler-side recipe classification.
  Rationale: the provider already exposes generic Float conversion and Float32 math operations.
  The compiler owns the source semantic type and helper topology, while the provider should remain
  unaware that a Float32 operation originated from a Half helper.
  Date/author: 2026-09-01, Codex.
- Decision: revise the provider ABI from 32 to 33 for hyperbolic functions, FMA, and `modf`
  projections only.
  Rationale: revision 32 can express conversion, min/max, classification, and most ordinary math,
  but has no semantic IDs for these canonical operations. Text reconstruction or multiply/add
  substitution would change semantics. Pure `modf` projections keep LLVM-local temporary pointers
  behind the provider boundary.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

The bounded workload passes native CUDA and direct NVVM O0/O3 and now owns two permanent direct
lanes. Frozen corpus v1 remains exactly 452 rows/427 healthy references and advances from
414/414/414 to 415/415/415 O0/O3/both; `scalar-half` is the only changed row and there are no
old-correct regressions. Discovery remains exactly 82 rows/72 healthy references at 72/72/72 with
no changed row. All-row frozen direct totals are 429 correct, three runtime mismatches, and 20
preflight failures in each mode.

The selected prefix passes 434/434 and the permanent `nvvm` category passes 84/84. Native, direct
O0 SM70, and direct O3 SM70/SM80/SM90 PTX all assemble through CUDA 12.9. At SM70 the exploratory
one-repetition measurements are 589.8 ms/64,486 bytes native, 329.8 ms/126,480 bytes direct O0,
and 382.8 ms/33,284 bytes direct O3.

## Context and Current Pipeline

Consider this source expression from `scalar-half.slang`:

```slang
min(0.0h, 0.0h / 0.0h)
```

CUDA target selection in `source/slang/hlsl.meta.slang` chooses
`__intrinsic_asm "$P_min($0, $1)"`. `StmtLoweringVisitor::visitIntrinsicAsmStmt` produces a final
`IRGenericAsm`, specialization fixes the helper signature to `half(half, half)`, and linking leaves
that instruction as the sole executable body of a one-block helper.
`_resolveNVVMGenericAsmValueOperation` in `source/slang/slang-emit-nvvm.cpp` already maps the exact
assembly spelling to `SLANG_NVVM_VALUE_OP_MIN`, but the semantic catalog rejects the Half overload
because libdevice exposes only Float32/64 minimum routines. `_validateNVVMFunction` therefore
reports the exact helper before provider discovery.

The principled legalization is a compiler-owned recipe: convert each Half parameter to Float32,
invoke the exact Float32 semantic through the existing queried operation interface, convert a
Float32 result back to Half, and return it. Classification/out-parameter operations may have a
different result topology and must receive an equally exact recipe rather than being forced into
that unary/binary template.

## Scope and Non-Goals

In scope are exact scalar Half helper shapes reached by `scalar-half.slang`; reusable recipe
classification; complete preflight requirement collection; fake-provider topology; real provider
coverage for any new semantic; native/O0/O3 differential validation; fixture promotion; and the
frozen/discovery corpus, selected-prefix, representative, architecture, and formatting gates.

Out of scope are vectors or matrices not reached by the bounded workload, BFloat16 and FP8,
half2 atomic assembly, arbitrary GenericAsm parsing, source intrinsic or fixture-name checks,
textual LLVM rewriting, compatibility ABI branches, and downstream repair of malformed helpers.

## Architecture and Invariants

- Only a one-block helper whose sole executable instruction is final `IRGenericAsm` is eligible.
- Exact assembly plus the complete specialized signature selects a recipe; substring parsing and
  source/fixture identity are forbidden.
- Half operands are converted numerically to Float32. This is exact for all Half values; the
  selected Float32 operation owns the math semantics and a floating result is narrowed once.
- Every primitive recipe step is queried during requirement collection before builder discovery or
  module mutation. Libdevice is linked if any promoted Float32 operation requires it.
- Result topology remains explicit. Integer results and pointer out parameters are not treated as
  Half-returning operations.
- Unsupported near-matches retain deterministic producer/type/operation diagnostics.

## Interfaces and Dependencies

Primary work belongs in `source/slang/slang-emit-nvvm.cpp` and, if a reusable operation is absent,
the shared semantic catalog and forward-only provider API, now revision 33. Tests belong in the
established NVVM emitter/provider unit files and beside `tests/hlsl-intrinsic/scalar-half.slang`.

The environment uses the Release host build, isolated LLVM 14 provider at
`build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`, CUDA 12.9.86, the local SM120 GPU, and
CUDA 12.9 `ptxas` for SM70/80/90 assembly checks. Builds and tests run outside the sandbox.

## Milestones

1. Use the exact diagnostics/final linked shapes to inventory the complete Half helper chain,
   separating ordinary unary/binary/trinary results from out-parameter recipes.
2. Add a single typed promotion-recipe representation and focused positive/negative fake-provider
   coverage. Reuse existing conversion, value-operation, store, and return operations.
3. For each exact semantic the generic interface cannot express, prove the gap with the real
   provider, then add only the smallest generic operation ID and provider implementation.
4. Run `scalar-half` through native CUDA and direct NVVM O0/O3, audit every newly exposed blocker,
   and promote lanes only after exact differential correctness.
5. Regenerate frozen corpus v1 and discovery artifacts without changing either denominator; run
   selected/permanent regressions, representative measurements, SM70/80/90 PTX assembly,
   formatting, diff review, and the producer-side input-shape audit.

## Validation and Acceptance

Acceptance requires focused unit tests that prove exact promotion topology and reject adjacent
wrong signatures; Release host/provider builds outside the sandbox; native/O0/O3 correctness for
every promoted lane; frozen corpus v1 still exactly 452 rows and 427 healthy-MVP rows with no
old-correct regression; discovery still exactly 82 rows and 72 healthy references; complete
selected-prefix and permanent-category passes; representative compile-time/PTX/runtime and
SM70/80/90 assembly evidence; pinned formatting; and `git diff --check`.

The selected prefix remains a regression count, not a coverage denominator. Corpus v1 and
discovery metrics must be reported separately.

## Failure and Recovery

If a Half helper exposes an operation with materially different semantics, record that exact first
shape and either add an explicit typed recipe within this slice's common promotion invariant or
leave it for a separately justified slice. If promotion changes observable behavior relative to
native CUDA, preserve the generated IR/PTX and stop widening the family. Do not replace exact
semantics with multiply/add, textual rewriting, or fixture-specific handling.

All implementation changes are additive to the direct backend and can be reverted without
affecting NVRTC. Census and metric runs write only below ignored `build/nvvm-census` paths and are
safe to rerun.

## Artifacts and Hand-Off

Keep linked IR, generated NVVM IR/PTX, cubins, diagnostic progression, and timing samples below
ignored `build/nvvm-census/slice177-*`. Commit the completed plan with implementation, tests,
fixture directives, regenerated TSV/JSON reports, and durable design/capability updates. The final
report must list the exact admitted helper signatures, provider ABI outcome, both corpus metrics,
and the next highest-value producer-owned cluster.
