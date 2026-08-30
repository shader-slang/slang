# Represent canonical integer bit operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the canonical CUDA helper bodies for scalar `countbits`, `reversebits`,
`firstbithigh`, and `firstbitlow` are represented as typed integer-bit semantics and emitted through
the direct NVVM path. The work starts from the post-Slice-133 exact inventory: these four operations
are the largest coherent remaining ordinary-intrinsic family, blocking 11 MVP census rows at O0 and
O3 across 16-, 32-, and 64-bit operands.

The slice will use the existing generic typed value-operation callback. It may add only the concrete
operation IDs that revision 25 cannot express and advance the forward-only ABI once; it will not add
an operation-specific provider callback, fixture checks, or a text-rewrite fallback. Every workload
that becomes differentially correct at both optimization levels will receive a direct regression
directive. Workloads that merely reach a later blocker will remain unpromoted and will be recorded
as such.

## Progress

- [x] (2026-08-30) Committed Slice 133 as `cb8c58d16`; the 452-row census reports 58 remaining MVP
  ordinary-intrinsic failures after scalar minimum/maximum.
- [x] (2026-08-30) Regrouped the exact post-Slice-133 diagnostics. Integer bit operations account
  for 11 first blockers: five bit counts, two bit reversals, two high-bit scans, and two low-bit
  scans.
- [x] (2026-08-30) Traced the canonical producer and exact width/signedness/result contracts for
  all 11 rows.
- [x] (2026-08-30) Added one typed semantic family through compiler classification, preflight, requirement
  collection, emission, fake provider, and real LLVM-provider lowering.
- [x] (2026-08-30) Proved LLVM 14 construction and libNVVM textual verification for every retained operation and
  width; reject malformed, vector, and unsupported-width descriptors deterministically.
- [x] (2026-08-30) Reran the 11-row family and full denominator at O0/O3, promoted every newly correct workload,
  update coverage/Pareto evidence and representative measurements, format, validate, self-review,
  and commit Slice 134.

## Surprises and Discoveries

- The exact inventory contains five `countbits` rows, including one newly exposed by scalar
  minimum/maximum. Its result is always scalar UInt32 even when the operand is 16 or 64 bits, so it
  is not the existing same-type integer-unary family.
- Signed `firstbithigh` is not merely a leading-zero count. The CUDA prelude contract complements a
  negative operand before scanning and returns UInt32 all-ones for zero (and therefore signed
  all-ones). This composite behavior must remain compiler-visible or be represented by an exact
  operation whose provider contract includes it.
- `firstbitlow` returns UInt32 all-ones for zero, while `reversebits` preserves the exact operand
  type. These distinct result contracts belong in one parameterized integer-bit family, not one
  falsely uniform unary rule.
- LLVM 7 already defines generic `ctpop`, `bitreverse`, `ctlz`, and `cttz`; its NVVM-specific older
  names are auto-upgraded to those intrinsics. LLVM 14 adds only optimization attributes and the
  scan declaration's `immarg` marker that the existing strict NVVM IR 2.0 serializer must remove.
- All 11 first blockers become differentially correct rather than exposing a later blocker. The
  full corpus gains exactly those 11 rows at each optimization level with zero old-correct
  regressions.

## Decision Log

- Decision: select the four integer bit operations ahead of isolated `abs` or one math spelling.
  Rationale: they form the largest coherent post-min/max first-blocker family (11 rows), share one
  integer-width representation boundary, and exercise reusable semantics used by real kernels.
  Date/author: 2026-08-30, Codex.
- Decision: recognize only the final one-block CUDA helper bodies and exact specialized signatures.
  Rationale: `StmtLoweringVisitor::visitIntrinsicAsmStmt` and CUDA target specialization produce
  these `IRGenericAsm` helpers. Their assembly plus concrete `IRFunc` signature is the canonical
  producer output at direct-NVVM preflight; fixture paths and source test names are not semantics.
  Date/author: 2026-08-30, Codex.
- Decision: keep the existing generic typed callback and extend only its operation vocabulary.
  Rationale: the callback already carries operation, result type, operand types, and values. The
  missing capability is the four concrete bit semantics, not a new provider interface shape.
  Date/author: 2026-08-30, Codex.
- Decision: advance the forward-only provider ABI to revision 26 with `COUNT_BITS`, `REVERSE_BITS`,
  `FIRST_BIT_HIGH`, and `FIRST_BIT_LOW` operation IDs.
  Rationale: revision 25 has no operation capable of expressing population count, bit reversal, or
  bit scans. Their result and operand contracts already fit the existing descriptor/query/emit
  callback, so a new callback would duplicate established generic machinery.
  Date/author: 2026-08-30, Codex.
