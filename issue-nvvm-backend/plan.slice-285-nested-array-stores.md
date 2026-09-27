# Preserve nested record padding inside array stores

This ExecPlan follows `.agent/PLANS.md` and the committed NVVM plan exception. The authorized loop
is active; skip Slack, no push/system changes. Root owns scope, build/gates, acceptance and commits;
a bounded worker owns prototypes and, only after promotion, provider/tests. A separate reviewer audits.

## Purpose and Observable Result

Correct the reproduced NVVM O3 padding loss for arrays of nested records without unrolling arrays,
changing physical layouts or adding helper call arguments. A whole copy of `Cell[3]`, where Cell has
`uint16 first; Child child` and Child has `uint16 first; uint last`, must preserve fields at offsets
0/4/8. Independent integer oracles must also observe wrapped/multidimensional copies and saved values.

## Progress

- [x] 2026-09-27: Read WORKFLOW/STATUS. Research284 committed8fbf0f84e; original and annotation
      candidate both exceed the large O3 gate, while O0 compiles. This is not a new size restriction.
- [x] 2026-09-27: Fresh author nested_array285 freezes two annotation-only cells; independent reviewer/root approve exact single-store delta and inherited282 oracle.
- [x] 2026-09-27: Promotion O0/O3 both execute with full correct buffer `[0,123,0,456]`; no retries.
- [x] 2026-09-27: Accepted279 identities exact; 100 file/link recovery snapshot and configurations verified.
- [x] 2026-09-27: Final formatted fixtures run in before-v2: O0 all three correct; NVRTC all wrong37; NVVM O3 root wrong37 and wrapped/multidimensional wrong5. Original nine syntax failures retained.
- [x] 2026-09-27: Omission controls reproduce bad1/4; three O3 LLVM traces show valid canonical whole stores. Raw harness classifications preserved beside full-output runtime-mismatch interpretation.
- [x] 2026-09-27: Root and independent reviewer approve promotion and before traces; sole author authorized minimal canonical-type predicate and terminal-store alignment correction.
- [x] 2026-09-27: Candidate1 builds successfully; exact source patch12f503e9, compiler62469125, providerABI42 af1661de, version301-g8fbf0f84e. Recovery snapshot verified.
- [x] 2026-09-27: Focused6units/24nativeGPU pass, flat6 pass; new three fixtures correct at both NVVM modes, NVRTC3wrong37 retained. Actual LLVM changes only2/1/1 alignment annotations; independent review passes.
- [x] 2026-09-27: Full1740 outcomes/576inputs and all material artifacts exact279. Units1097/13 and semantics1170/78 preserve all prior IDs. Runtime4/toolkit18/contracts pass. Independent final review accepted with128 verified references and no findings.
- [x] 2026-09-27: Complete compact five-part report, exact accepted-full ledger/contract/navigation; formatter, final byte audit and local commit close this slice.

## Surprises and Discoveries

Before any fixture execution, independent review found that the destination still held the expected
snapshot before the second out-copy. Initialize it with complementary values before both copies so
omitted or partial stores fail deterministically. Retain provisional sources/freeze as unexecuted;
freeze two root-only omission controls at NVVM O0, expecting masks1/4, to verify these checks.
Before execution, review also corrected the new output parser to recognize the actual `uint32_t`
header (alongside `uint`). Runner hashes are refrozen; no shader/compiler attempt is lost.
The first nine actual-adapter cells fail parsing after formatting joins adjacent struct declarations
without explicit semicolons (`} struct Cell`). Native test cases ran; no GPU execution occurred.
Retain all nine outcomes in `before/`, archive source/runner freezes, add struct terminators and run
new frozen bytes in `before-v2/`. Controls and IR captures were not started on the invalid fixtures.
Existing280 NVRTC padding defect remains independent and open. The large O3 timeout remains
an explicitly inherited limitation; matching bounded failures do not prove identical causes.

## Decision Log

