# Distinguish launch parameters from entry-point block parameters

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM classifies an `IRParam` by its parent block before choosing the
physical CUDA launch ABI. Only parameters owned by an entry function's first block use pointer-backed
LLVM `byval` aggregate access. Parameters of merge, loop, and switch blocks remain ordinary SSA phi
values, so aggregate field extraction uses LLVM `extractvalue`.

The observable target is the three frozen-v1 workloads currently clustered as
`provider-aggregate-field-pointer`: `generic-interface-dynamic-param`,
`generic-interface-multi-conform`, and `generic-interface-nested`. The slice fixes this shared
role-classification invariant, records later blockers exactly, and does not widen unrelated
aggregate address, helper ABI, or resource shapes.

## Progress

- [x] (2026-08-31) Reconciled the Slice 152 frozen/discovery Pareto and selected the three-row
  provider aggregate-field-pointer cluster.
- [x] (2026-08-31) Dumped final linked IR for `generic-interface-dynamic-param` and traced the
  failed provider operation to a merge-block `%floatOp : Tuple` parameter.
- [x] (2026-08-31) Proved that the emitter's `as<IRParam>` test conflates entry-block launch
  parameters with non-entry-block phi parameters.
- [x] (2026-08-31) Restricted pointer-backed entry-aggregate extraction to first-block function
  parameters and added six direct O0/O3 regression lanes.
- [x] (2026-08-31) Ran all three motivating workloads through native NVRTC and direct NVVM O0/O3;
  all three are differentially correct with no later blocker.
- [x] (2026-08-31) Built, passed the 427/427 selected unit prefix, ran both complete corpora and the
  twelve-gate SM70/80/90 measurements, and updated separate artifacts and durable documentation.
- [x] (2026-08-31) Completed the input-shape/self-review audit, exact corpus-identity and JSON
  integrity checks, and diff checks; Slice 153 is ready to commit.

## Surprises and Discoveries

- Final linked IR has already removed the source entry parameters from these compute kernels. The
  offending value is instead `%floatOp : Tuple`, a parameter of the branch merge block that joins
  `FloatDoubler` and `FloatNegator` values.
- Slang uses `IRParam` for both first-block function parameters and later-block phi values. Other
  compiler passes distinguish them with `param->getParent() == func->getFirstBlock()`.
- `NVVMTypeLoweringContext` correctly represents a scalar aggregate launch parameter as an LLVM
  generic pointer with `byval` attributes. The provider's `emitStructFieldPointer` correctly rejects
  the offending tuple because its lowered value is an LLVM struct, not a pointer.
- `extras/formatting.sh --check-only --modified` cannot run in the available Windows bash
  environment because `gersemi`, `clang-format`, `prettier`, and `shfmt` are absent from its path.
  The focused C++ change was manually style-reviewed and `git diff --check` passes.

## Decision Log

- Decision: classify physical launch parameters by first-block ownership, not merely `IRParam` op.
  Rationale: block parentage is the canonical IR role distinction and is already used elsewhere in
  Slang. The merge tuple is valid first-class SSA and should use the existing aggregate extraction
  operation.
  Date/author: 2026-08-31, Codex.
- Decision: leave the provider ABI at revision 30.
  Rationale: the existing pointer GEP and aggregate extraction operations already express both
  correct representations. The compiler selected the wrong one.
  Date/author: 2026-08-31, Codex.
- Decision: bound the slice to field extraction from entry-function block parameters.
  Rationale: discovery's struct-field-address and sequential-pointer rows involve distinct pointer
  producers and must not be admitted through this role fix without their own exact-shape proof.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. O0/O3/both
correctness reaches 380/384/380, up three in every numerator with zero old-correct regression. The
three provider aggregate-field-pointer rows all become correct, eliminating that cluster.

Discovery remains exactly 82 workloads and 72 healthy native references at 54/54/54, with zero
old-correct regression. Its remaining aggregate pointer failures use different canonical producers,
confirming that this slice did not admit adjacent shapes through the entry-parameter correction.

The selected unit prefix passes 427/427 and all six promoted lanes pass. All twelve measurement
gates assemble through CUDA 12.9 at direct O3 for SM70, SM80, and SM90. Provider ABI revision 30 is
unchanged. The result reinforces that typed provider rejection is useful evidence: the provider
correctly exposed a compiler-side role mismatch rather than needing a wider operation.

## Context and Current Pipeline

Consider this source shape:

```slang
IProcessor<float> op;
if (condition)
    op = FloatDoubler();
else
    op = FloatNegator();
output[0] = op.process(5.0);
```

