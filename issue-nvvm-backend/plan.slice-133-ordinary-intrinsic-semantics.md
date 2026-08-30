# Legalize the dominant ordinary intrinsic semantics

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the largest coherent family inside the 66-workload MVP ordinary-intrinsic
`GenericAsm` cluster is represented as exact typed semantics and emitted through the direct NVVM
path. Unsupported assembly reports its canonical assembly spelling and final helper signature, so
the retained Pareto rows identify a semantic operation rather than the undifferentiated
`GenericAsm` container.

The slice starts with the complete 66-row denominator and selects operations by measured workload
unlock and semantic reuse. It may extend the existing generic value-operation catalog and callback,
but it will add no fixture checks and no operation-specific provider callback. Builder ABI revision
24 remains unchanged if the existing operation IDs suffice; otherwise one forward-only revision
extends the generic operation enum only for concrete semantics that cannot be expressed today.

## Progress

- [x] (2026-08-30) Committed Slice 132 as `69d60214d`; the full 452-row census identifies 66 MVP
  ordinary-intrinsic `GenericAsm` failures, the largest remaining root-cause cluster.
- [x] (2026-08-30) Added a deterministic canonical `GenericAsm` diagnostic, reran all 66 rows at O0/O3, and grouped
  exact assembly/signature pairs independently of source or fixture name.
- [x] (2026-08-30) Selected scalar minimum/maximum, the 17-row largest coherent family, and traced each canonical producer from
  the CUDA intrinsic expansion to final linked helpers, and prove which existing generic builder
  operations can express it.
- [x] (2026-08-30) Added one typed semantic representation across catalog, preflight, requirement collection,
  emission, fake provider, and real LLVM 14/NVVM IR 2.0 provider paths. Revise the ABI only for the
  exact operation IDs missing from revision 24.
- [x] (2026-08-30) Reran the original 66 rows and full denominator at O0/O3, promoted every newly correct fixture,
  update coverage/Pareto evidence and representative measurements, format, validate, self-review,
  and commit Slice 133.

## Surprises and Discoveries

- The current census records the canonical container (`GenericAsm`) but not its assembly spelling
  or final helper signature. That was sufficient to rank the broad cluster, but it is not sufficient
  to choose a principled semantic vertical slice. Improving this diagnostic is measurement work,
  not speculative feature admission.
- The generic value-operation callback is already typed and queried. Several likely rows—wider bit
  reinterpretation, floating conversion, and square root—may need only catalog entries. Other
  ordinary math/bit intrinsics may require new operation IDs, but not a new callback shape.
- Exact O0/O3 inventories agree: 66 rows contain 48 assembly/signature pairs. Minimum/maximum is
  the largest coherent first-blocker family at 17 rows: Int32 maximum (6), Float32 maximum (4),
  Float64 maximum (4), UInt32 maximum (1), Int32 minimum (1), and Float64 minimum (1).
- Floating minimum/maximum cannot use comparison plus selection without changing NaN and
  signed-zero behavior. Exact libdevice calls are part of the semantic contract, not an
  optimization or provider workaround.
- Of the 17 first blockers, eight become correct and nine expose later exact intrinsic blockers.
  Coverage counts only the eight differential successes.

## Decision Log

- Decision: prioritize the 66-row ordinary-intrinsic cluster ahead of the 31-row wave cluster and
  28-row residual helper cluster.
  Rationale: it is the largest remaining MVP blocker and contains ordinary arithmetic/libdevice
  behavior expected across real compute workloads. Wave operations require a separate convergence
  contract, while remaining helper rows mix unrelated address-space, borrow, resource, and deferred
  scalar families.
  Date/author: 2026-08-30, Codex.
- Decision: classify by final assembly text plus exact linked helper signature.
  Rationale: `StmtLoweringVisitor::visitIntrinsicAsmStmt` and CUDA intrinsic expansion produce
  `IRGenericAsm`; after specialization, the assembly string and concrete `IRFunc` signature are the
  semantic source of truth. Source intrinsic names and fixture paths are not.
  Date/author: 2026-08-30, Codex.
- Decision: admit canonical scalar minimum/maximum as one parameterized family and advance the
  forward-only builder ABI to revision 25 with only `MIN` and `MAX` operation IDs.
  Rationale: the exact same-type scalar topology fits the existing generic typed callback. No
  revision-24 operation has minimum/maximum semantics, so two new IDs are the smallest honest ABI
  change; a new operation-specific callback would duplicate the existing interface.
  Date/author: 2026-08-30, Codex.
