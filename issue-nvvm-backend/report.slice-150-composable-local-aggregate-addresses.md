# Slice 150 report: composable local aggregate addresses

## 1. Motivation

Consider this ordinary local mutation:

```slang
struct Payload
{
    int2 lanes;
    int value;
}

Payload values[2];
values[index].value += 1;
```

Final CUDA linking represents this as a canonical `IRGetElementPtr` selecting `Payload` from the
fixed array, followed by `IRFieldAddress` selecting `value`. Direct NVVM already accepted and
emitted the array-element pointer. It also accepted direct fields of a local `Payload`, but it
could not compose those two proven operations. `compute/pointer-emit.slang` therefore stopped at a
typed `Ptr<int>` field-address diagnostic even though the generic builder already modeled both
steps.

The frozen `pointer/const-ref.slang` workload exposed the other missing root role:

```slang
int test(__constref Thing thing)
{
    return thing.bigArray[0];
}
```

Helper ABI and pointer validation already accepted the exact `BorrowInParam<Thing>` parameter, but
field resolution did not reuse that proof. The discovery Pareto grouped these with fields in
collected global parameters. Linked-IR tracing showed those global rows are different: one contains
a structured-buffer element with `DescriptorHandle<Texture2D>`, and another contains a parameter
group whose specialized element is outside the current storage algebra. Widening field provenance
for those rows would hide their actual first unsupported representation.

## 2. Proposed solution

Compose only already-proved canonical aggregate address roles:

1. Reuse `_getNVVMSequentialElementPointer` when an `IRFieldAddress` base is the exact
   `IRGetElementPtr` producer.
2. Reuse `asNVVMSupportedLocalCopyableValuePointerType` for `out`/`inout` copyable struct helper
   parameters.
3. Reuse `asNVVMSupportedHelperReferencePointerType` only for an exact helper `IRParam`, preserving
   its access qualifier.
4. Find the selected field by semantic key, require exact declared/result pointee identity, and
   inherit root mutability.

Preflight and emission continue to call the same resolver. Emission uses the existing
`emitSequentialElementPointer` and `emitStructFieldPointer` operations, so forward-only provider
ABI revision 30 is unchanged.

## 3. Change summary

- `source/slang/slang-emit-nvvm.cpp`
  - makes sequential-element provenance available to field resolution;
  - composes fields after an exact admitted aggregate element;
  - admits exact copyable `out`/`inout` and immutable helper-reference roots;
  - preserves root access and all existing key/type checks.
- `tools/slang-unit-test/unit-test-nvvm-support.h` and
  `tools/slang-unit-test/unit-test-nvvm-emitter.cpp`
  - add one focused source combining fixed-array selection, direct field mutation, an `out` helper,
    and a `__constref` helper;
  - prove that the existing generic sequential and field pointer callbacks receive both selected
    element and helper-parameter bases.
- `tests/compute/pointer-emit.slang` and
  `tests/language-feature/pointer/const-ref.slang`
  - add stable direct-NVVM O0/O3 differential lanes.
- `issue-nvvm-backend/census.slice-150*` and
  `issue-nvvm-backend/discovery-census.slice-150*`
  - preserve separate frozen-v1 and discovery evidence, classifications, and Pareto data.
- `issue-nvvm-backend/discovery-metrics-workloads.slice-150.json`
  - adds the larger local-pointer kernel to the established exploratory compile/PTX gates.

## 4. Concepts and vocabulary

- **Sequential aggregate pointer:** the typed pointer produced by canonical `IRGetElementPtr` for a
  fixed array or vector after the resolver proves the aggregate, index, result type, layout, and
  access.
- **Root role:** the storage provenance and mutability established before member selection and
  inherited by every child address.
- **Helper reference:** an exact `BorrowInParam`, `BorrowInOutParam`, or related helper ABI pointer;
  this slice uses the existing classifier and requires the canonical parameter producer.

## 5. Process report

### The census operation label did not identify one representation

The Slice 149 discovery Pareto listed three healthy rows at
`aggregate-struct-field-pointer`. Inspecting final linked IR separated them:

- `compute/pointer-emit.slang` first used
  `IRFieldAddress(IRGetElementPtr(Ptr<Array<Something, 2>>, index), field1)`;
- `gh-6657-bindless-uniform.slang` selected a conventional global
  `StructuredBuffer<Data>`, where `Data` contains `DescriptorHandle<Texture2D>`;
- `type-legalize-bug-1.slang` selected a conventional global output beside
  `ParameterBlock<B>`, whose specialized element is not yet admitted.

Both global structs retain `IRSynthesizedParameterGroupDecoration` before and after final pipeline
passes. Their rejection is not missing producer metadata. `_getNVVMConventionalGlobalParams`
validates the complete collected storage type because provider emission must lay out the whole
block, not only the first used sibling. Those two rows remain exact preflight failures and are not
special-cased in this slice.

### Sequential selection and field selection form one canonical path

`IRBuilder::emitElementAddress` produces `IRGetElementPtr`; `_getNVVMSequentialElementPointer`
proves its base, index, aggregate type, exact result pointee, layout, address space, and access.
`IRBuilder::emitFieldAddress` then uses that exact result. The field resolver now calls the
established element resolver, accepts only a supported struct pointee, and inherits
`isImmutable`. It does not walk arbitrary operands or reproduce the element/type rules.

The shape is canonical and intentionally allowed. Removing this branch returns
`compute/pointer-emit.slang` to the typed field-address preflight failure. The existing provider
emits the path as one sequential element pointer followed by one struct field pointer; no new
callback or LLVM representation is needed.