- Decision: lower through LLVM generic integer intrinsics and keep signed-high-bit composition in
  typed LLVM construction.
  Rationale: LLVM 7 explicitly supports these intrinsic semantics. Signed `firstbithigh` requires
  complement-before-scan; scan-zero sentinels are formed in UInt32 so narrow operands still return
  exact all-ones. The serializer validates every declaration before removing LLVM-14-only
  attributes; this is the existing deterministic dialect boundary, not a fallback.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The slice is complete. One parameterized semantic family covers the four canonical CUDA helpers
over selected scalar 8/16/32/64-bit integers. Builder ABI revision 26 adds four operation IDs and no
callback. The compiler now uses one generic value-helper recognizer for these operations and the
Slice-133 minimum/maximum family. The isolated provider constructs LLVM `ctpop`, `bitreverse`,
`ctlz`, and `cttz`, including signed-high-bit and zero-sentinel composition.

All 11 measured first blockers become correct at O0 and O3 and receive direct regression lanes;
none merely advances to another blocker. The full 452-row census remains 430 MVP plus 22 extension
workloads. Native NVRTC O3 is correct for 449 with three infrastructure failures. Direct O0 is
correct for 237, mismatches seven, fails preflight for 203, and fails in the provider for five.
Direct O3 is correct for 233, mismatches 15, fails preflight for 203, and fails in the provider for
one. Exact set comparison reports zero old-correct regressions and the same 11 gains in each mode.

Among 427 MVP workloads with a healthy native reference, 236 compare correctly at O0, 231 at O3,
and 228 at both. The ordinary-intrinsic cluster falls from 58 to 47. The largest remaining MVP
clusters are ordinary/reconvergence `GenericAsm` families, remaining helper ABI contracts,
aggregate/pointer transport, and ordinary numeric/bit operations. The selected NVVM prefix passes
402/402 and all 22 promoted lanes pass.

The three representative gates remain correct. Direct O3 PTX assembles with CUDA 12.9 for SM70,
SM80, and SM90. Their median standalone direct O3 compile times are 265.0, 244.2, and 255.4 ms,
versus NVRTC's 381.2, 367.2, and 369.5 ms; direct O3 PTX sizes are 919, 793, and 1,404 bytes versus
8,889, 8,839, and 9,190 bytes. Census timings remain end-to-end compile/load/execute/compare, not
kernel-only runtime. CUDA 13 and physical SM70/80/90 runtime remain infrastructure gaps.

## Context and Current Pipeline

The CUDA prelude definitions in `source/slang/hlsl.meta.slang` select `$P_countbits($0)`,
`$P_reversebits($0)`, `$P_firstbithigh($0)`, or `$P_firstbitlow($0)`. Specialization leaves a linked
one-block `IRFunc` containing `IRGenericAsm`. `collectNVVMModuleRequirements` currently diagnoses
that canonical shape before provider mutation. Slice 133 established exact assembly/signature
diagnostics and a typed recognizer/emitter pattern for scalar minimum/maximum.

The shared semantic resolver in `source/compiler-core/slang-nvvm-semantic-catalog.h` is used by both
host and isolated provider. The provider constructs LLVM 14 IR, serializes compatible NVVM IR 2.0
text for CUDA's LLVM-7-era reader, links libdevice when explicitly required, and asks libNVVM to
verify and compile PTX. The implementation must use LLVM construction APIs; it may not manipulate
serialized IR text to synthesize operations.

## Scope and Non-Goals

In scope are the scalar integer overloads selected by the 11 measured rows, bounded to canonical
8/16/32/64-bit integer descriptors where the prelude contract is well-defined, exact UInt32 scan
and count results, and same-type reversal. Adjacent malformed arities, vectors, floating operands,
wrong results, and unsupported widths remain deterministic preflight/provider rejections.

Out of scope are vector helper admission, floating math, half conversion, wave/reconvergence work,
resource widening, unrelated helper ABI failures, provider packaging, and new CUDA-toolkit workers.
Those retain their measured Pareto positions unless a selected workload exposes a direct dependency.

## Architecture and Invariants

The canonical semantic descriptor is the sole contract between compiler and provider. A recognized
helper must have one block, no ordinary instructions besides its `IRGenericAsm`, one parameter, and
the exact assembly spelling. `reversebits` requires the result and operand to be the same selected
scalar integer type. Count and scan operations require a scalar UInt32 result and one selected
scalar integer operand. The signedness bit remains explicit because signed `firstbithigh` has a
different semantic transform.

