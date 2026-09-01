# Slice 170: Resource-bearing aggregate storage layout

## Motivation

The discovery workload `buffer-type-splitting.slang` declares an array whose elements contain two
raw-buffer views:

```slang
struct S
{
    RWByteAddressBuffer a;
    RWByteAddressBuffer b;
};
S s[2];
```

Global-parameter collection retains an exact 64-byte array layout: each `S` is 32 bytes and each
raw view is the selected LLVM `{data, count}` pair at offsets zero and 16. Direct NVVM nevertheless
rejected the array because the context-free CUDA layout query cannot ask for offsets inside a
struct with opaque resource fields.

## Proposed solution

Use the target layout already retained on the synthesized conventional-global field as the
canonical ABI contract. Walk that layout recursively by semantic field key and prove its byte
offsets, array strides, sizes, and alignments against the provider representation. Keep the older
context-free query for boundaries where retained target metadata is unavailable. Recognize the
canonical collected-global producer independently from field support so an unsupported sibling is
diagnosed at its own exact type rather than erasing provenance from supported field addresses.

## Change summary

- Conventional-global aggregate arrays pass their retained `IRTypeLayout` into the existing
  provider-layout proof.
- Struct fields are matched by `IRStructKey`; arrays propagate their element layout and verify the
  retained element stride. Complete size and alignment must agree at every aggregate level.
- Collected-global recognition now checks only the producer-owned global/constant-buffer/
  synthesized-struct shape. Module validation separately rejects the first unsupported field with
  an exact typed diagnostic.
- `buffer-type-splitting.slang` gains permanent direct O0/O3 lanes. The discovery classifier owns
  the new role-based conventional-global-field diagnostic.
- Frozen and discovery census/Pareto artifacts, one measurement manifest, the design, capability
  ledger, plan, and this report retain the evidence. Provider ABI revision 32 is unchanged.

## Concepts and vocabulary

**Retained canonical layout** is the `IRTypeLayout` graph produced by target layout and preserved
through global-parameter collection. Unlike a later context-free query, it records the exact
layout chosen for opaque resource fields in their containing aggregate.

**Conventional-global provenance** is the identity of the synthesized constant-buffer-backed
`GlobalParams` block. Recognizing that producer and deciding whether every field has an executable
representation are separate questions.

## Process report

The first input-shape audit traced `buffer-type-splitting.slang` through
`collectGlobalUniformParameters`. The producer creates a synthesized `GlobalParams` field keyed by
the original `s` parameter. Its `IRArrayTypeLayout` records size 64, alignment eight, element stride
32, and an `IRStructTypeLayout` for `S`. The nested field-layout attributes identify `a` and `b` by
their actual `IRStructKey` values and record Uniform offsets zero and 16. These are canonical target
facts, not an alternative spelling invented by direct NVVM.

The existing provider type representation is already identical: each raw view is a 16-byte,
eight-aligned `{data, count}` aggregate; `S` is therefore 32 bytes and `S[2]` is 64 bytes. The failed
`getOffset(CUDA, field)` call was the wrong source of truth because it recomputed a struct layout
without the retained resource context. `_getNVVMAggregateStorageLayout` now accepts optional layout
metadata, recursively finds struct entries by key, propagates array element layouts, and compares
every provider offset, stride, size, and alignment. Missing, non-finite, wrongly typed, or
inconsistent metadata still fails before provider mutation. No packed type or padding field is
synthesized downstream.

The two adjacent probes exposed a separate representation issue. Both initially failed while
addressing a supported output buffer because `_getNVVMConventionalGlobalParams` required every
sibling field to be supported before it would recognize the canonical collected block. Recognition
now checks only the exact producer shape; `validateNVVMSupportedIR` performs the field-support
check explicitly. The frozen unaligned-cbuffer probe therefore advances to its actual parameter-
group pointer shape, while discovery `type-legalize-bug-1` now reports
`conventional global field: ParameterBlock<B>`. Neither parameter-group shape was admitted in this
slice.

The self-review inventory contains three retained helpers/branches. The finite byte-layout reader
survives because it merely reads canonical metadata and rejects non-finite values. The keyed field
lookup survives because layout fields are semantic key/value entries and must not be matched by
position. The optional metadata path survives because the real discovery workload fails without
it and exact O0/O3 differential execution proves this validation layer owns the physical ABI
check. The provisional homogeneous-raw-buffer offset guess and the unproven relaxation of
context-free root alignment were both removed before final validation.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy references at 407/407/407 O0/O3/
both, with no changed row and no old-correct regression. Its all-row direct totals remain 421
correct, eight runtime mismatches, and 23 preflight failures in each mode. Discovery remains
exactly 82 workloads and 72 healthy references and advances from 68/68/68 to 69/69/69, with
`buffer-type-splitting` as the only newly correct row. All-row discovery totals become 69 correct,
seven infrastructure failures, five preflight failures, and one runtime mismatch per direct mode.

The selected regression prefix passes 433/433. After promotion, the permanent `nvvm` category
passes 62/62. The new workload's exploratory three-repetition measurement reports 1,271-byte
direct O3 PTX versus 10,182-byte NVRTC PTX; direct O3 PTX assembles with CUDA 12.9 for SM70, SM80,
and SM90. Median standalone compile times were 241.1 ms for direct O3 SM70 and 351.8 ms for NVRTC,
but remain non-controlled exploratory measurements.

No fixture-name check, syntax reconstruction, compatibility fallback, arbitrary operand-graph
search, downstream malformed-IR patch, serialized-text rewrite, provider callback, or provider ABI
revision remains.
