# Establish a plan-producing direct-NVVM preflight

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, direct-NVVM preflight produces a compiler-owned `NVVMEmissionPlan` that owns the
reachable function order, physical function names, and the resolved descriptors for every ordinary
value operation emitted through the generic provider callback. Operand validation and emission
consume those records instead of invoking the semantic resolver again. All capability checks still
finish before provider module creation.

This is the first bounded vertical migration to a plan-producing boundary, not a claim that every
resource, atomic, pointer, aggregate, or compound recipe has moved. Those families remain explicit
follow-up inventory and may need typed plan variants rather than one universal record.

## Progress

- [x] (2026-09-01) Inventoried the function-closure/name and ordinary-value decisions repeated by
  preflight, operand validation, and emission.
- [x] (2026-09-01) Added an owned emission plan beside deduplicated provider requirements.
- [x] (2026-09-01) Migrated reachable function order/names and direct value-operation descriptors.
- [x] (2026-09-01) Replayed both corpora, documented the remaining typed-plan variants, and
  confirmed unchanged provider ABI and coverage.

## Surprises and Discoveries

`NVVMOperationRequirements` already formed most of a plan for capability discovery, but its value
operations were deduplicated by typed overload. Emission needs one source-keyed record per IR
instruction. Keeping both views is intentional: capability checks should query each overload once,
while emission must map each canonical producer to its previously resolved descriptor.

Compound recipes cannot use the same record without losing their ordered intermediate flow.
Resource and atomic operations similarly carry typed shapes and memory roles beyond a value
descriptor. They should become typed plan variants as their resolver/emitter pairs are migrated.

## Decision Log

- Decision: retain preflight and make it plan-producing.
  Rationale: deterministic unsupported-shape diagnostics and capability checks before provider
  mutation are valuable; repeated interpretation is the problem.
  Date/author: 2026-09-01, Codex.
- Decision: keep deduplicated capability requirements and source-keyed emission records.
  Rationale: they answer different questions and sharing descriptor ownership avoids pointer
  lifetime hazards in `SlangNVVMValueOperationDesc`.
  Date/author: 2026-09-01, Codex.
- Decision: migrate the common direct-value vertical slice before heterogeneous recipes.
  Rationale: it proves the boundary across ordinary arithmetic, comparisons, conversions, selects,
  fixed wave values, and non-pointer bit casts without inventing a universal operation payload.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Implementation and broad replay are complete. The emitter no longer recollects the function
closure, regenerates physical names, or calls `_resolveNVVMValueOperation`. The second preflight
operand pass proves a planned record exists instead of resolving the same instruction again.

Frozen corpus v1 remains 418/418/418 O0/O3/both over 427 healthy references, with no old-correct
loss. Discovery remains 72/72/72 over 72 healthy references. The selected prefix passes 437/437,
the permanent NVVM category passes 92/92, and provider ABI revision 34 is unchanged.

## Context and Current Pipeline

Before this slice, `validateNVVMSupportedIR` collected reachable functions and names, and
`emitNVVMIRFromLinkedIR` repeated both walks. `_validateNVVMFunction` resolved common arithmetic,
comparison, conversion, select, wave, and bit-cast descriptors while collecting capabilities; its
second operand-validity pass resolved them again; emission resolved them a third time.

## Scope and Non-Goals

In scope are stable function order/names, source-keyed ordinary value-operation records, owned
descriptor storage, removal of their repeated resolver calls, and provider-before-mutation
invariants. Out of scope are new shader semantics, removing preflight, a general target escape
record, moving every heterogeneous recipe in one patch, provider ABI changes, and changing corpus
denominators.

## Architecture and Invariants

- NVVM legalization produces the only accepted IR representation.
- Preflight owns canonical shape/type/ABI decisions and deterministic diagnostics.
- Deduplicated capability requirements and source-keyed emission records own their descriptor data.
- Every planned ordinary value instruction is resolved exactly once.
- Function order and physical names are selected exactly once.
- Capability checks complete before module creation; emission does not reinterpret planned IR.

## Milestones

1. Inventory repeated function and direct-value decisions.
2. Add `NVVMEmissionPlan` with owned function/name/value-operation records.
3. Populate records during the first preflight classification pass.
4. Make operand validation and emission consume the records; delete repeated resolver/walk calls.
5. Build, replay both corpora, document remaining typed plan variants, and commit as `slice 184`.

## Validation and Acceptance

Run outside the sandbox:

```powershell
cmake.exe --build build --config Release --target slang-unit-test slangc slang-test
$env:SLANG_NVVM_BUILDER_PATH = 'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Release/bin/slang-test.exe -category nvvm
python.exe issue-nvvm-backend/run-compute-census.py --output build/nvvm-census/slice184 --workload-ids-from issue-nvvm-backend/census.slice-183.tsv --jobs 8 --modes nvvm-o0 nvvm-o3
python.exe issue-nvvm-backend/run-compute-discovery.py --frozen-v1 issue-nvvm-backend/census.slice-146.tsv --output build/nvvm-discovery/slice184 --jobs 8 --modes nvvm-o0 nvvm-o3
```

Acceptance requires unchanged frozen/discovery denominators, no old-correct regression, 437/437
selected-prefix and 92/92 permanent-category results, no `_resolveNVVMValueOperation` call in
emission, no repeated closure/name walk, provider ABI revision 34 unchanged, and `git diff --check`.

## Failure and Recovery

If a descriptor references temporary operand storage, copy it into the plan record; never retain a
resolver stack pointer. If an instruction does not fit the ordinary value record, leave its existing
typed path intact and inventory the exact additional data its future variant needs.

## Artifacts and Hand-Off

Commit the completed plan, implementation, both corpus artifacts, durable docs/capability updates,
and five-part report. The next cleanup slice should rank remaining duplicated families by decision
count and corpus importance, then add typed plan variants in reusable groups.