Capability discovery and emission must call the same resolver. No descriptor may be emitted unless
the provider was queried for that exact descriptor. Any library or intrinsic declaration required
by provider lowering must be selected before module mutation. Unsupported canonical forms retain
E52017 with the exact assembly/signature diagnostic.

## Interfaces and Dependencies

If the four semantics require new operation IDs, update
`source/compiler-core/slang-nvvm-ir-builder-api.h` in one forward-only ABI revision and keep
`SlangNVVMBuilderValueOperationsAPI` unchanged. Update the shared catalog resolver, compiler
classifier/emitter, fake-provider observations, and real provider together. LLVM intrinsic or
ordinary instruction selection must be verified through the isolated LLVM 14 provider and CUDA
12.9 libNVVM; do not assume that an LLVM 14 intrinsic spelling is accepted by the LLVM 7 reader.

## Milestones

First, capture the 11 exact rows and trace the prelude/specialized-helper contracts. Second, add
focused resolver and fake-provider tests, then implement the narrow compiler recognizer. Third,
implement real-provider lowering and prove serialized LLVM/NVVM text plus PTX compilation across
the measured widths. Fourth, rerun all 11 rows at O0/O3 and promote only full differential
successes. Finally, run the complete census, representative gates, selected regression prefix,
formatter, diff audit, and commit plan plus implementation.

## Validation and Acceptance

Acceptance requires Release host and isolated-provider builds, focused fake- and real-provider
tests, all promoted direct indices, the selected NVVM prefix, the 11-row family at O0/O3, and the
full 452-row NVRTC/direct O0/direct O3 census. There must be zero regressions among Slice-133
old-correct rows. Record healthy native-reference O0/O3/both counts, post-slice root-cause clusters,
and representative compile-time/PTX/runtime metrics. Assemble representative direct O3 PTX for
SM70, SM80, and SM90. CUDA 13 and physical cross-architecture runtime remain explicit
infrastructure gaps if unavailable.

## Failure and Recovery

Builds and censuses write only under existing build directories and are safe to rerun. Generated
inventories, logs, PTX, cubins, and measurement samples stay under ignored `build/nvvm-census/`.
If a proposed LLVM intrinsic fails libNVVM verification, preserve the exact diagnostic, remove the
unsupported admission, and choose an equivalent principled typed lowering only if it preserves the
documented source semantics. Do not retain partial descriptors or hide verification failures.

## Self-Review

- The four new operation IDs survive because revision 25 cannot express the 11 measured population
  count, reversal, and scan helpers. Removing any corresponding classifier branch restores its
  fixture failures. No callback, feature flag, or signature-combination enum was added.
- `ValueOperationFamily::IntegerBit` survives as the single shared legality source. Reversal is
  same-type; count/scans return exact UInt32; operands are selected scalar integers. Vector,
  floating, wrong-result, and 24-bit negatives prove the boundary in both host and provider.
- `_resolveNVVMGenericAsmValueOperation` survives as a generalization of the Slice-133 recognizer.
  It accepts only one-block, asm-only CUDA prelude helpers and resolves their exact specialized
  signatures through the shared descriptor catalog. Fixture paths and source intrinsic names never
  participate.
- The provider's `IntegerBit` branch survives because generic LLVM construction is the responsible
  layer for four concrete semantic IDs. It preserves signed `firstbithigh`, all-ones zero sentinels,
  and narrow/wide UInt32 result conversion; the 11 differential workloads fail without it.
- The NVVM IR writer's integer-intrinsic validation survives as a widening of the existing strict
  `cttz.i32` dialect rule. LLVM 7 supports the semantic intrinsics; only verified LLVM-14-only
  attributes and scan `immarg` syntax are removed. Native and compatible serialization tests cover
  all four widths, while real libNVVM compiles and runs the promoted corpus rows.
- The ABI-version unit assertion now derives its expected text from
  `SLANG_NVVM_BUILDER_ABI_REVISION`; this removes stale duplicated ABI state found by the full
  prefix. No fixture check, syntax reconstruction, fallback, downstream malformed-IR patch,
  unqueried operation, or accidental vector admission remains.

## Artifacts and Hand-Off

Commit the completed plan, implementation, promoted fixtures, post-slice census TSV and Pareto JSON,
and a Slice-134 report. Keep generated mirrors and raw artifacts ignored. The report must distinguish
newly correct workloads from workloads that only advance to another blocker and must keep the fixed
coverage denominator visible.
