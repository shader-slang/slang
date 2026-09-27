# Evaluate bounded corrections for nested-array copies

This ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. The development loop
continues; skip Slack, no push/system changes. Root owns scope/acceptance/docs. Reused author280 owns
ignored prototypes; separate reused reviewer pch275 audits read-only. No production changes or builds.

## Purpose and Observable Result

Identify which small emitted-code changes preserve canonical nested-array layout without generating
one IR operation per array element. Research280 committed85c08e2d0 proves preexisting NVRTC O3/NVVM O3
padding loss. A correction candidate must first pass independent output and snapshot/alias checks;
passing prototypes do not themselves justify a new production lowering path.

## Progress

- [x] 2026-09-27: Read current STATUS/WORKFLOW;280 committed; accepted279 remains installed unchanged.
- [x] 2026-09-27: Author/reviewer identify existing pointer-copy loops and aggregate SSA spill constraint.
- [x] 2026-09-27: All37runtime/8source/2config/576input/22pin identities match accepted279.
- [x] 2026-09-27: Freeze11 standalone sources/17cells (11smallGPU,6largecompile-only),
      independent full-buffer oracle and exact runner hashes; pre-execution independent review passes.
- [x] 2026-09-27: First17cells complete: both optimized baselines wrong9234; NVVM O0 correct.
      All three small loop cells and both NVVM optnone-helper cells pass. All three large loop cells
      compile with compact output. Six memcpy cells fail declarations before execution and are retained.
- [x] 2026-09-27: All six memcpy-v2 cells succeed: three smallGPU buffers correct, three large
      compile-only cells complete. Removed LLVM7-incompatible immarg and redundant NVRTC builtin
      declaration only; original six declaration failures retained.
- [x] 2026-09-27: Pointer LLVM counts102/114 constant; optimized PTX grows one instruction.
      Local storage grows36/108→786432/2359296bytes; no largeGPU claim.
- [x] 2026-09-27: Independent reused reviewer verifies103 references,23attempts/17effective,
      repair deltas, complete buffers, growth and caller/helper traces. Root strict evidence generation
      passes. Complete report/record/navigation ready for formatting and local research commit.

## Context and Current Pipeline

For Cell={uint16,Child{uint16,uint32}}, canonical scalar offsets are0/4/8 and stride12. Both optimized
backends turn ordinary copies into adjacent16-bit stores0/2. O0 succeeds. Existing
`lowerCopyLogicalWithDestImpl` (`slang-ir-lower-copy-logical.cpp`) uses canonical field pointers and
loops for arrays>16; it requires source/destination addresses. Provider `_emitStore` receives an SSA
aggregate, which may represent an earlier memory snapshot or a phi. A pointer-copy loop cannot recover
that snapshot by rereading current source memory. `_emitSequentialElementExtract` selects across all
constant array indices, so a dynamic SSA extraction loop is not a bounded IR-size solution.

## Scope and Non-Goals

At most three families, with ordinary-copy failing controls:

1. Pointer memcpy with distinct source/snapshot/destination storage, NVRTC and NVVM.
2. Runtime-count canonical leaf copy loop, separate snapshot then commit, both backends. Inhibit loop
   unrolling where supported; inspect actual code rather than assuming the annotation works.
3. NVVM-only typed SSA copy helper carrying noinline/optnone, while the caller remains optimized.
   This isolates the store optimizer; it is not blanket compilation atO0 or an accepted production fix.

Use3 elements for GPU experiments and65536 for bounded compile/IR-size probes of pointer candidates.
Freeze full integer oracle for source/old/destination plus self-alias cases before execution. No broad
substandard-array support, arbitrary overlapping ranges, GPU performance claim or production ABI change.
Volatile aggregate stores are deferred; they change memory-access semantics and are unnecessary here.

## Architecture and Invariants

