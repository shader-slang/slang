# Generalize canonical helper value transport

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, one recursive compiler-owned copyable-value contract governs direct-NVVM helper
parameters, helper results, local storage, calls, and aggregate transport. Canonical scalar,
selected vector, fixed-array, and struct compositions that the existing generic builder can
represent cross direct helper boundaries without per-fixture admission rules. The Slice 131 helper
ABI cluster is rerun as a denominator: every newly unlocked workload is promoted into durable
direct O0/O3 coverage, and remaining failures move to their next canonical root-cause cluster.

The provider remains LLVM 14 builder ABI revision 24. No callback is added unless an admitted
canonical value operation proves impossible to express using its existing generic type, pointer,
aggregate, load/store, function, call, and return operations.

## Progress

- [x] (2026-08-30) Committed the measurement-only Slice 131 census as `fed2883b9` and selected
  helper ABI types, aggregate/pointer/layout transport, and ordinary intrinsic semantics as the
  three leading reusable clusters.
- [x] (2026-08-30) Confirmed the first concrete inconsistency: final linked IR for
  `hlsl-intrinsic/scalar-double.slang` contains `sincos(Double, OutParam<Double>,
  OutParam<Double>)`. Scalar `Double` is a supported value and the builder can lower it, but the
  local numeric-pointer classifier excludes `Double`, so preflight rejects the helper parameter.
- [x] (2026-08-30) Inventoried the first rejected canonical parameter/result type for all 51 MVP helper-ABI
  workloads and separate finite copyable values/local borrows from device/shared pointers,
  existential/dynamic-dispatch artifacts, and explicitly deferred scalar families.
- [x] (2026-08-30) Replaced overlapping scalar/struct/array helper checks with one recursive, cycle-safe
  copyable-value classification and exact local-pointer relation, then use that contract through
  preflight, type lowering, reachability/layout checks, emission, and fake-builder coverage.
- [x] (2026-08-30) Reran the 51-workload helper cluster at O0/O3, inspected every transition and new first shape,
  promote every newly unlocked fixture, and update the broad census numerators/Pareto ledger.
- [x] (2026-08-30) Formatted, built Release host/provider outside the sandbox, ran focused positive and negative
  tests plus all representative gates and the selected NVVM regression, ran `git diff --check`,
  completed the input-shape self-review, and prepared Slice 132 for commit.

## Surprises and Discoveries

- The first helper-parameter failure is not a new ABI family. `Double` values already cross helper
  results and ordinary parameters, and builder ABI 24 already has generic 64-bit floating types,
  local storage, pointers, loads, stores, and calls. Only the older memory-oriented
  `isNVVMSupportedNumericValueType` classifier excludes scalar `Double`.
- The 51-workload cluster is heterogeneous. A signature-level census must distinguish values that
  are already valid LLVM/NVVM transport from signatures whose semantic family remains outside the
  MVP; admitting the latter merely to move the diagnostic downstream would be a regression.
- Internal helper/local aggregate values do not cross a CUDA external-storage ABI boundary. Requiring
  their LLVM size/alignment to match CUDA layout regressed three already supported workloads and
  rejected otherwise self-consistent helper transport. CUDA layout checks remain on resources,
  parameter groups, and other external storage.
- A derived `Ptr<T>` type does not establish local mutable ownership. Treating typed field pointers
  as local roots regressed eight immutable matrix/resource workloads. Keeping plain local/output/
  mutable-borrow pointers separate from derived pointers, then validating the canonical producer
  chain, restored all eight without a compatibility path.
- The full denominator found eleven neighboring aggregate/pointer workloads in addition to the ten
  original helper-cluster rows. All 21 existing workloads plus the new fixture are correct at O0
  and O3, with no old-correct regression.

## Decision Log

- Decision: bound Slice 132 to canonical helper value and local-pointer transport.
  Rationale: this is one representation invariant shared by the 51 helper-ABI failures and several
  neighboring aggregate failures. Device/global pointer semantics, existential reconstruction,
  BFloat16, and the operation bodies reached after a signature is admitted remain separate root
  causes unless the existing selected representation already owns them.
  Date/author: 2026-08-30, Codex.
- Decision: generalize compiler classification before considering provider ABI 25.
  Rationale: builder ABI 24 already expresses arbitrary integer/floating/vector/array/struct types,
  generic pointers, typed helper signatures, aggregate construction/extraction, loads/stores, and
  calls. A provider callback would duplicate type policy and move a compiler-owned legality
  decision across the isolation boundary.
  Date/author: 2026-08-30, Codex.
- Decision: use final post-specialization linked IR as the signature source of truth.
  Rationale: helpers seen by `_validateNVVMHelperTarget` are concrete `IRFunc` definitions after
  specialization and linking. Reconstructing source syntax or recognizing fixture names would
  ignore the exact representation the emitter must transport.
  Date/author: 2026-08-30, Codex.