- 2026-09-27, root: Prefer the demonstrated truthful alignment annotation over whole-array expansion,
  pointer snapshot reconstruction or typed helpers. The latter fail arbitrary SSA or large-size gates.
  Retain accepted279 direct-struct splitting. Promote only after independent SSA/boundary evidence.

## Outcomes and Retrospective

Candidate1 is accepted under build/RelWithDebInfo: final identity equals the built/tested candidate,
and the full279 comparison preserves every main/native obligation. Six new NVVM directives pass;
NVRTC focused failures remain. All material artifacts are unchanged. Accepted279 recovery remains
verified at build/nvvm-nested-array-stores285/accepted279-layout. Last full285/targeted233/cadence0.
Next returns to material generic-overload inference under a fresh bounded plan; the loop stays active.

## Context and Current Pipeline

Slang canonical values reach NVVM `_emitStore`; provider `_emitStorePreservingNestedStructLayout`
splits direct struct fields using LLVM canonical types and DataLayout, but leaves arrays opaque.
Direct libNVVM283 counterfactuals show annotation4→1 alone corrects small whole-array stores on actual
aligned4 roots. No malformed producer exists. Whole SSA values can come from construction, phi or an
earlier load; rereading the source pointer can lose the snapshot. The proposed annotation preserves
that canonical SSA value and all call signatures. Optimization may still infer stronger alignment,
so direct-vendor and actual-adapter output tests are required.

## Scope and Non-Goals

Provider store translation only; no ABI change, type admission change, Slang lowering rewrite, new
representation, volatile store, copy helper, optimization-level override or arbitrary size cutoff.
At a terminal whole store, reduce alignment only when traversal of canonical aggregate element/member
types finds an immediate struct-in-struct boundary hidden below an array. Ignore pointer pointees.
Flat arrays/flat-record arrays/scalars/vectors remain unchanged; immediate struct splitting stays.
Classification depends on type structure, not array count, and does not iterate array elements.
Do not claim FP8 record-array admission, all address-space/packed forms or large GPU feasibility.
NVRTC remains a differential control; its known wrong output cannot be counted as correct.

## Architecture and Invariants

Reuse canonical LLVM element types/fields and existing helpers. Preserve allocas, loads, scalar stores,
SSA values and signatures. Alignment1 is a truthful weaker guarantee even for aligned4 storage; never
strengthen the incoming guarantee. One store remains per opaque array regardless of element count.
Audit every new helper/special case against AGENTS, name its failing test and responsible layer.

## Interfaces and Dependencies

Likely production file `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`, provider unit file
`tools/slang-unit-test/unit-test-nvvm-builder.cpp`, focused integer fixtures in `tests/cuda/`.
Raw root `build/nvvm-nested-array-stores285`. Accepted279 compiler9e013b2c/providerABI42 a861b242,
source0043e8d17 +patch62ae6473; 37 runtime artifacts/8 source/2 config/576 inputs/22 pins.
Native Linux, CUDA12.9.2/NVRTC12.9.86, SM80/L4SM89. Already-read slang-build skill governs native
RelWithDebInfo build. Max4workers,2unitservers; serialize build/GPU/suites/benchmarks, gate<=1800s.

## Milestones and Promotion Gate

1. Verify identities and freeze exactly two direct-vendor promotion cells: take282 `ordinary-3.ll`,
   change only the aligned whole Payload store annotation4→1, retain the existing unaligned store,
   actual aligned roots, noinline and absence of optnone. Run O0/O3 with independent full output
   `[0,123,0,456]`, including constructed values, earlier source mutation, both phi choices and offset1
   canaries. Preserve inherited original O3 wrong18, O0pass. Each cell120s, no retry. A failure closes
   this method without retaining implementation; do not expand the experiment to rescue it.
