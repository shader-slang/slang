# Legalize the remaining canonical scalar intrinsic recipes

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the 21 healthy-MVP workloads whose first direct-NVVM blocker was ordinary
`IRGenericAsm` use compiler-owned typed recipes instead of CUDA source snippets. The complete
measured family consists of Half bit transport and conversion, Double word pack/unpack, scalar
floating classification, `sincos`, and `frexp`. Exact final assembly plus the complete specialized
helper signature selects each recipe; fixture paths and source intrinsic names never participate.

The compiler composes recipes from the current generic value, constant, load/store, and return
operations. A provider ABI revision is allowed only if a concrete `frexp` projection cannot be
expressed exactly through revision 27. Every workload that becomes differentially correct at both
O0 and O3 receives explicit direct regression lanes. Later first blockers remain failures and are
recorded in the fixed 452-workload census.

## Progress

- [x] (2026-08-30) Committed Slice 137 as `d022a8436`; healthy-MVP both-mode correctness is
  278/427 with zero old-correct regression.
- [x] (2026-08-30) Repartitioned the remaining ordinary-GenericAsm bucket into 12 exact
  assembly/signature pairs across 21 healthy MVP workloads.
- [x] (2026-08-30) Selected the whole coherent scalar-intrinsic family rather than another
  one-operation slice; traced its canonical producer to CUDA intrinsic expansion and final linked
  one-block helpers.
- [x] (2026-08-30) Added one focused source containing all 18 exact recipe signatures plus an exact
  wrong-out-parameter-type negative.
- [x] (2026-08-30) Implemented one compiler-side classification and recipe layer using queried
  generic builder operations.
- [x] (2026-08-30) Proved the missing `frexp` projection contract with real LLVM/libdevice tests,
  advanced the forward-only provider ABI to revision 28, and rebuilt both Release targets outside
  the sandbox.
- [x] (2026-08-30) Reran all 21 original rows, promoted 19 both-mode successes and one O3-only
  success, then regenerated the complete census and Pareto evidence.
- [x] (2026-08-30) Ran selected regression, representative compile/PTX/runtime, SM70/80/90 assembly,
  formatting, diff, and producer-side self-review gates, then commit the slice.

## Surprises and Discoveries

- The broad 21-row label contains only 12 exact assembly/signature pairs. Five workloads stop at
  Double-to-two-UInt decomposition, three at packed UInt-to-Float Half conversion, two at
  Float-to-packed-Half conversion, two at two-UInt-to-Double construction, three at `sincos`, two
  at `frexp`, two at exact 16-bit-to-Half reinterpretation, and two at classification.
- Revision 27 already exposes typed bit reinterpretation, integer width conversion, shifts,
  Boolean/integer comparisons, bitwise operations, Float/Half conversion, scalar sine/cosine,
  constants, pointer stores, and void/value returns. Passing assembly text to LLVM or adding one
  callback per intrinsic would duplicate that interface.
- `sincos` is not a new provider semantic: the canonical helper writes the results of the already
  supported typed sine and cosine operations to two out parameters.
- `frexp` has a scalar floating result and an integer out result. Exact handling of zero,
  subnormal, infinity, and NaN makes an ad-hoc compiler bit decomposition high risk. The real
  libdevice signature must determine whether two queried value projections are the smallest
  honest extension.
- The initial 12 first-blocker pairs exposed six adjacent, still-in-family signatures after
  legalization: Half-to-bits plus Float16/Float32/Float64 finite/infinite classification. The
  bounded resolver therefore contains 18 exact assembly/signature rows, not an open-ended parser.
- Nineteen workloads become correct in both modes. `bit-cast-16-bit` is additionally correct at O3
  but O0 reaches libNVVM's existing unoptimized-Half operation failure; `scalar-half` advances to
  its later `$P_min` Half overload, outside this slice's scalar-recipe family.

## Decision Log

- Decision: address all 21 remaining ordinary scalar intrinsic helpers in one vertical slice.
  Rationale: they share one canonical producer, one exact recognition boundary, and one reusable
  compiler-side recipe representation. Splitting each spelling into a separate slice would repeat
  validation without establishing a broader invariant.
  Date/author: 2026-08-30, Codex.
