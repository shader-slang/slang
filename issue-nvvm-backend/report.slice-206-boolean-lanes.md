# Slice 206: Legalize private Boolean vector lane accesses

Status: accepted full checkpoint after parent implementation, preservation and provenance review.

## Motivation

The unchanged tiled-brass `eval_buffer` entry rejected the Boolean vector result of its
`any(isnan(eval)) || any(isinf(eval))` checks. Consider the equivalent source:

```slang
float3 values = float3(asfloat(inputBuffer[index]), asfloat(inputBuffer[(index + 1) % 6]), 1.0);
bool3 nanLanes = isnan(values);
bool3 infLanes = isinf(values);
outputBuffer[index] = uint(any(nanLanes) || any(infLanes));
```

The generic vector intrinsics build their result in a loop. Fresh final IR has a local
`Ptr<Vec<Bool,3>>` result and `GetElementPtr` scalar Boolean stores. Direct NVVM's sequential
pointer classifier rejected that shape. The source itself is valid, but the provider's packed
LLVM Boolean vectors cannot supply independent byte-addressable scalar lanes.

Two independent runnable fixtures reproduce the boundary. Runtime NaN, positive/negative infinity,
signed zero and finite inputs produce the classification masks `[88,74,65,0,0,80]`. A separate
fixture updates bool2/3/4 at dynamic indices, reads back updated and neighboring lanes, and completely
initializes an uninitialized bool4 in a runtime-dependent order. Its exact output is
`[5260,5859,5176,6063]`. Both final formatted sources passed NVRTC O3 and failed NVVM O0/O3 on a
deliberately reverted compiler before passing all six modes with the change.

## Proposed solution

Normalize valid, nonescaping lane reads/writes on a direct private local vector into existing
vector value operations in `legalizeIRForNVVM`, before supported-IR preflight. A lane load becomes
a whole-vector load and extraction. A lane store loads the vector, selects the new value for its
indexed lane while preserving every other lane, constructs the resulting vector and stores it.
The canonical packed value representation, layouts, address-space mapping and provider ABI 35 stay
unchanged. Numeric pointers and external Boolean storage retain their existing owners.

## Change summary

- `source/slang/slang-ir-nvvm-legalize.cpp` adds one documented target legalization helper and its
  invocation before preflight.
- Two new runnable CUDA fixtures supply independent dynamic-lane and intrinsic output oracles;
  two disjoint discovery selections register them without changing frozen or prior discovery contracts.
- A diagnostic fixture preserves rejection of a lane address passed to a noinline `inout` helper.
  Two cases in the existing fake-provider rejection unit preserve shared/external Boolean lane
  rejection before provider loading or module mutation.
- The completed plan, portable outcome TSVs, manifest, STATUS and design facts retain scope,
  provenance and the complete preservation audit. Raw logs and fresh IR remain under ignored build/.

## Concepts and vocabulary

A **semantic lane address** is Slang's valid `GetElementPtr` result for a vector subscript. It need
not have a directly addressable LLVM counterpart. A **packed Boolean vector** stores its semantic
lanes in LLVM `<N x i1>`; scalar `i1` allocation stride is a byte, so its GEP cannot identify those
packed bits. **Private local storage** here means a direct `Var` root, not a shared or buffer root.
A **nonescaping lane address** has only load/store address-operand users.

## Process report

`IRBuilder::emitElementAddress` creates the semantic vector subscript. The shared buffer-element
pass's `fixBufferAccessPointerTypes` calls `copyBufferLayoutToPointer`, retaining Generic/ReadWrite
and attaching ScalarLayout. Fresh slice 206 focused and material final IR shows vector `isinf` and
`isnan` functions with local result Vars and loop-indexed Boolean stores. This is fresh corroboration
of the producing path, not a claim that debug instrumentation identified the first preflight
instruction. Accepted205's exact rejection is retained as before evidence.

`NVVMTypeLoweringContext::lowerType` represents local value vectors with LLVM vectors of the
semantic scalar type, making bool3 `<3 x i1>`, with alignment1 from
`getNVVMCopyableValueAlignment`. The numeric classifier in `_getNVVMSequentialElementPointer`
therefore must continue rejecting Boolean lane GEPs. The provider's
`_emitSequentialElementPointer` emits a real LLVM GEP, which is not an appropriate packed-bit
operation. No pointer classifier was broadened and no scalar/vector storage cast was added.

