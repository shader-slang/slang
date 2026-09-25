# Admit exact narrow-integer masked min/max

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception.
Base `16207a0c3781d8e9158f205aee2e48e68cc11c3a`, branch `nvvm-backend`.
The worker returned checkout ownership; the parent independently accepted the slice.

## Purpose and Observable Result

Signed/unsigned 8/16-bit scalar/vector masked minimum/maximum reductions and inclusive/exclusive
prefixes execute through NVVM O0/O3. Exclusive empty prefixes return exact type extrema. Existing
matrix reduction leaves receive the same support without changing matrix capability rules.
Research 228 supplies independent prefix expectations; this slice establishes reduction semantics
before implementing the shared bounded integer MIN/MAX family.

## Progress

- [x] 2026-09-25: Read workflow, STATUS, 227/228 evidence, build skill and shared identity helper.
- [x] 2026-09-25: Plan narrow MIN/MAX family; parent agrees reductions require before proof.
- [x] 2026-09-25: Scalar/vector reduction proof: 1,344 source launches and 32 direct rejections.
- [x] 2026-09-25: Extend proof to existing matrix reductions and truncation: 1,568 source launches.
- [x] 2026-09-25: Identify aggregate leaf admission and provider-argument construction gates;
      update bounded implementation and require a full checkpoint.
- [x] 2026-09-25: Reverse the complete final emitter patch, rebuild original source, and prove the
      final formatted fixture passes NVRTC and rejects both direct modes; restore/rebuild patch.
- [x] 2026-09-25: Final smoke 4/4, fixture 3/3 and 53,088 exact research replay launches pass.
- [x] 2026-09-25: Admission boundaries 88/88, units 479 plus existing skip, toolkit 18 and contracts 6 pass.
- [x] 2026-09-25: Full frozen 1,356 cells preserve all 1,335 correct results, with only four
      diagnostic changes from int8_t to int64_t.
- [x] 2026-09-25: Discovery 315 cells preserves all old outcomes and adds three correct cells;
      material compile/assembly 6/6 passes. Full acceptance audit passes.
- [x] 2026-09-25: Complete exact inventories, all old outcome/input comparisons, failure histories,
      design, report, STATUS, result manifest and cumulative censuses for parent acceptance.

- [x] 2026-09-25: Parent independently verified the implementation, 192 unique evidence references,
      1,671 fresh outcomes, failure histories, all hashes and 53,088 semantic replay launches.

## Surprises and Discoveries

The identity helper serves reductions and prefixes. Simply broadening its width guard would also
admit arithmetic/bitwise operations. A prefix-mode gate would split an identical associative integer
algebra. Restrict extra widths to MIN/MAX after proving source reductions independently.

The identity-only candidate exposes `_getNVVMHomogeneousWaveAggregateLeafType` restricting leaves
to 32-bit/FP64 values. Make it classify numeric structure and let its two consumers enforce operation
support. Preserve the old aggregate shuffle width policy explicitly in the shuffle resolver.

The next candidate reaches constant creation and fails E52018. Identity materialization interpreted
all bits through int32_t. Provider `getIntegerConstant` requires a signed value fitting its destination
width, so UInt8 bits 255 and Int8 minimum bits 128 must become -1 and -128. Fix this emitter-side
argument producer with width-aware signed interpretation; the provider correctly rejects bad values.

Matrix reductions already permit CUDA and share aggregate leaves, while matrix prefixes retain
an independent capability restriction. The final fixture and reduction prototype cover matrix2x2.
The initial fixture's generic `int(T)` was ill-typed; compare results to representable `T(expected)`
while the mathematical oracle uses wide integer arithmetic. Retain its error log separately.

## Decision Log

2026-09-25, worker/parent: include the coherent narrow MIN/MAX reduction/prefix family. Integer
min/max selects representable operands; promotion and tree/scan ordering cannot change extrema.
Keep narrow arithmetic/bitwise, 64-bit integer and FP16 support unchanged.

2026-09-25, worker/parent: make aggregate classification structural rather than adding an
operation-dependent flag or duplicate MIN/MAX width list. Move unchanged shuffle policy to its
consumer. This shared classifier change triggers a full checkpoint, superseding the initially
planned frozen selection208 (107 identities/321 cells). Execute all 1,671 cells freshly and compare
full225 with accepted227 overlaid. Reset cadence from one to zero only after exact preservation.

2026-09-25, worker: format with the installed toolchain from the sourced setup environment. Revert
eight unrelated historical design-document formatting hunks. Final fixture whitespace changes
receive another complete-patch before drill. Record final compiler/library hashes during replay.

## Outcomes and Retrospective

Parent acceptance passed. Full checkpoint: 1,671 fresh cells, 1,620 correct and 51 known failures;
all 1,617 previous correct cells are preserved, and one new fixture adds three. Exactly four old
prefix diagnostics advance from int8_t to int64_t exclusive min/max; no other old outcome field
changes. Preserve all 51 first-known failures and six resolved histories. All 552 prior runtime
input hashes remain unchanged. The audit verifies 28 source and 12 artifact hashes, with 553 current
input hashes. Full 229 resets cadence to zero.

