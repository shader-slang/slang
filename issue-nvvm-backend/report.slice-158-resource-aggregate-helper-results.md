# Slice 158: Canonical resource-aggregate helper results

## 1. Motivation

Consider a helper that returns an ordinary struct containing a resource:

```slang
struct Things
{
    int first;
    RWStructuredBuffer<int> rest;
}

Things getThings()
{
    return gThings;
}
```

Final linked IR preserves `%Things = {Int, RWStructuredBuffer<Int>}` exactly. The constant-buffer
load produces that type, `getThings` returns it, its caller receives it, and field extraction uses
both members. Direct NVVM already used the same first-class provider struct for helper parameters
and ordinary values, but rejected it specifically as a helper result. Discovery therefore exposed
one healthy cross-layer blocker despite all required provider operations already existing.

Frozen `optional-single-concrete-layout` has the same root representation with an additional
canonical construction step. Type-flow lowering produces `%Tuple = {UInt tag, FooImpl payload}`;
`FooImpl` contains a `StructuredBuffer<int>`, and `makeValue` builds and returns the exact tuple.

## 2. Proposed solution

Use `asNVVMSupportedResourceStructType` as the single classifier for finite resource-bearing
structs in helper-result preflight, helper-result type legality, and explicit ordered
`makeStruct` construction. Keep the same recursively lowered provider struct handle used by
helper parameters and ordinary values. Reuse the existing generic function, call, value-return,
aggregate-construction, and aggregate-extraction operations.

Do not add a provider callback, revise the ABI, infer source syntax, or accept neighboring
parameter-block, borrowed aggregate, resource-array, append/consume, pointer-indirection, FP8, or
BFloat16 shapes.

## 3. Change summary

- `slang-emit-nvvm.cpp` admits the existing resource-struct classifier for helper results and
  canonical ordered aggregate construction.
- `slang-emit-nvvm-type-lowering.cpp` makes the `HelperResult` role consistent with helper
  parameters and ordinary values.
- The focused fake-provider fixture transports one resource struct through an exact helper result
  and call result while retaining its established local-storage and texture-use coverage.
- `return-opaque-type-in-struct.slang` gains stable direct-NVVM O0/O3 differential lanes.
- Frozen-v1, discovery, Pareto, measurement-manifest, design, ledger, plan, and report artifacts
  record separate denominators and the newly exposed frozen blocker.

## 4. Concepts and vocabulary

- **Resource struct**: a finite non-empty `IRStructType` whose recursive field leaves are selected
  executable scalar or resource values, as proved by `asNVVMSupportedResourceStructType`.
- **Role legality**: the type-lowering check that decides whether an otherwise representable type
  may be used as a helper result, helper parameter, body value, storage value, or launch ABI value.
- **Canonical aggregate construction**: `makeStruct` with one ordered operand for every exact
  declared field in its exact result type.
- **Advanced row**: a workload that passes the slice's blocker but reaches an independent later
  blocker; it is not counted as correct.

## 5. Process report

The input-shape audit begins after specialization, type-flow lowering, parameter-group lowering,
and linking. In `return-opaque-type-in-struct`, the producer keeps `%Things` as a two-field struct;
`getThings` loads it from `ConstantBuffer<Things>`, returns it with `return_val`, and `test` calls
the helper before extracting `first` and `rest`. In `optional-single-concrete-layout`, type-flow
lowering resolves the single concrete interface payload to `FooImpl` and creates the ordinary
tagged tuple `%Tuple = {UInt, FooImpl}`. `makeValue` constructs one tuple for each tag and returns
it. These are canonical, intentionally distinct user types, not alternate spellings that an
upstream producer should collapse.

`_validateNVVMHelperTarget` checks the exact linked result with
`_isSupportedNVVMHelperResultType`. That gate omitted resource structs even though
`_isSupportedNVVMHelperParameterType` accepted the same classifier. Later,
`NVVMTypeLoweringContext::lowerType` repeated the asymmetry in `NVVMTypeUse::HelperResult` while
already accepting the exact type in `HelperParameter` and `Value`. Adding the existing classifier
to both result gates establishes one role invariant and avoids a second structural matcher.

The first frozen probe then reached `makeStruct(%Tuple)`. `_getNVVMAggregateConstruction` already
owns canonical explicit aggregate construction: it verifies exact result kind, exact operand
count, ordered declared fields, and exact field/operand type equality before emission. It admitted
ordinary helper structs and physical-array wrappers but omitted the same accepted resource struct.
Reusing `asNVVMSupportedResourceStructType` there is therefore the construction half of the same
value representation, not a fixture-specific fallback. Removing this widening restores the exact
`makeStruct` first blocker in the frozen test.

No custom equivalence, graph walk, syntax reconstruction, fixture-name check, compatibility path,
or downstream IR-text patch was added. Generic provider operations already preserve the exact
type from struct creation through function declaration, value return, call result, and field
extraction. Provider ABI revision 30 remains unchanged.

The discovery workload becomes differentially correct at direct O0 and O3 and gains two permanent
lanes. The frozen row advances to `defaultConstruct<StructuredBuffer<int>>`, produced by
`TypeFlowSpecializationContext::specializeMakeOptionalNone` for the optional `none` payload's
default untagged-union value. `_validateNVVMFunction` reports
`direct NVVM lowering does not support Slang IR instruction or shape 'defaultConstruct'`.
Materializing a null raw-buffer view is a different canonical operation and may require its own
exact null-pointer/value recipe, so this slice records it without widening further.

Frozen corpus v1 remains exactly 452 workloads/427 healthy MVP references and stays at 384 O0,
388 O3, and 384 both-mode correct (89.9%/90.9%/89.9%), with zero old-correct regression. Across
all rows, native CUDA remains 449 correct/three infrastructure; direct O0 remains 397 correct,
42 preflight, eight runtime mismatch, and five provider; direct O3 remains 402 correct, 42
preflight, and eight runtime mismatch.

Discovery remains exactly 82 workloads/72 healthy native references and improves from 59/59/59 to
60/60/60 (83.3%) with zero old-correct loss. Each direct mode has 60 correct, 12 preflight, two
provider, seven infrastructure, and one runtime mismatch. The one-row healthy
`helper-aggregate-result-abi` cluster is eliminated. The selected NVVM unit prefix passes 427/427.

All fourteen representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90.
The new resource-aggregate-result gate measures 252.1 ms and 875-byte PTX at direct O3 SM70 versus
387.0 ms and 8841-byte PTX through NVRTC O3. Direct O0 measures 247.9 ms and emits 2807-byte PTX.
These measurements remain exploratory. The promoted source's two direct CUDA lanes pass; its full
file run also encounters an unrelated synthesized WebGPU bind-group-layout failure on this
machine, which does not affect the direct CUDA differential result.

The self-review inventory contains no new helper or fallback. Three exact classifier uses survive:
helper-result preflight, helper-result role legality, and canonical resource-struct construction.
The two motivating linked-IR traces prove the first two own the result boundary; the frozen
`makeStruct` failure and `_getNVVMAggregateConstruction`'s existing exact operand contract prove
the third owns construction. Removing any one restores its named first blocker. The provider ABI,
corpus identities, corpus-v2 proposal, and unrelated untracked `external/slang-binaries/` remain
untouched.