- Decision: represent local pointer ownership and derived pointer type relations separately.
  Rationale: canonical `FieldAddress`/`GetElementPtr` results may have the same final `Ptr<T>` type
  as a local variable while retaining immutable resource or parameter-group provenance. The type
  relation is sufficient for an exact call argument, but only the producer chain can authorize a
  mutable load/store/address consumer.
  Date/author: 2026-08-30, Codex.
- Decision: do not impose CUDA external-storage layout on internal helper/local values.
  Rationale: both sides of an internal call use the same provider type, while external resource
  and launch ABI boundaries still require CUDA agreement. Applying the external check internally
  duplicated an ABI that no consumer observes.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The full census contains 452 workloads: 449 native-correct plus three infrastructure rows; direct
O0 is 218 correct, seven runtime mismatches, 222 preflight failures, and five provider failures;
direct O3 is 214 correct, 15 mismatches, 222 preflight failures, and one provider failure. Against
427 healthy MVP references, 217 compare correctly at O0, 212 at O3, and 209 at both.

Of the original 51 MVP helper rows, ten become correct, 13 reach their next exact root cause, and 28
retain a helper type-contract failure. Eleven neighboring aggregate/pointer rows also become
correct. All 21 existing newly correct workloads are promoted beside their native CUDA lanes, and
the focused recursive Double-array/struct/output fixture passes native plus direct O0/O3. No
previously correct row regresses.

Builder ABI revision 24 is unchanged. The selected NVVM prefix passes 400/400; Release host and
provider builds pass; the three representative workloads remain correct in the census and assemble
for direct O3 SM70/SM80/SM90 with CUDA 12.9. The leading remaining MVP clusters are ordinary
intrinsic `GenericAsm` (66), wave/reconvergence `GenericAsm` (31), helper type contracts (28),
aggregate/pointer/layout transport (23), and ordinary numeric/bit operations (16). Slice 133 should
use the typed ordinary-`GenericAsm` inventory to choose the largest reusable semantic family.

## Context and Current Pipeline

Consider this representative source pattern from the ordinary double-intrinsic corpus:

```slang
double s;
double c;
sincos(value, s, c);
```

After specialization and linking, the CUDA path contains a direct helper with the canonical
signature `Func<Void, Double, OutParam<Double>, OutParam<Double>>`; the caller has local
`Ptr<Double>` variables. `StmtLoweringVisitor` and the ordinary parameter-lowering path produce
the parameters, local variables, loads, stores, and call. `_collectNVVMFunctions` reaches that
exact helper. `_validateNVVMHelperTarget` asks `_isSupportedNVVMHelperParameterType` about each
canonical parameter. `asNVVMSupportedLocalNumericPointerType` currently rejects `Double` because
`isNVVMSupportedNumericValueType` admits Half/Float but not Double, even though
`isNVVMSupportedValueType`, `getNVVMNumericValueAlignment`, `NVVMTypeLoweringContext::lowerType`,
and builder ABI 24 already admit scalar Double.

When a signature passes, `_validateNVVMFunction` validates local storage, loads/stores, exact call
argument relations, and return values. `_collectNVVMRequirements` then records reachable struct
types and layout obligations. `NVVMTypeLoweringContext` lowers the same source type by its role,
and `_emitNVVMModule` creates the typed helper, parameters, local storage, call, and return through
the generic builder. The slice must make these stages share one classification; it must not widen
only the first preflight check.

## Scope and Non-Goals

In scope are a measured final-signature inventory; a cycle-safe recursive copyable-value
classifier over already-selected scalar/vector leaves, fixed arrays, and structs; exact local
`Ptr`/`OutParam`/`BorrowInOutParam` relations over those values; consistent helper result,
parameter, call, return, local-storage, block-parameter, type-lowering, layout, and emission use;
focused fake/real tests; promotion of newly unlocked corpus fixtures; O0/O3 census delta; and
durable capability/Pareto updates.

Out of scope are fixture-name or source-syntax checks, compatibility fallbacks, new existential or
dynamic-dispatch representations, arbitrary device/global/shared pointer transport, raw address
space erasure, BFloat16/FP8, matrix operations whose canonical lowering is not a selected finite
value, new arithmetic/libdevice semantics reached after the ABI gate, general `GenericAsm`
semantics, provider ABI 25 without a demonstrated interface gap, and unrelated aggregate layout or
resource-operation widening.

## Architecture and Invariants

- The final linked `IRFunc` signature is canonical. Parameter and result admission depends only on
  exact IR type/role, never the helper name, source file, or intrinsic spelling.
