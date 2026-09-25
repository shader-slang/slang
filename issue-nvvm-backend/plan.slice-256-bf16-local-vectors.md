# Admit local BF16 vector storage and mutable helper references

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans/reports in
local commits. Raw evidence stays under ignored build/. User authorization is the active loop.

## Purpose and Observable Result

A noinline `replace(inout vector<BFloat16, 3> x, vector<BFloat16, 3> y)` must preserve every raw BF16
lane through local storage at NVRTC O3 and NVVM O0/O3. Cover widths 2/3/4 and out parameters too.

## Progress

- [x] 2026-09-25: Read workflow/status and accepted 255; clean base9da02cb46b6cc900b5586bb240654307da73470f.
- [x] Declare bounded implementation and full checkpoint before edits.
- [x] Freeze432-word fixture/oracle and capture133 source/12 artifact/565 input identities. NVRTC passes;
      both NVVM modes reject BorrowInOutParam<vector<BFloat16, 2>> before implementation.
- [x] Implement explicit local Storage and helper-pointee roles with symmetric array/vector conversions.
- [x] Add three-width physical storage unit and exported-reference negative; optimized build passes.
      Runtime 4/focused 26 and 9 exhaustive GPU controls pass; independent audit checks 37749024 words.
      All 6 research 255 aggregate-pointer diagnostics remain exact.
- [x] Full frozen 1356/discovery 351/material 6 checkpoint passes exact preservation. Every 1704 old
      outcome and 39 unresolved/18 resolved histories survives;3 additions give1707total/1668correct.
      Units 1049 pass/13 skip and semantics 1052 pass/77 skip preserve all old IDs; toolkit 18/contracts 6 pass.
- [x] Both role-cache visitation orders pass 9 source GPU controls each, 18 total/75498048 words.
      All input/expected bytes are identical between orders. PTX helper order proves the intended path.
- [x] Complete exact preservation, self-review, report/design/STATUS and local acceptance audits.
      Include completed artifacts in the local slice commit; no push.

## Surprises and Discoveries

Fresh delegation remains unavailable after 253's agent-thread limit. Use WORKFLOW's local fallback,
one writer with separate oracle/mechanical audits; do not claim fresh independent-agent review.

The first positive fake-builder unit used a runtime BF16 bitcast. The legacy fake records semantic
BF16 operation results but does not classify them as physical integer lanes, so vector construction
failed in the fake. The test now uses a canonical BF16 constant to isolate the physical memory/type
contract; unchanged real-provider runtime fixtures retain dynamic bit transport and exhaustive
encodings. No production change was made for this fake limitation; the failed log is retained.

PTX confirms actual mutable-reference loads/stores. O3 can scalarize readLocal parameters but
retains caller loads; BF4 O3 raises local frame alignment to 8 without changing component access or
the 2-byte storage contract. Frame extent/alignment is not aggregate ABI evidence.

## Decision Log

- 2026-09-25, parent: Select the explicit next action in accepted 255. Physical layouts were qualified
  there; external helper ABI differs from CUDA and remains excluded. Material runtime contracts are
  unavailable. Rolling 252/254/256 retains one material compile-time slice.
- Keep recursive helper/copyable/aggregate predicates unchanged. Admit canonical BF vectors only in
  explicit local storage and existing one-operand Generic mutable pointer roles.

## Outcomes and Retrospective

The unchanged432-word fixture passes all 3 modes;18 exhaustive GPU controls preserve 75498048 words
across both cache visitation orders. Bare local BF2/BF3/BF4 vectors and internal mutable references
are admitted with exact bit transport. Recursive aggregates/resources/device pointers remain closed;
the 6 original Ptr<H> rejections remain exact. Provider ABI41 is unchanged.

Full 256 preserves 1704 old outcomes and adds 3 correct:1707 total/1668 correct/39 unresolved, retaining18
resolved histories. Units 1049 pass/13 skip and semantics 1052 pass/77 skip retain every previous ID.
Runtime 4/focused 26/toolkit 18/contracts 6/material 6 pass. Full 256/targeted233/cadence0; rolling 252
material compile-time, 254 CUDA layout correctness, 256 local BF16 vector capability.

Reassess material compile-time work next; runtime bindings/textures/LUT/input/output contracts are
still unavailable. The next bounded action is fresh current-compiler material profiling, before
selecting another feature slice. Physical record qualification remains inherited255, not admitted.
Separate oracle/mechanical audits are recorded without claiming fresh independent-agent review.