2. Freeze at most three actual Slang focused sources: strengthen280 root-array checker with an
   independent fresh-source out copy; add guarded wrapped-array and multidimensional cases. Include
   whole assignment, noinline calls/returns/phi, changed-source saved values and independent field
   expectations. Exercise all65536 low16-bit patterns with distinct per-index values/32-bit sentinels.
   Freeze oracle/dimensions/commands before execution. Run all three modes on accepted279 once; retain
   correct/failing/preflight outcomes and inspect emitted canonical shapes. Run two derived root-only
   NVVM O0 controls omitting the first/second out-copy, expecting full buffers `[1,123,0,456]` and
   `[4,123,0,456]`; record them as deliberate runtime mismatches with reproduced control oracles. Direct caller checks must
   not reuse corrupted destination as expected source. Do not infer pass from an exit code.
3. Root and reviewer accept promotion/before traces. Save/verify complete accepted279 installed
   layout with symlinks/modes/configuration/pins under this root before build. Author becomes sole
   provider/test writer. Introduce minimal documented canonical-type predicate, using existing helper
   if available; apply only at terminal store. Unit tests require bounded single-store nested arrays
   at counts3/65536, multidimensional/wrapped types, align1/4/8 guarantees and unchanged flat/scalar/
   vector controls. Existing invalid-operation and nested-struct unit identities stay intact.
4. Format changed source and build native presets: configure default clearing cached version fields,
   preserve existing options/core module policy; build releaseWithDebugInfo slangc/slang-test/
   slang-unit-test/slang-numerics-modules --parallel4. Record actual source patch and loaded artifacts.
   Freeze final fixtures before final validation; preserve every failed attempt/candidate.
5. Focused GPU at NVRTC O3/NVVM O0/O3: NVVM must correct targeted failures and preserve all previously
   correct outputs. NVRTC known mismatches stay explicit. Provider shape/invalid-operation units,
   existing279 integer/substandard fixtures and flat-array neighbors must pass. Revert evidence from
   inherited original annotation counterfactuals and accepted279 before cells proves necessity.
6. Provider changes require full checkpoint against `runtime-validation.slice-279.json` using
   RESULTS commands: runtime4/frozen1356/discovery384/material6, all units/semantic suites with2servers,
   toolkit18 and census/discovery/complex/results contracts. Preserve exact1740 main cells/576inputs,
   37unresolved/20histories and every old native identity; record new tests separately. No retry or
   failed/review-required outcome becomes acceptance. Inspect all changed material PTX/cubin/resources
   with independent review; no material GPU runtime contract exists.
7. Independent input-shape/helper/source/material review, root final identity and exact result audit;
   complete report/ledger/plan, relevant contract and navigation, formatter/diffcheck/local commit.

## Validation and Acceptance

One disposition per requested mode: classification, compiler/process return, executed/passed/ignored,
diagnostic, canonical shape and complete independent output. Maintain known NVRTC failures outside
main corpus, with truthful test directives/support limitations; do not introduce permanently failing
unconditional native tests. Keep main manifests unchanged. Test directives may select NVVM regressions
while focused differential evidence retains failing NVRTC control. New fixtures are additions, not
retroactively part of inherited corpus. Full acceptance resets cadence only after exact reviewed
preservation. Operational times and PTX counts are not speedup measurements.

## Failure and Recovery

Promotion failure ends the candidate before production. Unexpected valid shape or independent blocker
gets a separately bounded slice. If implementation regresses accepted correctness, fix within scope
or restore verified accepted279 artifacts/source and record the blocker; no subsequent feature work
on a regressed baseline. Preserve failures, known large-size timeout and evidence directories; no
silent retries or broader type admission. User stop instructions take precedence.

## Artifacts and Hand-Off

Raw root owns frozen inputs, before/after, loaded identities, recovery, logs and artifacts. Durable
report.slice-285-nested-array-stores.md plus one structured validation record, this completed plan and
necessary contract/navigation updates form the accepted slice. The final ledger generator verifies
source patch12f503e9, all current runtime/configuration/pin bytes, native maps and independent review.
After local commit, inspect the current WORKFLOW/STATUS and start a bounded material inference plan.