- Decision: lower integer minimum/maximum to typed compare/select and Float32/Float64 to exact
  libdevice `fmin`/`fmax` calls.
  Rationale: signedness is already explicit in integer descriptors. Floating comparison/select is
  not semantically equivalent for NaNs or signed zero, while libdevice defines the required CUDA
  behavior and is already selected before provider mutation.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The slice is complete. Exact diagnostics divide the original 66 ordinary-intrinsic failures into
48 assembly/signature pairs. Scalar minimum/maximum accounts for 17 first blockers and is now one
typed compiler/provider family. ABI revision 25 adds only generic `MIN` and `MAX` operation IDs;
integer overloads use compare/select and Float32/Float64 use exact libdevice calls.

Eight existing workloads become correct at both O0 and O3 and receive direct regression
directives. Nine more reach later `fmod`, Half-conversion, `round`, `abs`, or `countbits` blockers
and remain failures. The complete 452-row census reaches 226 O0 and 222 O3 successes, with 214
preflight failures at each level and zero old-correct regressions. On 427 healthy MVP references,
225 compare correctly at O0, 220 at O3, and 217 at both. The remaining leading MVP clusters are
ordinary intrinsic semantics (58), wave/reconvergence semantics (31), helper ABI types (28),
aggregate/pointer/layout transport (23), and ordinary numeric/bit operations (16).

Release host/provider builds, focused fake/real-provider tests, the selected NVVM prefix, promoted
fixtures, the full census, and representative NVRTC/direct compile-size measurements pass. All
three representative direct O3 modules assemble for SM70, SM80, and SM90. CUDA 13 and physical
SM70/80/SM90 runtime workers remain infrastructure gaps. The direct lanes in the fixture-wide
minimum/maximum run pass; its unrelated existing WGPU lane still fails Dawn bind-group validation,
and the exact promoted direct indices pass independently.

## Context and Current Pipeline

Consider an ordinary intrinsic such as a floating classification, bit-count, bit reinterpretation,
or scalar math call. CUDA target intrinsic expansion produces a helper containing `IRGenericAsm`.
Linking and specialization retain a concrete helper signature and the CUDA assembly template.
`_validateNVVMFunction` currently asks `_findNVVMGenericAsmSemantic` to match this pair against
`NVVMSemantics::kCatalog`; unmatched ordinary semantics stop as E52017 `GenericAsm`.

For a matched row, the catalog supplies a complete typed operation descriptor and whether libdevice
is required. Requirement collection requests the selected toolkit library before provider mutation.
Emission gathers the exact SSA operands and calls the generic builder operation interface. The
isolated provider validates the same catalog row, emits an LLVM instruction/intrinsic or exact
libdevice declaration/call, then serializes semantic NVVM IR 2.0 for CUDA 12.9 libNVVM. This slice
must extend that end-to-end source of truth rather than recognizing a source function name.

## Scope and Non-Goals

In scope are exact diagnostics for unmatched ordinary `GenericAsm`; a complete 66-row semantic
inventory; one measured coherent family of scalar/vector bit, conversion, floating, classification,
or libdevice operations; typed compiler/catalog/provider implementation; adjacent negative tests;
real CUDA 12.9 PTX/runtime validation; promotion of every unlocked workload; and a complete census
delta.

Out of scope are wave/reconvergence assembly, texture/surface operations, atomics, device clock,
source-name checks, parsing arbitrary assembly as a programming language, source syntax
reconstruction, compatibility callbacks, unrelated helper/address-space widening, FP8/BFloat16,
and downstream patches for malformed IR. A workload may advance to an out-of-scope next blocker
without being counted as unlocked.

## Architecture and Invariants

- The final `IRGenericAsm` text and linked `IRFunc` signature together identify one exact semantic.
  Neither is interpreted without the other.
- One exact catalog row or parameterized family resolution owns compiler query, provider query,
  operand/result validation, libdevice demand, and emission. There is no parallel switch that
  independently decides legality.
- Ordinary IR operations continue to reuse the same typed operation descriptors where their
  semantics agree. GenericAsm does not create a second arithmetic type system.
- A new operation ID is justified only when no revision-24 ID has the required semantics. The
  existing generic query/emit callback remains the interface unless a concrete result topology
  proves it insufficient.
- Unsupported text/signatures stop before provider discovery with a deterministic diagnostic.
  Provider failures are retained and clustered, not converted into preflight success claims.

## Interfaces and Dependencies