`_legalizeNVVMLocalBooleanVectorAddresses` is the sole new production helper. It requires a direct
`Var` root admitted by `asNVVMSupportedLocalCopyableValuePointerType`, a Boolean vector admitted by
`asNVVMSupportedValueVectorType` (exactly 2–4 lanes, bounding `lanes[4]`), and a result admitted by
`asNVVMSupportedDerivedCopyableValuePointerType`. The latter owns Generic address space and the
canonical three-operand/no-layout or four-operand/ScalarLayout spelling. The new pass additionally
requires ReadWrite, exact same Boolean element via `isTypeEqual`, and a signed/unsigned 32-bit index.
It checks every use before rewriting any use. An escaping address remains for normal preflight
rejection; it is not copied into a temporary or silently assigned a default. The new helper survives
the revert drill because both final runnable sources fail at this boundary without it.

For a source assignment `flags[index] = value`, each generated lane selects `value` if its constant
index equals the source index, otherwise the extracted old lane. Source evaluation order is
preserved by inserting at each original load/store. This keeps already initialized neighbors
unchanged. For a defined in-range index, the first store into an uninitialized vector defines its
selected lane without requiring the old lane's value. Other lanes remain unspecified until assigned.
By induction, completely initializing all lanes defines the whole vector. No uninitialized lane
controls a branch or address. The pinned LLVM 14 semantics say an uninitialized alloca load yields
undef, and select depends only on its condition and selected operand; vector construction inserts
lanes independently. The rotated initialization test corroborates this reasoning at O0/O3.
See the [LLVM 14 alloca](https://releases.llvm.org/14.0.0/docs/LangRef.html#alloca-instruction) and
[poison dependence](https://releases.llvm.org/14.0.0/docs/LangRef.html#poison-values) contracts.

Existing alternatives were audited. `AddressInstElimination` is an autodiff transformation that
walks arbitrary address chains and copies mutable call arguments. It introduces `UpdateElement`,
which has no direct NVVM consumer; existing peephole expansion handles constant aggregate cases,
not dynamic Boolean vectors. Applying that broad pass or adding another provider operation would
extend scope without improving the representation. Existing buffer-element physical lowering
selects resources and UserPointer/Input/Output pointers; its LLVM policy handles matrices and does
not own generic private vectors. Reusing the established compare/select/extract/construction
pipeline is the target-specific normalization required here, not a repair to malformed frontend IR.

Shared/external lanes are explicitly excluded because their separate writers cannot safely become
whole-vector read/modify/write operations. Fake-provider tests prove those cases reject before any
provider mutation. The escaping helper fixture checks O0/O3 rejection. Existing numeric local vector,
invalid sequential provider addressing, incompatible structured storage and readonly pointer tests
preserve adjacent element/layout/access/address-space contracts; malformed source-inexpressible IR
is not claimed as new source-level coverage.

The full checkpoint is required for the lowering change. All six registered complex cells now
compile and assemble, with unchanged material source. There is no next compiler blocker in this
registered complex corpus. Material bindings, textures/LUT/input and expected-output contracts are
still absent, so no material runtime or performance claim follows. Independent wave-transport
failures remain candidates for a later slice; they were not investigated here.

### Validation record

Final required gates already completed: focused 16/16, separate escaping-pointer diagnostics 2/2,
runtime 4/4, NVVM/routing/reporter units 473/473 with one Windows-only skip, toolkit 18/18 and
complex compile/assembly 6/6. Full frozen/discovery exact preservation audit passes: all 1617 old cells retain exact classifications, return codes, execution counts, diagnostics and canonical shapes. Six new cells pass, for 1623 fresh cells (1562 correct, 61 unchanged known failures), with zero inherited cells or missing/duplicate/extra keys. Frozen correct counts are 449/438/438 over 452 identities; discovery counts are 79/79/79 over 89 identities.
The final compiler library is SHA256
`f22b30cc8732d1794dc9ce33a8c5f7901892949dfae6c8eba574bfae30ade933`; the unchanged provider is
`1f3ef9bd03de64838dc039a97ec30f4fe00cd33682d0d95b06abac895446124f`.

The partial preformat run is historical evidence only. Its interruption was caused by correcting
formatter tool discovery and two whitespace-only helper wraps, not a GPU failure or compiler
regression. Final binaries were rebuilt and every required gate restarted. The standalone diagnostic
annotation was then corrected to the exact harness syntax; its failed harness log is retained, and
the corrected test executed successfully in both modes without a compiler change.