- Decision: retain final assembly and complete linked signature as the semantic key.
  Rationale: `StmtLoweringVisitor::visitIntrinsicAsmStmt` and CUDA specialization deliberately
  produce these one-block helpers. The finalized signature supplies types and out-parameter roles;
  neither fixture identity nor reconstructed source syntax is required.
  Date/author: 2026-08-30, Codex.
- Decision: express compound behavior in the compiler through existing generic operations.
  Rationale: Half transport, Double word transport, classification, and `sincos` are exact bounded
  compositions of operations revision 27 already queries and emits. This keeps the isolated LLVM
  provider economical and prevents semantic recognition from drifting across two layers.
  Date/author: 2026-08-30, Codex.
- Decision: require a concrete libdevice proof before adding `frexp` operation IDs.
  Rationale: the current typed callback cannot pass an out pointer as a semantic operand. If exact
  fraction/exponent projections require provider-local storage around `__nv_frexp[f]`, two generic
  operation IDs are preferable to a new callback family, but only after real LLVM/libNVVM tests
  prove the need and contract.
  Date/author: 2026-08-30, Codex.
- Decision: advance the provider ABI to revision 28 with two generic value-operation IDs for the
  `frexp` fraction and exponent projections.
  Rationale: the existing one-result typed callback cannot represent libdevice's exponent out
  pointer, but it can represent each semantic projection. Provider-local temporary storage keeps
  LLVM pointers out of the shared ABI and preserves libdevice special-value behavior without an
  intrinsic-specific callback family.
  Date/author: 2026-08-30, Codex.
- Decision: promote `bit-cast-16-bit` only at O3.
  Rationale: its recipe is valid and differentially correct after optimization, while the O0
  module deterministically reaches the separately measured libNVVM Half-operation limitation. An
  O0 lane would claim support the backend does not have.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The fixed 452-workload census gains 19 exact O0 successes and 20 exact O3 successes with no lost
Slice-137 success identity. Direct totals are 298 O0 and 303 O3. Among 427 healthy MVP references,
O0/O3/both correctness is 297/301/297 (69.6%/70.5%/69.6%). The ordinary-GenericAsm healthy-MVP
cluster falls from 21 to one; that remaining workload is the later Half minimum overload.

All 59 CUDA lanes in the promoted files pass: 20 native references and 39 explicit direct lanes.
The selected NVVM prefix passes 406/406. All three representative workloads remain correct, and
their direct O3 PTX assembles with CUDA 12.9 for SM70, SM80, and SM90. CUDA 13 tooling and physical
SM70/80/90 workers remain infrastructure gaps. The full report is
`report.slice-138-canonical-scalar-intrinsic-recipes.md`.

## Context and Current Pipeline

Consider the finalized helper for HLSL `asuint(double, out uint, out uint)`:

```text
GenericAsm("$P_asuint($0, $1, $2)")
signature=Void(double, OutParam<uint>, OutParam<uint>)
```

CUDA intrinsic selection creates the helper, specialization fixes Double and UInt types, and
linking retains it as a one-block `IRFunc`. `_validateNVVMFunction` currently reaches the generic
assembly terminator but no scalar-value row can represent two output stores. The existing builder
can instead reinterpret Double as UInt64, truncate the low word, shift/truncate the high word,
store both UInt32 values, and return void.

The other recipes have the same ownership. Packed Half helpers combine exact UInt16/Half bit
reinterpretation with existing width conversion. `asdouble` combines two UInt32 words in UInt64
before bit reinterpretation. Classification uses exact exponent/mantissa bit tests. `sincos` emits
the selected sine and cosine semantics and stores both results. `frexp` alone needs a libdevice
operation that produces two logical results.

## Scope and Non-Goals

In scope are the 18 measured exact assembly/signature pairs; Float16/Float32 conversion recipes;
signed/unsigned 16-bit Half reinterpretation; Float64/UInt32 word transport; Float32 finite and
Float16 NaN classification; Float32/Float64 `sincos`; Float32/Float64 `frexp`; typed preflight;
focused fake and real-provider coverage; fixture promotion; and complete census/representative
evidence.

Out of scope are arbitrary assembly parsing, vectorized variants not present in the measured
canonical family, wave/reconvergence assembly, atomics, textures, FP8/BFloat16, source-name or
fixture checks, syntax reconstruction, compatibility ABI branches, and downstream repair of
malformed helpers.

