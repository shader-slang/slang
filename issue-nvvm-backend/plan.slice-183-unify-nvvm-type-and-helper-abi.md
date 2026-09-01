# Cache one direct-NVVM type-role classification

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, `NVVMTypeLoweringContext` classifies each canonical linked-IR type once into one
cached `NVVMTypeInfo`. That record owns the complete role matrix for entry results/parameters,
helper results/parameters/values, ordinary values, ordinary storage, parameter-group storage, and
structured-buffer storage. Provider type construction consumes the record instead of independently
rebuilding more than fifty overlapping predicates and one separate legality expression.

This bounded slice does not rewrite helper ABIs. Its aggregate prototype gate determines whether a
uniform memory ABI is a sound next step; it must not replace established first-class/resource
representations merely to reduce emitter code.

## Progress

- [x] (2026-09-01) Inventoried the role predicates and representation shims in `lowerType`.
- [x] (2026-09-01) Evaluated the aggregate-as-memory prototype against established canonical roles.
- [x] (2026-09-01) Added one cached type-information record and moved role legality into it.
- [x] (2026-09-01) Replayed both corpora and documented the unchanged ABI and coverage.

## Surprises and Discoveries

The old `lowerType` entry rebuilt the entire predicate lattice before it checked the requested role,
even when a previous recursive query had already classified the same type. The provider-handle
caches could not safely answer legality because the same canonical type may be accepted for one
role and rejected for another.

A uniform aggregate-memory ABI is not currently principled. Copyable structs and arrays are
first-class helper values; resource-containing structs carry opaque handles; parameter-group and
structured-buffer storage have distinct CUDA layouts; LLVM 14's typed pointers also require the
exact pointee representation. Converting all of these to memory would erase real role distinctions
and create extra caller/callee transport without unlocking a failing canonical shape.

## Decision Log

- Decision: cache classification independently from provider handles.
  Rationale: role legality must be checked before a handle cache can be consulted, while one
  canonical type may have multiple physical provider representations.
  Date/author: 2026-09-01, Codex.
- Decision: make `NVVMTypeInfo::supports` the sole role-admission matrix used by type lowering.
  Rationale: classification fields and role policy should change together when a producer adds a
  canonical type shape.
  Date/author: 2026-09-01, Codex.
- Decision: reject the universal aggregate-memory prototype and preserve role-specific forms.
  Rationale: current resource, layout, pointer, and first-class value contracts prove there is no
  single physical aggregate form; no corpus failure demonstrates that such a rewrite is needed.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Implementation and broad replay are complete. The classification record contains the resolved
pointer pointees, aggregate/resource forms, exact descriptor structs, scalar widths, and role
booleans that were formerly local one-off queries. Recursive type construction still selects the
established role-specific provider representation; this slice changes ownership and reuse, not ABI.

Frozen corpus v1 remains 418/418/418 O0/O3/both over 427 healthy references, with no old-correct
loss. Discovery remains 72/72/72 over 72 healthy references. The selected prefix passes 437/437,
the permanent NVVM category passes 92/92, and provider ABI revision 34 is unchanged.

## Context and Current Pipeline

`source/slang/slang-emit-nvvm-type-lowering.cpp` has mature canonical classifiers for numeric,
copyable, helper, aggregate-storage, parameter-group, resource, and pointer shapes. Before this
slice, `NVVMTypeLoweringContext::lowerType` invoked all of them into local variables, assembled a
large `isLegal` expression, and only then consulted provider-handle caches. Recursive lowering
repeated that work.

## Scope and Non-Goals

In scope are one type-information record, a per-context cache, one explicit `supports` matrix,
switching `lowerType` to that record, and aggregate-policy evidence. Out of scope are changing any
helper/launch/storage ABI, widening type support, deleting canonical classifier APIs still used by
preflight and operation validation, LLVM opaque pointers, provider ABI changes, and fixture-specific
admission.

## Architecture and Invariants

- A canonical type is classified once per module emission context.
- Role legality is checked before provider-handle lookup.
- `NVVMTypeInfo::supports` is the one role-admission matrix used by `lowerType`.
- The record preserves distinct value, helper, storage, parameter-group, resource, and pointer data.
- Provider handle caches remain role-specific where physical representations differ.
- Unsupported types retain the existing role-specific diagnostics.

## Milestones

1. Inventory every query at the start of `lowerType` and the consumers of each resolved value.
2. Evaluate a uniform aggregate-memory representation against copyable, resource, parameter-group,
   structured-buffer, local-pointer, and helper-boundary contracts.
3. Add `NVVMTypeInfo`, cache it by canonical type, and move the complete role matrix to `supports`.
4. Make `lowerType` consume the record and remove its duplicate classifier/legality construction.
5. Build, replay both corpora, update durable docs/report, and commit with subject `slice 183`.

## Validation and Acceptance

Run outside the sandbox:

```powershell
cmake.exe --build build --config Release --target slang-unit-test slangc slang-test
$env:SLANG_NVVM_BUILDER_PATH = 'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Release/bin/slang-test.exe -category nvvm
python.exe issue-nvvm-backend/run-compute-census.py --output build/nvvm-census/slice183 --workload-ids-from issue-nvvm-backend/census.slice-182.tsv --jobs 8 --modes nvvm-o0 nvvm-o3
python.exe issue-nvvm-backend/run-compute-discovery.py --frozen-v1 issue-nvvm-backend/census.slice-146.tsv --output build/nvvm-discovery/slice183 --jobs 8 --modes nvvm-o0 nvvm-o3
```

Replay frozen v1 and discovery separately. Acceptance requires unchanged denominators, no
old-correct regression, no provider ABI change, identical deterministic unsupported-role
diagnostics, no new type special case, a clean build without new warnings, and `git diff --check`.

## Failure and Recovery

If caching changes answers because a classifier depends on mutable IR state, remove that field from
the cache and fix the canonicalization boundary; do not add cache invalidation guesses. If a role
cannot be expressed by the common record, improve the record rather than restoring a second
legality expression.

## Artifacts and Hand-Off

Commit the completed plan, implementation, both corpus artifacts, durable design/capability updates,
and five-part report. Hand Slice 184 a single cached type-role classification boundary and the
evidence that emission planning must preserve role-specific physical representations.