Expected compiler files are `source/compiler-core/slang-nvvm-semantic-catalog.h`,
`source/slang/slang-emit-nvvm.cpp`, and, only if exact missing operation IDs require it,
`source/compiler-core/slang-nvvm-ir-builder-api.h`. The provider implementation is
`source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`; fake and real coverage live in the existing NVVM unit
test files. Census scripts and committed evidence remain under `issue-nvvm-backend/`.

The Release host and isolated provider builds run outside the sandbox. Runtime validation uses CUDA
12.9.86 and the RTX 5090/SM120 host; representative direct O3 modules are assembled for SM70, SM80,
and SM90. CUDA 13 and physical SM70/80/90 runtime workers remain infrastructure gaps.

## Milestones

1. Diagnose the exact assembly text and canonical helper result/parameter types for all 66 rows at
   both optimization levels. Group identical semantics and record how many workloads each blocks.
2. Select the largest coherent group whose result/operand topology fits the generic value-operation
   interface. Add focused negative tests for wrong text, arity, widths, lane counts, and libdevice
   requirements before widening production admission.
3. Extend the shared typed catalog and provider implementation. Reuse existing operation IDs where
   semantically exact; if new IDs are necessary, increment the forward-only ABI and update every
   current-interface negotiation test without a compatibility branch.
4. Run focused fake/real-provider, PTX assembly, and differential runtime tests. Rerun the original
   66-row denominator, inspect every transition, and retain only changes explained by the selected
   semantic family.
5. Promote all newly correct rows, regenerate the 452-plus-workload census/Pareto evidence, run the
   selected 400-plus regression and representative gates, update durable design records, and commit.

## Validation and Acceptance

Acceptance requires a complete exact-semantic inventory for the 66 rows; typed positive and
adjacent-negative tests; Release host/provider builds; LLVM 14 and semantic NVVM IR 2.0 provider
coverage; CUDA 12.9 libNVVM compilation and `ptxas`; O0/O3 differential correctness for every
promoted workload; zero unexplained old-correct regressions; all three representative gates; the
complete selected NVVM prefix; regenerated census and Pareto evidence; formatting; `git diff
--check`; and no staged `external/slang-binaries/` content.

## Failure and Recovery

If an admitted semantic reaches a later failure, classify that canonical next blocker and keep it
out of the success numerator. If the existing callback cannot represent a result topology, preserve
the smallest real proof before revising the interface. If libNVVM rejects an LLVM intrinsic or
dialect spelling, use the exact CUDA 12.9 log to choose a documented libdevice mapping or narrow the
catalog; do not rewrite text post hoc. Generated mirrors, IR dumps, PTX, cubins, and raw inventories
remain below ignored `build/nvvm-census/`.

## Self-Review

- `MIN` and `MAX` survive as the only new operation IDs. The 17 measured canonical scalar helpers
  fail without them; revision 24 has no equivalent semantics. No callback or compatibility path was
  added.
- `_diagnoseUnsupportedGenericAsm` survives as measurement infrastructure. It reports the exact
  final producer text/signature, escapes diagnostic delimiters, and does not parse or admit text.
  The 48-pair inventory and malformed-helper unit cases prove this preflight layer owns it.
- `_resolveNVVMGenericAsmMinMax` survives as the one canonical-shape recognizer. The CUDA prelude
  produces a one-block helper with exact `$P_min`/`$P_max` text, two same-type parameters, and the
  same result type. Wrong arity and nonselected scalar types retain E52017. No source or fixture name
  participates.
- `ValueOperationFamilyResolution::requiresCUDADeviceLibrary` survives because the shared overload
  resolver must own both legality and pre-mutation library demand. Integer descriptors leave it
  false; scalar Float32/Float64 descriptors set it true. An initially overbroad selected-integer
  vector admission was removed and an adjacent negative test now fixes the scalar boundary.
- `_getLibdeviceFunctionName` and `_emitLibdeviceOperation` survive as a refactor of the former unary
  helper. They keep the operation-to-symbol map in one provider location and validate exact typed
  operands. Floating comparison/select was rejected because it changes NaN and signed-zero
  behavior.
- No fixture checks, syntax reconstruction, fallback, downstream malformed-IR patch, or
  provider-only unqueried overload remains. Removing min/max resolution restores the eight corpus
  failures; removing exact libdevice demand breaks the floating fake/real-provider tests.

## Artifacts and Hand-Off

Commit the completed plan, implementation, promoted fixtures, post-slice census table, Pareto JSON,
and slice report. Keep raw 66-row semantic inventories and generated artifacts under ignored
`build/nvvm-census/slice133-*`. The outcome must state the exact number of workloads unlocked and
the next general invariant selected from the updated denominator.
