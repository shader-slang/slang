# Slice 171: Canonical parameter-group vector storage

## Motivation

The frozen aligned cbuffer workload contains a fixed array of three-lane vectors:

```slang
cbuffer Constants
{
    float3 v0;
    float v1;
    float v2;
    float3 v3[2];
    float v4;
    float v5;
    float3 v6[1];
    float v7;
};
```

Direct NVVM compiled this workload but read the wrong values. Its provider pointee contained
`[2 x <3 x float>]`, so LLVM gave `v3` a 16-byte element stride while the canonical CUDA layout
requires stride 12. The unaligned sibling mixed scalar Half, `half3`, `half4`, and `float3`; it
stopped in preflight before its parameter-group pointer could be emitted. Together the two probes
showed that parameter-group type admission and physical vector layout needed one shared invariant.

## Proposed solution

Give every selected parameter-group vector whose LLVM SSA layout differs from CUDA an explicit
storage representation. Three-lane 32-bit vectors use an array of three scalars. CUDA defines both
`half3` and `half4` as eight-byte, four-byte-aligned values, so they use an array of two `half2`
chunks. Fixed arrays recursively lower their element through this storage algebra. Immutable field
and fixed-array-element loads extract physical lanes and reconstruct the ordinary semantic vector.

Use the existing canonical CUDA layout query to prove complete storage size, alignment, field
offsets, and array strides. The revision-32 provider already has generic vector, array, aggregate
extraction, sequential extraction, construction, pointer, and load operations, so it needs no ABI
change.

## Change summary

- Parameter-group aggregate classification includes canonical Half scalars and the exact compact
  Half/three-lane-32-bit vector families.
- Compact vector type lowering emits `[N x scalar]` for three-lane 32-bit vectors and
  `[2 x <2 x half>]` for `half3`/`half4`.
- Fixed numeric arrays recurse through parameter-group storage lowering instead of delegating to
  ordinary value lowering.
- Exact immutable field and array-element loads convert the compact representation back to the
  semantic vector.
- Both cbuffer fixtures gain permanent direct O0/O3 lanes. Frozen/discovery TSV and Pareto JSON,
  a two-gate measurement manifest, the design, capability ledger, plan, and this report retain the
  evidence. Provider ABI revision 32 is unchanged.

## Concepts and vocabulary

**Semantic vector** is the ordinary SSA value used by arithmetic, calls, and swizzles, represented
as an LLVM vector.

**Parameter-group storage vector** is the physical CUDA memory representation behind a constant-
buffer or parameter-block pointer. It may be an LLVM aggregate when an ordinary LLVM vector would
select incompatible size, alignment, or stride.

**Canonical CUDA layout query** is the target-owned size/alignment computation over the final
lowered Slang IR type. It is distinct from source-facing HLSL/SPIR-V offset decorations and from a
retained semantic layout graph that may predate type legalization.

## Process report

The first input-shape audit traced `cbuffer-float3-offsets-aligned.slang` through global-uniform
collection and parameter-group type lowering. The final parameter-group element is a canonical
synthesized struct. CUDA target layout places `v3` at byte 20 with two 12-byte elements, followed
by `v4` at byte 44. `NVVMTypeLoweringContext::lowerType` nevertheless treated every fixed numeric
array as an ordinary Value use. `_lowerArrayType` therefore received `<3 x float>` as its element
and built `[2 x <3 x float>]`; LLVM placed the following fields at offsets 64 and beyond. PTX loads
at those provider offsets explain the runtime mismatch directly. No malformed upstream IR or host
launch error was involved.

The fixed-array shortcut was an accidental alternative representation. Removing it only for
parameter-group storage lets `_lowerArrayType` recurse through the same element use already used by
struct fields. `float3` then becomes `[3 x float]`, the outer array acquires stride 12, and generic
struct field/array element pointers select the correct provider fields without byte-offset patches.
`_getNVVMCompactParameterGroupVectorPointer` now accepts those exact immutable fixed-array element
producers as well as direct field addresses. The load case extracts three scalars and constructs
the existing `<3 x float>` semantic value before any downstream operation sees it.

The second audit traced `cbuffer-float3-offsets-unaligned.slang`. Its source-facing offset
decorations describe the HLSL packing contract, but the CUDA launch ABI is recorded by the target
layout: scalar Half is size/alignment 2/2, while `half3` and `half4` are both 8/4. LLVM's ordinary
Half vector alignment does not reproduce that struct. A scalar array would give `half3` size six
and alignment two, so it would also be wrong. Two native `half2` chunks give size eight and
alignment four exactly. Type lowering emits `[2 x <2 x half>]`; a load extracts each chunk, then
its ordered lanes, ignores the padded fourth lane for `half3`, and constructs the semantic three-
or four-lane vector. The producer is the selected CUDA vector layout rule, not fixture syntax.

The self-review inventory contains four retained changes. The widened compact-vector classifier
survives because both CUDA layout and the two differential fixtures prove its exact selected
families. Recursive fixed-array storage survives because the aligned workload fails without it and
it removes a duplicate representation choice. Fixed-array compact-pointer recognition survives
because that pointer is the direct canonical child of the same storage array. Half chunk
reconstruction survives because generic provider operations recover the ordinary SSA type while
preserving all stored lanes.

One attempted change did not survive. Passing every retained parameter-group semantic layout graph
into the physical pointee validator regressed four old-correct workloads involving existential,
packed-constant, and interface-parameter legalization. Those producers intentionally preserve
semantic metadata that is not structurally isomorphic to the legalized pointee. A six-row revert
drill showed all four old workloads and both new cbuffers correct after removing that mandatory
walk. The final code instead uses the canonical CUDA layout query over the actual lowered type,
which owns this physical representation and requires no compatibility fallback.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy references. Healthy correctness
advances from 407/407/407 to 409/409/409 O0/O3/both, with exactly the two cbuffer gains and zero
old-correct regression. All-row direct totals become 423 correct, seven runtime mismatches, and 22
preflight failures in each mode. Discovery remains exactly 82 workloads/72 healthy references and
69/69/69, with no changed row; `type-legalize-bug-1` retains its independent `ParameterBlock<B>`
conventional-global-field blocker.

The selected regression prefix passes 433/433 and the permanent `nvvm` category passes 66/66.
Both promoted workloads compile and assemble through CUDA 12.9 in all five measured configurations:
native NVRTC, direct O0 SM70, and direct O3 SM70/SM80/SM90. At SM70, aligned direct O3 PTX is 1,619
bytes versus 12,548 bytes from NVRTC; unaligned direct O3 PTX is 4,596 bytes versus 17,466 bytes.
Median standalone compile times remain exploratory rather than controlled benchmarks.

No fixture-name check, source reconstruction, compatibility fallback, arbitrary operand-graph
search, downstream malformed-IR patch, serialized-text rewrite, provider callback, or provider ABI
revision remains.