- A copyable value is finite, acyclic, and composed recursively from admitted scalar/vector leaves,
  fixed positive-size arrays, and non-empty structs. Its LLVM value representation is built using
  existing generic builder types and aggregate operations.
- Pointer-kind and address-space semantics remain explicit. A caller `Ptr<T>` may satisfy an exact
  local mutable `OutParam<T>` or `BorrowInOutParam<T>` only for the same canonical `T`; global,
  shared, and thread-local context pointers retain their separately owned contracts.
- Preflight, requirement collection, type lowering, and emission call the same classifier. A cached
  provider type handle never turns a source type legal for a role that rejected it.
- Layout-sensitive external storage aggregates are checked using CUDA layout at the canonical
  boundary before provider mutation. Internal local/helper values use one provider representation;
  the slice does not reinterpret external padding or invent a packed representation.
- Invalid recursive aggregates, non-literal/unbounded arrays, unsupported leaves, and mismatched
  pointees fail deterministically before builder discovery.

## Interfaces and Dependencies

The implementation is expected to touch `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` and
`source/slang/slang-emit-nvvm.cpp`, with focused coverage in
`tools/slang-unit-test/unit-test-nvvm-emitter.cpp` and, only if generic provider behavior lacks
coverage, `tools/slang-unit-test/unit-test-nvvm-compiler.cpp`. Existing `NVVMIRBuilder` operations
remain authoritative. Direct fixture lanes live next to their native CUDA directives.

The Release host build and isolated provider build use the existing CMake configuration outside
the sandbox. Runtime validation requires CUDA 12.9, the RTX 5090 host GPU, and
`SLANG_NVVM_BUILDER_PATH` pointing to
`build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`.

## Milestones

1. Extract the first rejected parameter/result type from final linked IR for every helper-cluster
   workload that the harness can specialize. Group exact types by representation rather than
   fixture, name their canonical producing pass/function, and select only the finite copyable/local
   pointer groups owned by this slice.
2. Add focused negative and positive fake-builder tests that describe the recursive value and
   exact pointer relation. Prove the test fails at the current helper signature boundary without
   adding a temporary compatibility path.
3. Implement one recursive classifier and route preflight, requirement/layout collection, role-
   based type lowering, local storage, call arguments, returns, and emission through it. Remove any
   narrower helper-specific classification made redundant by the new source of truth.
4. Build Release host/provider and run focused real PTX/runtime tests. Rerun the original helper
   cluster at O0/O3, audit each newly reached first failure, and retain only transitions explained
   by the new invariant.
5. Add direct O0/O3 coverage for every workload that becomes correct, update the census matrix,
   report, capability ledger, and MVP numerator/Pareto counts, then run representative and selected
   regression gates.

## Validation and Acceptance

Acceptance requires an exact signature inventory for all 51 MVP helper-cluster rows; focused tests
for scalar Double local out/inout transport, recursive array/struct by-value transport, exact
pointee matching, unsupported recursive/unsized/address-space neighbors, and provider-observed
generic type/pointer/call topology; Release host and provider builds; PTX accepted by CUDA 12.9
`ptxas`; O0 and O3 differential runtime success for every promoted fixture; all three Slice 131
representative workloads; the complete selected NVVM prefix; regenerated census rows and Pareto
counts; formatting; `git diff --check`; and no staged `external/slang-binaries/` content.

An implementation that merely changes 51 helper diagnostics to later failures is not complete.
The report must distinguish newly correct workloads from newly exposed clusters and quantify both.

## Failure and Recovery

If a newly admitted value reaches an unsupported operation, classify that next canonical producer
instead of adding it to this slice unless it is necessary to transport the admitted value itself.
If LLVM verification or libNVVM rejects a generic value topology that the fake builder accepted,
retain the real IR/PTX log, narrow the canonical contract, and add a pre-provider negative test. If
a representation needs a provider operation absent from ABI 24, isolate the smallest proof before
revising the ABI. All corpus probes and final-IR dumps remain under ignored `build/nvvm-census/`.

## Self-Review

Before completion, inventory every new helper, fallback, and special case in the diff. For each,
name the exact final IR shape, its producer, why it is canonical, the test that fails without the
change, and why this layer owns it. Remove source-name checks, syntax reconstruction, structural
equivalence helpers, duplicated type classification, silent defaults, and downstream patches for
malformed IR. Perform the revert drill on the central classifier when practical.

## Artifacts and Hand-Off

Keep final-IR dumps, per-signature inventory, focused census logs, PTX, cubins, and measurement
tables below ignored `build/nvvm-census/slice132-*`. Commit this completed plan with the compiler,
tests, fixture promotions, regenerated census evidence, and durable design/ledger updates. The
outcome must state how many of the 51 helper rows became correct, how many moved to each next
cluster, and what the aggregate-transport slice should reuse.
