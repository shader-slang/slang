# Implement scalar BF16 storage and Float32 conversions

This ExecPlan follows `.agent/PLANS.md`. Maintainer exception: completed NVVM plans/reports are
committed with each slice. The worker does not commit; the parent owns acceptance and local commit.

## Purpose and Observable Result

Canonical Slang BFloat16 scalar values will cross helpers, mutable local storage, branches and
16-bit bitcasts, and convert exactly to/from Float32 on SM80. A registered readable runtime fixture
and unchanged research238 inputs prove the result at public NVRTC O3 and direct NVVM O0/O3.

## Progress

- [x] 2026-09-25: Read workflow, STATUS, full237 and accepted research238, build skill/environment.
- [x] 2026-09-25: Write bounded plan before production edits.
- [x] 2026-09-25: Freeze final fixture/projection; NVRTC pass and direct reject before proofs retained.
- [x] 2026-09-25: Implement ABI38 semantic kind, bounded scalar roles and provider conversion.
- [x] 2026-09-25: Format explicit paths, restore unrelated hunks and build final matching tools.
- [x] 2026-09-25: All final gates pass; full1686/1643correct, exact old-cell preservation.
- [x] 2026-09-25: Complete exact audit, design/report, compact evidence/censuses and STATUS for parent review.

- [x] 2026-09-25: Parent independently verifies all outcomes, complete failure histories, identities,
  1,073 evidence references and all5,855,205 replay words; accepts full checkpoint239.

## Surprises and Discoveries

Research238 proves native libNVVM bfloat rejection and physical i16 transport. Existing FLOATING_POINT
width16 means IEEE Half. Existing public helper/copyable predicates recurse into aggregates and
resources, so scalar BF16 admission must remain a role-specific boundary rather than widening them.

## Decision Log

2026-09-25: Distinct BF16 semantic kind, physical i16, explicit scalar Float32 pair only. Narrow via
cvt.rn.bf16.f32; widen by zero-extension and upper-word bitcast. No generic FP32 intermediate for
integer conversion: 16842753 is a measured counterexample. Keep canonical producer IR unchanged.
Material runtime remains blocked by absent binding/texture/LUT/input/oracle; reassess six support cells.

## Outcomes and Retrospective

Implementation, validation and independent parent acceptance completed 2026-09-25.
Full checkpoint239 is accepted and resets implementation cadence to zero. Final1686 fresh cells /1643correct preserve all1640
old correct cells,43 unresolved histories and14 resolved histories. Only two frozen BF16 diagnostic
boundaries move to vector<BFloat16,4>; no old failure resolves. Final smoke4/fixture3/units482+skip/
toolkit18/contracts6/material6 pass. Five exhaustive replay launches and5 assemblies pass with
5,855,205 returned words checked. Every gate matches37 source/12 artifact/558 input hashes;
all557 prior inputs remain exact. Next bounded work is separately qualified BF16 vector transport/
conversion or exact integer construction; source-ordered dot remains another required boundary.
No worker commit/push/system change or GPU loss. Incomplete attempt2 is retained, never accepted.

## Context and Current Pipeline

`BFloat16(asfloat(bits))` becomes canonical FloatCast, and bit_cast<uint16_t> becomes BitCast.
Helper signature preflight currently rejects BFloat16. `_getNVVMSemanticType` and shared catalog
will preserve a distinct format; `NVVMTypeLoweringContext` will cache physical i16 for selected scalar
roles. Provider semantic validation selects the exact conversion independently of physical width.
The frozen vector/dot source remains a separate unsupported boundary and is not the success proof.

## Scope and Non-Goals

Scalar values/constants/bit transport, helper arguments/results, mutable local storage, branches and
Float32 conversion only. No frontend, library, runner changes; no integers/half/double conversion,
arithmetic, vectors/dot, aggregates or resource BF16 support. No push, worker commit or system changes.

## Architecture and Invariants

BFloat16Type remains canonical. IEEE classifier and numeric arithmetic families exclude BF16.
Scalar helper/value/storage roles admit BF16 explicitly without recursive aggregate admission.
Existing storage/cache mechanics carry i16. Constants preserve the checked IR literal via core
FloatToBFloat16; runtime NaN narrowing checks classification and widening preserves upper-word bits.
Provider ABI revision negotiates the added semantic kind. Descriptor validation rejects malformed
width/lanes and unsupported numeric conversions before emission.

## Interfaces and Dependencies