### Helper root classifiers remain the source of truth

`pointer-emit.slang` later reaches `out Something`, whose type is already accepted by
`asNVVMSupportedLocalCopyableValuePointerType`, and `pointer/const-ref.slang` reaches
`BorrowInParam<Thing>`, already accepted by `asNVVMSupportedHelperReferencePointerType`. Field
resolution now reuses those classifiers instead of spelling the pointer operands again.

Only an `IRParam` can own the helper-reference case. An immutable borrow reports
`isMutable = false`, so `_validatePointerValue` continues to permit only exact loads and rejects a
write. Mutable copyable helper storage retains write access. Field-key lookup and declared/result
type identity remain unchanged. The focused fake-provider source requires selected-array,
`OutParam`, and `BorrowInParam` paths; its callback trace fails if any one is removed.

### Permanent tests protect two distinct combinations

`compute/pointer-emit.slang` combines nested local structs/arrays, dynamic selection, out/inout
helpers, a thread-local explicit-global context, vectors, and a structured-buffer output.
`pointer/const-ref.slang` combines a 128-element struct field, loops, repeated immutable helper
calls, barriers, and a structured-buffer output. Both have deterministic existing inputs and
reference outputs and are correct through the real provider at O0 and O3. Their four exact direct
lanes pass 4/4.

The whole `pointer/const-ref.slang` prefix still observes an unrelated WGPU synthetic-lane failure
on this machine; its native CUDA and both new direct-NVVM lanes pass. The slice does not alter or
claim the WGPU infrastructure issue.

### Coverage remains split across frozen v1 and discovery

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references:

| Frozen corpus-v1 metric | Slice 149 | Slice 150 |
|---|---:|---:|
| Direct O0 correct | 371/427 | 372/427 (87.1%) |
| Direct O3 correct | 375/427 | 376/427 (88.1%) |
| Correct in both modes | 371/427 | 372/427 (87.1%) |
| Newly correct in both modes | — | 1 |
| Old-correct regressions | — | 0 |
| Selected NVVM regression prefix | 425/425 | 426/426 |

The newly correct frozen identity is
`language-feature/pointer/const-ref.slang#cuda-1`. Across all tiers, native NVRTC O3 has 449
correct and three infrastructure rows. Direct O0 has 385 correct, 52 preflight, eight runtime
mismatches, and seven provider failures; direct O3 has 390 correct, 52 preflight, eight runtime
mismatches, and two provider failures.

The separate discovery corpus remains 82 selected workloads with 72 healthy references:

| Discovery metric | Slice 149 | Slice 150 |
|---|---:|---:|
| Direct O0 correct | 50/72 | 51/72 (70.8%) |
| Direct O3 correct | 50/72 | 51/72 (70.8%) |
| Correct in both modes | 50/72 | 51/72 (70.8%) |
| Newly correct in both modes | — | 1 |
| Old-correct regressions | — | 0 |

The newly correct discovery identity is `compute/pointer-emit.slang#discovery-1`. Across all 82
rows, native NVRTC O3 has 72 correct, two runtime mismatches, and eight infrastructure results.
Each direct mode has 51 correct, 22 preflight, one provider, seven infrastructure, and one runtime
mismatch result.

The leading remaining healthy discovery clusters are now eight distinct two-row groups: typed
global field pointers, array-element pointer relations, sequential aggregate pointers,
entry-point parameters, function identity, helper aggregate parameters, helper pointer parameters,
and helper resource results. The former three-healthy-row field cluster therefore shrinks to two
without merging it with the local aggregate invariant. Corpus v1 and discovery denominators are
not combined, and no corpus v2 is proposed.

### Exploratory performance and architecture evidence

For `compute/pointer-emit.slang`, three standalone compilations per configuration give:

| Configuration | Median compile | PTX size | Cubin size |
|---|---:|---:|---:|
| NVRTC O3 native | 347.1 ms | 8584 B | 13664 B |
| Direct NVVM O0 SM70 | 237.4 ms | 8667 B | 4840 B |
| Direct NVVM O3 SM70 | 242.3 ms | 645 B | 2792 B |
| Direct NVVM O3 SM80 | 239.3 ms | 645 B | 2920 B |
| Direct NVVM O3 SM90 | 240.4 ms | 645 B | 3360 B |

CUDA 12.9 `ptxas` accepts direct O3 output for this kernel and all eight established discovery
measurement gates at SM70, SM80, and SM90. Census end-to-end times include compilation, loading,
execution, and comparison; these numbers remain uncontrolled exploratory evidence. CUDA 13 and
physical SM70/SM80/SM90 runtime workers remain open productionization requirements.

### Self-review inventory

- The relocated sequential-pointer record and forward declaration survive. Mutual field/element
  composition consumes one canonical producer per recursion step and does not inspect an arbitrary
  graph.
- The local-copyable helper root survives. It reuses the existing pointer and copyable-struct
  classifiers and is required by the real `out Something` helper.
- The helper-reference root survives. It is limited to an exact `IRParam`, preserves read access,
  and is required by `pointer/const-ref.slang` and the focused unit source.
- The sequential-parent branch survives. It delegates all relation/layout/access validation to the
  established element resolver and is required by `pointer-emit.slang`.
- Both promotions survive. They protect different semantic combinations and pass native/direct
  differential execution at O0 and O3.

The diff adds no fixture-name check, syntax reconstruction, compatibility fallback, custom type
equivalence, provider ABI widening, or downstream repair of malformed IR.