Existential lowering creates a canonical tagged `%Tuple` and sends each branch value to a merge
block. The merge block owns `%floatOp : Tuple` as an `IRParam`; `get_field(%floatOp, %value0_)`
extracts its runtime type tag. Register allocation maps block parameters to LLVM phi values.

In `source/slang/slang-emit-nvvm.cpp`, `kIROp_FieldExtract` currently chooses the entry ABI path when
the current function is the entry point and the base is any `IRParam`. That condition ignores the
base parameter's parent block. It calls `emitStructFieldPointer` on the already-lowered first-class
tuple, and the typed LLVM provider returns `SLANG_E_INVALID_ARG` because a struct is not a pointer.

The canonical construction is valid: the phi parameter is the ordinary SSA representation of the
branch merge. The consumer must preserve that role. An actual aggregate launch parameter is owned
by the entry function's first block, is lowered by `NVVMTypeUse::EntryPointParameter` to a pointer,
and receives `byval` attributes. Its fields still require pointer GEP plus invariant load.

## Scope and Non-Goals

In scope are exact first-block ownership classification, the three motivating frozen workloads,
focused regression directives after differential correctness, precise cascade recording, complete
separate-corpus measurement, and durable design/ledger updates.

Out of scope are arbitrary `IRFieldAddress` widening, sequential aggregate pointers, global
parameters, entry-point signature legalization, helper aggregate ABI, resource layout support,
provider callbacks or ABI revision, fixture-name checks, syntax reconstruction, compatibility
fallbacks, frozen-corpus identity changes, and corpus v2.

## Architecture and Invariants

- A function parameter is an `IRParam` owned by the function's first block.
- A non-first-block `IRParam` is an SSA block parameter and lowers to a phi value.
- Only first-block scalar aggregate parameters of the entry function are pointer-backed CUDA
  `byval` parameters.
- Field extraction from a first-class aggregate uses `emitAggregateElementExtract`; field extraction
  from a physical `byval` aggregate parameter uses `emitStructFieldPointer` followed by load.
- The provider validates typed LLVM roles and must continue rejecting struct GEP on a non-pointer.
- Frozen corpus v1 and discovery keep their existing separate identities and denominators.

## Interfaces and Dependencies

The implementation should remain in `source/slang/slang-emit-nvvm.cpp` and reuse the existing IR
parent relationship, type lowering, aggregate extraction, and pointer GEP builder operations. No
public API, provider callback, provider ABI revision, or LLVM dependency change is expected.

## Milestones

1. Amend the `kIROp_FieldExtract` classification so the `byval` path requires the base `IRParam` to
   belong to `function->getFirstBlock()`. Explain the launch-parameter/phi distinction with the
   motivating existential branch example.
2. Build and run the three frozen workloads through real-provider O0/O3 differential execution.
   Promote only stable correct lanes and capture any next first blocker by exact shape and producer.
3. Run the selected direct-NVVM unit prefix and complete frozen-v1/discovery corpora. Verify exact
   corpus identity, separate metrics, and zero old-correct regression.
4. Update Slice 153 TSV/JSON snapshots, report, measurement manifest where useful, design document,
   capability ledger, and this plan; complete the self-review and commit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools as required by
`AGENTS.md`. Acceptance requires:

- the final linked-IR probe still contains the valid merge-block tuple parameter;
- the emitter sends its `get_field` through aggregate extraction, while established first-block
  aggregate launch tests still exercise pointer-backed `byval` field loading;
- every newly promoted workload is correct against native NVRTC at direct O0 and O3;
- frozen v1 remains exactly 452 workloads/427 healthy references and discovery remains exactly 82
  workloads/72 healthy references, with separate O0/O3/both, classification, Pareto, and regression
  evidence;
- the selected direct-NVVM prefix passes with zero old-correct regression;
- representative O3 PTX still assembles for SM70, SM80, and SM90 where the harness permits;
- provider ABI revision remains 30; and
- Python/artifact checks and `git diff --check` pass, with no staged content from
  `external/slang-binaries/`.

## Failure and Recovery

If a motivating workload reaches a later failure, record that cascade and leave it for Pareto
selection. If a real first-block aggregate launch parameter stops working, compare its parent and
lowered LLVM pointer type with the existing byval unit coverage; do not weaken provider validation.
Generated dumps and corpus outputs under `build/` are reproducible and remain untracked.

## Artifacts and Hand-Off

Retain the final-IR probe under `build/` and commit the completed plan because the user explicitly
requires plans and implementation together for this experiment. Commit promoted regression lanes,
Slice 153 frozen/discovery snapshots, the five-part report, and durable design/ledger changes. The
report must show why both shapes use `IRParam`, why parent block is the canonical discriminator, and
which tests prove this emitter layer owns the choice.