Base d7732c6ba2e2978811b628ab5740bdd25e9a273d, nvvm-backend. Native Ubuntu24 L4SM89 target80,
CUDA12.9.2/NVRTC12.9.86 LLVM14 driver580.126.09. Start ABI37 compiler a89e9b370b03e62a62fe5f6becaab312399d5cec60bac6d75a53ff649a75c19f,
provider dafc5a557ce6f83d358c89956910af9761e352bb2f70f5efc6d5e7bc7f8a89ea.
Use inspected slice-203-env.sh, CMAKE_BUILD_PARALLEL_LEVEL=1 and build preset releaseWithDebugInfo,
--parallel 4 targets slangc slang-test render-test test-server. Units two servers, sequential GPU.

## Milestones

1. Final fixture and scalar-only projection before-proof. Preserve research columns7/8 inactive.
2. API/catalog/type-role/provider implementation with meaningful negative real-provider coverage.
3. Final format/build followed by all gates with per-gate source/artifact/input hashes.
4. Exact five-field comparison and histories; parent-ready artifacts and review.

## Validation and Acceptance

Use workflow's 30-minute-bounded native commands, full frozen explicit --workload-ids-from
issue-nvvm-backend/census.slice-195.tsv:452 identities/1356 cells. Discovery109 old/327 cells plus
fixture3. Preserve all1640 old correct cells, 43 unresolved histories and14 resolved histories;
compare classification, return_code, complete execution_counts, diagnostic, canonical_shape.
Only narrowly explained BF16 next-boundary diagnostics may change. No omissions/duplicates/baseline
reset. Replay all73190 research inputs columns4/5/6 in public3 modes and accepted rawi16 O0/O3.
Full checkpoint mandatory for shared type/provider contract. All557 old input hashes immutable.

## Failure and Recovery

Keep unique before/after raw roots and never overwrite research236/238 or accepted237 evidence.
Investigate ordinary failures; stop on GPU loss, unresolvable regression or consequential semantics
choice. Sandbox bwrap fails; routine commands use escalation. Do not edit executing scripts.

## Artifacts and Hand-Off

Raw build/nvvm-loop/slice-239-before and slice-239-after. Completed plan/report, runtime-validation
JSON, two censuses, design note and STATUS; independent audit/index includes final hashes and all
proof artifacts. Parent owns final acceptance/local commit and any release decision.

## Completed Validation Milestones

2026-09-25: Initial ABI build302 and final formatted build263 succeeded. Revised fixture before-proof
used the preserved accepted compiler; hashes and mixed-provider limitation are recorded in
final-fixture-identity.json. Unrelated historical formatting was restored exactly.

Initial smoke4/fixture3/replay5 passed. The first unit matrix had two frontend-ambiguous probes,
which were removed from the backend matrix while retaining provider rejection coverage. The resource
expectation was corrected to its actual struct-field-address boundary. The unit-only9-step rebuild
succeeded, and attempt1 logs remain retained.

Self-review found that the GPU fixture exercises BF16 phi transport, not explicit Select. Parent
confirmed generic core.meta.slang::select(bool,T,T) is a canonical valid scalar producer. The named
nvvmIRBuilderBFloat16Contract now tests a dynamic i1 condition and two i16 values, exact query,
emission/serialization and wrong-Half/arity rejection; it fails if catalog BF16 selection is removed.
No production/header changed. Incomplete frozen attempt2 was safely stopped and retained, then the
unit-only9-step rebuild and every final gate completed on one matching identity set.

Final exact results: smoke4, fixture3, units482 plus one existing Windows-only skip, toolkit18,
contracts6 and material compile/assembly6 pass. Frozen1356/1343correct and discovery330/300correct
preserve all1683 old cells except the two justified BF16 diagnostic/canonical-shape advances.
All1640 old correct outcomes,43 unresolved first-known/reproduction records and14 resolved histories
remain; the new fixture adds3correct cells. Final1686/1643correct, no omissions/duplicates or reset.

Final replay checks1,097,850 active words and4,757,355 untouched words across5 launches and5 SM80
assemblies. The separate checker also validates the before NVRTC projection. Parent's independent
oracle agrees on all5,855,205 final words. The complete audit verifies693 compact references,
117 immutable research238 artifacts and241 immutable research236 artifacts. Source37/artifact12/
input558 identities match every gate; all557 old inputs remain exact. Final artifacts and hashes
are recorded in runtime-validation.slice-239.json. Worker releases checkout to parent acceptance.