## Context and Current Pipeline

The source producer emits canonical Vec(BFloat16Type,N), local PtrType with one operand and
BorrowInOutParamType/OutParamType. Existing asNVVMSupportedLocalHelperValuePointerType rejects the
pointee. NVVMTypeInfo admits BF vectors by value only. Local allocation and helper pointers must
select physical Storage, while load/store bridge to register values. Width2 storage is <2xi16>
(size/alignment 4); widths 3/4 are component arrays [Nxi16] (size6/8,alignment 2), as qualified255.
Register/by-value representation remains <Nxi16>. These are valid distinct roles of one canonical
IR type; do not reconstruct semantic types or add equivalence relations.

## Scope and Non-Goals

Bare local vectors, internal mutable helper references and whole-value loads/stores. Preserve
existing scalar BF16 and all ordinary paths. No recursive aggregate/array, global device pointer,
resource, parameter-group, shared, FP8, matrix or external CUDA helper support. Any newly reachable
lane addressing must either be tested with correct storage semantics or remain rejected.

## Architecture and Invariants

Use existing role maps so lowering order cannot alias storage arrays with register vectors.
Use a single explicit storage predicate/alignment contract where needed. Only exact admitted local
pointers qualify loads/stores. BF3/4 memory conversion extracts/constructs every lane without numeric
conversion, preserving signed zeros, subnormals, infinities and NaN payloads. BF2 needs no conversion.
Existing negative inout BF2 unit case must migrate to meaningful positive coverage.

## Interfaces and Dependencies

Expected source changes: slang-emit-nvvm-type-lowering.{h,cpp}, slang-emit-nvvm.cpp; unit-test-nvvm-
emitter.cpp and focused CUDA fixture. Existing provider operations suffice; ABI41 stays unchanged.
Native Ubuntu/L4SM89/driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14,targetSM80. Use local
slang-build skill and source build/nvvm-loop/slice-203-env.sh. At most4 CPU workers, sequential GPU.
Baseline compiler a595092cb50be989df9015d38852946afd599def83b5485ba5188b2a5e4e3f7a, provider
5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab, 132sources/12artifacts/564inputs.

## Milestones

1. Before roots slice-256-before; after roots slice-256-after. Freeze same source/input/oracle before
   production edits; NVRTC correct and NVVM diagnostic required. Record canonical producer/consumer.
2. Apply role/pointer/cache and memory conversion changes. Migrate old inout negative, add exact
   physical representation/cache-order tests and neighboring excluded role tests.
3. Format explicit paths with extras/formatting.sh, build preset releaseWithDebugInfo parallel4
   targets slangc slang-test render-test test-server. Run small runtime gate before GPU suites.
4. Focused all 3mode output, full 16-bit encoding local roundtrip replay, full acceptance below.
5. Review every helper/branch with input-shape audit, freeze identities/closed raw index; commit.

## Validation and Acceptance

Full checkpoint required for type-role/cache/memory lowering. Reuse exact254 before identities;
new fixture before is fresh. Add one discovery source116->117; frozen 452 unchanged. Preserve all 1704
old outcomes and 39 unresolved/18 resolved histories, report3 additions separately. Full units baseline
1048pass/13skip, semantics 1052 pass/77 skip. Runtime 4, focused new fixture/neighbor BF/FP8/helper units,
toolkit 18, discovery contracts 6, frozen 1356 (explicit census.slice-195.tsv), discovery 351, material 6.
Commands follow accepted 254 run-small-gates.sh and run-full-gates.sh with 256 output roots.
Compare exact IDs, diagnostics, execution counts and shape, not exit status. Full outputs must match
independent bit expectations; no numeric tolerance or weakened oracle. Research255 full physical
record controls remain inherited, not newly admitted source support. Reassess all material cells
without runtime/performance claims. Capture source/artifact/input hashes at each gate and final.

## Failure and Recovery

Retain failed attempts and exact logs. New regression blocks acceptance: repair within scope or
revert implementation, preserving fixture and findings. Stop at independent next unsupported shape
with minimal handoff. Do not weaken expectations, remove workload IDs, or change system/driver.

## Artifacts and Hand-Off

Commit completed plan, five-part report.slice-256-bf16-local-vectors.md, compact runtime-validation,
census/discovery results, design contract and STATUS. Raw logs/PTX/cubins/output/snapshots remain
build/nvvm-loop/slice-256-{before,after}. No push. Acceptance must be explicit before continuing.