Keep canonical record/array types and layout. Never read undefined padding as a semantic oracle. memcpy
requires disjoint source/destination; self-assignment must be handled without violating that contract.
A distinct snapshot must precede destination mutation. Preserve root alignment and derive child
alignment from actual offsets/stride; do not assert nominal ABI alignment on underaligned storage.
A provider-level general correction must handle arbitrary SSA producers, not only an immediate load.
The prototypes may expose placement constraints rather than supply a complete solution.

## Interfaces and Dependencies

Reuse279 direct libNVVM/CUDA replay harness, adding a narrowly scoped NVRTC replay where needed.
No compiler rebuild. CUDA12.9.2/NVRTC12.9.86, targetSM80/L4SM89, LLVM14, ABI42; accepted279 compiler
9e013b2c/version295-g0043e8d17 and provider a861b242. Raw root `build/nvvm-array-store-prototype281`.
Official NVVM12.9 specification lists noinline/optnone, volatile and memcpy support:
https://docs.nvidia.com/cuda/archive/12.9.1/nvvm-ir-spec/index.html . Actual installed-tool behavior must
still be tested. Its legacy dialect uses LLVM7 semantics; LLVM14 is the provider construction library.

## Milestones

1. Clone280 identity verifier under new raw root and verify37runtime/8source/2config/576inputs/22pins.
   Author freezes baseline, candidate sources/options, expected output and selection before GPU.
2. Run one serialized owned process group per attempt, max1800seconds, max4CPU workers. Capture compile
   return codes/logs, PTX and actual complete buffers. NVVM O0/O3; NVRTC O3 only, explicitly separate.
3. Confirm each control actually fails where expected and each remedy reaches the intended copy shape.
   Check baseline versus remedies with independently varied values and snapshot/self-alias obligations.
   Compile large pointer-array cases with strict timeout; record IR/source growth separately from PTX.
4. Audit how each successful candidate could fit existing machinery without recreating bad aggregate
   stores or violating snapshot semantics. Reject unsupported/circular remedies explicitly.
5. Verify installed identities unchanged; write compact five-part report, structured evidence and plan
   outcome, navigation updates, formatting and local commit. Continue the authorized loop.

## Validation and Acceptance

Exact inventory and oracle must be frozen before execution. All failures and unsupported API attempts
remain distinct; a successful later experiment never erases them. Evidence must distinguish direct
vendor prototypes from Slang language support, bounded IR-size observations from performance, and
pointer-copy experiments from a general SSA-store solution. Independent reviewer audits source/input
shape, snapshot/alias contracts, outputs and provenance. No production changes means inherited full279/
targeted233/cadence0 and1740cell outcomes remain unchanged, with280 open defects retained separately.

## Failure and Recovery

No accepted binaries are overwritten. Preserve all failed attempts and partial artifacts. Bound huge
compile probes; timeout is a failed qualification, not permission to retry with hidden limits. A
known-correct regression or identity drift blocks later feature work until resolved. If no candidate
meets semantic/size constraints, record the negative result and select the next bounded investigation.

## Decision Log

- 2026-09-27, root: Prioritize the new correctness defect. Test existing pointer-copy mechanisms and an
  isolated optimizer boundary; do not select array unrolling, arbitrary volatile changes or a new type
  representation merely because one regression would pass.

## Surprises and Discoveries

A dynamic extract from an SSA array uses O(N) constant extracts/selects in the current provider.
Pointer-loop machinery alone does not solve snapshot materialization; a whole-value spill can recreate
exactly the failing store. These constraints must shape any later production design.

## Outcomes and Retrospective

Initial loop and isolated-helper prototypes pass their bounded small GPU contracts. The pointer-loop
large compile boundary remains compact. Memcpy-v2 correction succeeds; independent evidence review passes. No production
remedy selected. These results do not solve arbitrary SSA snapshot materialization through pointer
loops, nor establish large helper behavior or CUDA production placement.

## Artifacts and Hand-Off

Ignored raw root above; durable report.slice-281-array-copy-prototypes.md,
research-evidence.slice-281.json and this completed plan. Resume with author scope/frozen inventory and
owned gate records; never rerun a completed cell merely because chat context is missing.