All required gates pass. Independent replay checks 53,088 launches and 46,663,680 output words
exactly, including inactive sentinels, with unchanged input buffers. Stop at the int64_t GenericAsm
boundary without investigating the next feature. No provider/ABI/system change or push. The parent owns the authorized local commit.

## Context and Current Pipeline

For `int8_t value=-128`, `WaveMultiPrefixExclusiveMin(value,uint4(3,0,0,0))` returns 127 at lane 0.
`WaveMultiMin` returns the least mask-selected integer at every selected caller. `hlsl.meta.slang`
produces canonical scalar or Multiple GenericAsm. Scalar resolution, or aggregate resolution and
its homogeneous leaf classifier, reaches `_initializeNVVMMaskedWaveScalarOperation` and the shared
identity helper. `_emitNVVMMaskedWaveScalarValue` creates the provider identity and scans selected
values. The checked AST/IR and identity bits are canonical; only admission and the emitter's
provider argument need repair. No semantic value is reconstructed as syntax.

## Scope and Non-Goals

8/16-bit signed/unsigned MIN/MAX prefixes and reductions, with existing scalar/vector and matrix
reduction leaves. No arithmetic/bitwise, 64-bit integer, FP16, matrix capability, provider ABI,
ordinary FP64 vector shuffle, quad reconvergence, resource context or application shader changes.
No worker commit, push, driver change or reboot.

## Architecture and Invariants

The identity recipe owns masked scalar admission. Integer extrema use uint64_t shifts at admitted
widths 8, 16 and 32. Convert identity bits to a signed destination-width provider argument by
subtracting `2^width` when the sign bit is set; all intermediates fit int64_t. Preserve typed lane
transport and semantic comparison signedness. Aggregate classification owns only structure;
masked recipes and shuffle resolution own support policy. No new helper or fallback is introduced.

## Interfaces and Dependencies

Native Ubuntu 24.04, L4 SM89/target80, CUDA 12.9.2, LLVM 14, ABI 36. Source
`build/nvvm-loop/slice-203-env.sh` and use matching RelWithDebInfo tools. Build with
`cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
`CMAKE_BUILD_PARALLEL_LEVEL=1`; sequential GPU suites, at most four CPU workers, two unit servers.
Shell escalation is required because sandbox initialization fails RTM_NEWADDR. No new dependencies.

## Milestones

1. Establish unchanged source/GPU before proof for the registered fixture and independent reductions.
2. Admit the bounded algebra, repair aggregate admission and constant argument construction, and
   retain intermediate diagnostics that demonstrate why each change is necessary.
3. Reverse the entire final patch and prove the unchanged final fixture on rebuilt original source;
   restore/rebuild and run the complete acceptance script.
4. Preserve exact outcome/input identities and all first-known failure histories; finish the
   completed plan, five-part report, design, STATUS, result manifest and cumulative census records.

## Validation and Acceptance

Run `build/nvvm-loop/slice-229-after/run-gates.sh`. Require smoke 4, focused 3, units 479 plus one
existing Windows-only skip, toolkit 18, discovery contracts 6 and material compile/assembly 6.
Replay unchanged research228 sources/inputs/expectations: 47,712 primary plus 672 wide-input launches
in all three modes, preserving every accepted 32-bit control. Replay the final reduction prototype
in all three modes: 4,704 launches. Check 80 excluded operation/width signatures and eight excluded
narrow aggregate shuffles before and after. Research launches are separate from registered cells.

Full frozen selection is immutable `census.slice-195.tsv`: 452 identities/1,356 cells. All 104 old
discovery sources plus one fixture give 105 identities/315 cells. Require exact requested inventories,
no duplicate/missing rows, and comparison of classification, return code, execution counts,
diagnostic and canonical shape against accepted225+227. Preserve 1,617 previous correct cells,
552 old runtime input hashes, 51 open first-known failure records and six resolved histories.
Resolve failures only with actual correct execution. One fixture adds three registered cells.

The shared classifier triggers a full checkpoint; there are no inherited runtime cells in acceptance.
A successful checkpoint becomes full229 and resets implementation cadence to zero. Material probes
remain compile/assembly checks; runtime/performance claims require application bindings, textures,
LUTs, inputs and an expected-output contract.

## Failure and Recovery

Stop GPU dispatch on device loss. Keep original and intermediate evidence in distinct files.
If final fixture semantics or bytes change, repeat its original-emitter proof. Reverse only the
recorded production patch, then restore/rebuild it. Never lower an oracle or reset an old baseline.
Stop at the next independent blocker rather than extending this family further.

## Artifacts and Hand-Off

Raw evidence lives under ignored `build/nvvm-loop/slice-229-before` and `slice-229-after`.
Retain final source, binary and input hashes, exact per-cell outcomes and diagnostic histories in
`runtime-validation.slice-229.json` and cumulative census TSVs. Commit-ready records include the
completed plan/report, design, STATUS and discovery addition. Return a compact parent handoff with
exact deltas, inherited evidence, limitations and checkout ownership explicitly returned.
