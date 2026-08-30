# Slice 137 numeric resources and Half helper ABI report

## 1. Motivation

The Slice 136 healthy-MVP Pareto report placed aggregate/pointer/layout transport second at 23
workloads. Six adjacent failures were conventional numeric structured buffers whose final linked
IR already used the established resource topology but whose element was Half or Double instead of
the bring-up-era 32-bit subset. Consider `conversion-to-half.slang`:

```slang
RWStructuredBuffer<half> outputBuffer;

[noinline] void storeValue(half v)
{
    outputBuffer[i++] = v;
}
```

`collectGlobalUniformParameters` creates the keyed `GlobalParams` resource field. Buffer lowering
produces the typed `RWStructuredBufferGetElementPtr` and store, and the noinline overload remains a
direct helper with a scalar Half parameter. Before this slice, direct NVVM stopped at the collected
resource field even though its generic resource view and memory operations could represent the
element.

The goal was not to add six fixture cases. It was to establish the reusable external numeric
resource invariant, follow every newly exposed first blocker, and promote only workloads correct
at both O0 and O3.

## 2. Proposed solution

The resource classifier now reuses `isNVVMSupportedNumericValueType`, the existing selected scalar
and two- through four-lane vector algebra. Every raw structured-buffer boundary separately checks
that CUDA and provider size/alignment agree before module creation. Generic resource-view,
pointer-offset, load, store, and aggregate operations remain the only emission path.

The newly reachable conversions exposed two valid canonical operations. Boolean-to-Half/Double is
the existing typed integer-to-float family with a Boolean operand. More importantly, direct LLVM
`half` helper arguments are not a reliable libNVVM O3 call ABI: O0 PTX initialized each caller
parameter with `st.param.b16`, while O3 omitted that store and the callee loaded uninitialized bits.

Canonical Slang IR remains Half. Only the physical LLVM helper boundary uses i16, with exact bit
reinterpretations at helper entry, call arguments/results, and returns. One shared helper-return
path covers ordinary `IRReturn` plus value GenericAsm, compound wave, surface, and texture helper
producers. Revision 27 already expresses integer types and bit reinterpretation, so no provider
callback or ABI revision is needed.

## 3. Change summary

- `slang-emit-nvvm-type-lowering.*` admits selected numeric structured-buffer elements and gives
  scalar Half helper parameters/results a role-specific i16 representation.
- `slang-emit-nvvm.cpp` checks external raw-buffer layouts, records both Half/i16 capability
  requirements, applies the physical boundary at calls/entries/returns, and routes every non-void
  helper return through one implementation.
- The shared semantic catalog admits Boolean integer-to-float inputs and names the UInt16 semantic
  type used by the physical boundary. Builder ABI and `slang-llvm-nvvm` stay at revision 27.
- Focused fake-provider coverage proves Half/Double scalar/vector resource views, Boolean
  conversions, the i16 helper signature, both reinterpretation directions, and retained Boolean
  and incompatible-layout negatives.
- Eleven conversion fixtures receive 22 direct O0/O3 regression lanes. The census summarizer and
  committed 452-row matrix classify the new structured-buffer element-layout diagnostic under its
  aggregate/layout owner.

## 4. Concepts and vocabulary

- **Canonical helper signature**: the post-specialization linked `IRFunc` result and parameter
  types. Half remains the semantic type here even when the target call representation is i16.
- **Physical helper ABI**: the LLVM-level type used only to transport a value across a direct call.
  Bit reinterpretation preserves every Half bit and does not perform numeric conversion.
- **External resource layout**: the CUDA-visible size, alignment, and stride of a structured-buffer
  element. It must agree with the provider representation before generic typed pointer arithmetic
  is valid.
- **Healthy MVP reference**: one of the 430 MVP workloads whose native CUDA/NVRTC comparison is
  correct. Three native infrastructure failures leave 427 healthy references.

## 5. Process report

### The resource widening is owned by the canonical resource producer

`collectGlobalUniformParameters` synthesizes `ConstantBuffer<GlobalParams>` and preserves each
resource field key. HLSL/resource lowering produces the selected raw view and typed
`RWStructuredBufferGetElementPtr`; `fixBufferAccessPointerTypes` carries the buffer layout on the
pointer. `_getNVVMConventionalGlobalParams`, `getNVVMSupportedRawBufferType`, and the typed access
resolvers consume exactly those shapes.

The old `_isNVVMSupportedResourceElementType` duplicated a narrower leaf list even though type
lowering and memory emission already supported the selected numeric algebra. Replacing that list
with `isNVVMSupportedNumericValueType` removes the duplicate source of truth. The new external
layout check is not a fallback: it proves the condition under which the existing provider
representation is the correct CUDA resource representation. Boolean buffers and incompatible
aggregates fail deterministic preflight rather than being packed or reconstructed downstream.

Six old `struct field address` failures advance past this boundary. Three reach exact ordinary
GenericAsm blockers, one matrix reaches `unmodified`, and the Double/Half conversion workloads
reach their ordinary casts. These transitions demonstrate that resource transport was the first
root cause without pretending every later operation was part of the same feature.

### Boolean conversion uses the existing typed semantic family

The source overload `storeValue(bool v) { outputBuffer[i++] = (half)v; }` produces canonical
`kIROp_CastIntToFloat` with Boolean input and Half output. `_resolveNVVMValueOperation` already
derives both exact semantic types and the provider implements nonsigned integer-to-float with the
unsigned conversion. Admitting Boolean in `ValueOperationFamily::IntegerToFloat` therefore reuses
the established operation rather than adding a spelling, callback, or fixture branch.