## Architecture and Invariants

- Only a one-block helper whose sole executable instruction is the final `IRGenericAsm` terminator
  can become a scalar intrinsic recipe.
- Assembly text and the entire specialized result/parameter signature, including out-parameter
  pointee types and order, select one recipe. Near matches fail deterministic preflight.
- Each recipe declares every typed value operation it will emit. Requirement collection queries
  those descriptors before provider discovery or module mutation.
- Out parameters are canonical helper pointers. The compiler obtains their existing lowered
  handles and uses the ordinary typed store operation; it does not rebuild source `out` syntax.
- Bit transport is expressed by exact reinterpretation, width conversion, shifts, masks, and
  stores. No numeric conversion substitutes for a bitwise contract.
- `frexp` must match native CUDA for zero, negative zero, normals, subnormals, infinity, and NaN.
  Approximate arithmetic reconstruction is not admissible.

## Interfaces and Dependencies

Primary compiler work is in `source/slang/slang-emit-nvvm.cpp` and the shared semantic catalog.
Only a proven `frexp` gap may change `source/compiler-core/slang-nvvm-ir-builder-api.h` and
`source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`. Fake and real provider coverage belongs in the
existing NVVM unit-test files. Promoted direct lanes belong beside the 21 existing corpus sources.

Validation uses the Release host and isolated provider builds outside the sandbox, CUDA 12.9.86
and the local RTX 5090/SM120 runtime, and CUDA 12.9 `ptxas` for SM70/SM80/SM90. CUDA 13 and physical
SM70/SM80/SM90 runtime workers remain productionization gaps.

## Milestones

1. Encode every exact final helper topology as a typed recipe and add malformed-signature/text,
   wrong-width, and unsupported-vector negatives.
2. Implement and query the revision-27 compositions for Half transport, Double word transport,
   classification, and `sincos`; validate them with the fake provider before real LLVM work.
3. Prove the exact Float32/Float64 `frexp` contract against the isolated provider and libdevice.
   Add only the minimal generic semantic IDs if no revision-27 composition is exact.
4. Run the original 21-workload family at O0/O3, diagnose every transition, and promote every
   both-mode differential success.
5. Regenerate the fixed census/Pareto report, compare exact success identities with Slice 137,
   rerun representative metrics and SM70/80/90 assembly, update durable documentation, self-review,
   format, and commit.

## Validation and Acceptance

Acceptance requires focused fake-provider topology for all recipes and adjacent negatives; real
LLVM/provider tests for every new semantic; Release host/provider builds outside the sandbox;
O0/O3 native-differential correctness for every promoted workload; zero unexplained Slice-137
correct-workload regressions; the complete selected NVVM prefix; all three representative gates;
regenerated 452-row census and Pareto evidence; representative compile-time/PTX/runtime metrics;
CUDA 12.9 SM70/80/90 PTX assembly; formatting; `git diff --check`; and the repository's
producer-side input-shape audit.

The selected NVVM unit prefix remains a regression score. Coverage claims use the fixed census and
427 healthy-MVP denominator.

## Failure and Recovery

If a recognized helper reaches a later unsupported operation, classify that next producer and do
not count it as unlocked. If a recipe fails libNVVM verification or differs at runtime, preserve
the generated module/PTX and narrow the exact contract rather than rewriting emitted text. If
`frexp` cannot fit the generic operation callback honestly, stop and record the concrete interface
gap before adding a new callback family.

## Self-Review

Inventory every new resolver, recipe kind, operation ID, and emission branch. For each, record the
exact final helper shape, canonical producer, why that shape is valid, which focused and corpus
tests fail without it, and why compiler or provider owns it. Remove fixture/source-name checks,
assembly substring parsing, duplicated type tables, syntax reconstruction, compatibility
fallbacks, and unqueried emitted operations. Perform a revert drill on the central recipe resolver
when practical.

## Artifacts and Hand-Off

Keep final IR, generated LLVM/NVVM IR, PTX, cubins, raw family results, and timing samples under
ignored `build/nvvm-census/slice138-*`. Commit the completed plan with implementation, focused
tests, fixture promotions, regenerated census TSV/Pareto JSON, report, and durable design/ledger
updates. The outcome must quantify how much of the 21-row cluster becomes correct and identify the
next largest producer-owned family.