Removing this widening restores the exact `castIntToFloat` preflight failure in the three newly
compiling conversion workloads. The focused unit source proves both Boolean-to-Half and
Boolean-to-Double descriptors.

### The O3 runtime mismatch belongs to target helper transport

After resource and Boolean conversion admission, `conversion-to-half.slang` and
`conversion-to-double.slang` were correct at O0 but mismatched at O3. The generated modules made
the failure concrete. For every noinline Half overload, O0 caller PTX contained:

```ptx
st.param.b16 [param0+0], %hN;
```

O3 declared the same parameter slot and the callee still used `ld.param.b16`, but the caller store
was absent. The failing values began exactly at the Half-input overload in each 71-element output,
so the canonical Slang value and numeric conversion were not the broken representations.

`NVVMTypeLoweringContext::lowerType` now selects i16 only for scalar Half under
`HelperParameter` or `HelperResult`. `_emitNVVMHalfHelperABIReinterpretation` crosses the boundary
without changing bits. Call arguments encode Half to i16; helper entry and call results decode i16
to Half; `_emitNVVMFunctionValueReturn` encodes every non-void Half result. Preflight requests both
typed reinterpretations before a provider module exists.

The first full-census attempt found two old-correct helpers whose specialized GenericAsm bodies
returned directly instead of reaching ordinary `kIROp_Return`. That was a real producer-path audit,
not a reason to add two cases: value operations, surface/texture helpers, compound wave recipes,
and ordinary returns now share `_emitNVVMFunctionValueReturn`. Rerunning the exact regression set
and the complete census restores both identities. Removing the physical boundary reproduces the
eight O3 integer-conversion mismatches and the missing PTX parameter stores.

### Promoted workload gates

The following existing fixtures are now correct through native CUDA, direct O0, and direct O3:

- `conversion-to-double`, `conversion-to-float`, and `conversion-to-half`;
- `conversion-to-int8`, `conversion-to-int16`, `conversion-to-int32`, and
  `conversion-to-int64`; and
- `conversion-to-uint8`, `conversion-to-uint16`, `conversion-to-uint32`, and
  `conversion-to-uint64`.

All 70 runnable tests in the conversion prefix pass; 36 unrelated unavailable-backend lanes are
ignored. The 22 new direct lanes are explicit regression tests, not additions to the census
denominator.

### Fixed-denominator coverage and Pareto result

The denominator remains 452 eligible workloads from 448 sources: 430 MVP and 22 extension.
Native CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 279 | 8 | 160 | 5 | 287 |
| Direct O3 | 283 | 8 | 160 | 1 | 291 |

Compared with Slice 136, O0 gains three correct identities and O3 gains eleven; neither mode loses
an old-correct identity. Among 427 healthy MVP references, O0 correctness is 278/427 (65.1%), O3
is 281/427 (65.8%), and both-mode correctness is 278/427 (65.1%). The original 405/405 unit result
remains a selected regression score rather than an overall coverage measure.

The leading healthy-MVP failure clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Helper ABI/type contract | 28 | 28 |
| Ordinary intrinsic GenericAsm | 21 | 21 |
| Wave/reconvergence semantics | 19 | 19 |
| Aggregate/pointer/layout transport | 17 | 17 |
| Ordinary numeric/bit operation | 16 | 16 |
| Residual target marker/undefined value | 9 | 9 |
| Atomic/wave operation | 8 | 8 |

The aggregate cluster falls from 23 to 17. Ordinary GenericAsm rises from 18 to 21 because three
resource failures now expose their actual semantic blocker; that is improved diagnostic depth, not
a regression. The committed census records every remaining first shape, producer, and diagnostic.

### Representative workload and productionization gates

All three release-gate workloads remain differentially correct. Median standalone compile time and
generated PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 395.3 ms / 8,889 B | 267.2 ms / 6,102 B | 271.5 ms / 919 B |
| Parameter-block layout | 372.8 ms / 8,839 B | 249.4 ms / 917 B | 252.4 ms / 793 B |
| Shared control/barriers | 388.5 ms / 9,190 B | 254.1 ms / 1,940 B | 257.8 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
4458.5/4728/4527.1 ms for NVRTC O3, 4248/4597/4299.6 ms for direct O0, and
4304.5/4590/4335.1 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts each representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 runtime
workers remain productionization gaps. Provider discovery/deployment policy is unchanged: the
isolated LLVM 14 provider stays compiler-matched at ABI revision 27.

### Validation and self-review

- Release host and isolated-provider builds pass outside the sandbox.
- Focused Half resource/Boolean conversion/i16 boundary tests pass, including exact typed
  preflight and adjacent negatives.
- The selected NVVM unit prefix passes 405/405.
- The complete conversion prefix passes 70/70 runnable tests, including all 22 promoted lanes.
- The final three-mode 452-workload census has zero old-correct regression.
- All representative runtime comparisons pass and direct O3 PTX assembles for SM70/80/90.

The retained helper inventory is bounded and producer-owned. The numeric classifier removes a
duplicate list; the layout checker proves an external ABI invariant; the Boolean family row maps a
canonical operation; the Half boundary maps a valid final signature to a measured target ABI; and
the shared return helper prevents canonical producers from drifting. No fixture-name check, syntax
reconstruction, compatibility fallback, custom semantic equivalence, provider callback, or
downstream repair of malformed IR is present.
