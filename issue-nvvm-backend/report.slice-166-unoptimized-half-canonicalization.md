# Slice 166: O0-compatible Half vector construction

## Motivation

Five frozen-corpus workloads had passed direct NVVM O3 for many slices but failed at O0 inside
`nvvmCompileProgram` with only `Error: unsupported operation`. Consider the focused value kernel:

```slang
half first = half(left);
half second = half(integerValue);
half2 pair = half2(first, second);
half2 result = -(chooseHalf2(pair, converted, left > right) + half2(1.0h, 2.0h));
bool2 compared = result < pair;
```

Its final provider input contains native Half2 construction, helper parameters/results, arithmetic,
comparison, widening, integer conversion, and dynamic extraction. A typed Half surface workload
adds scalar and Half4 surface loads plus a scalar store. The old coarse census cluster did not say
which of these valid-looking LLVM shapes libNVVM O0 rejected.

## Proposed solution

Establish the external contract with reduced direct-libNVVM probes, then give the LLVM provider one
vector-construction representation. For ordinary element types, retain native `insertelement`. For
runtime Half vectors of two through four lanes, bitcast each scalar Half to i16, insert those exact
bits into `<N x i16>`, and bitcast the completed vector once to `<N x half>`.

Use this helper for every provider-owned construction producer: the generic builder callback,
scalar broadcast materialization, and vector surface-load reconstruction. Keep O0 as O0; do not run
an optimization pipeline, select libNVVM O3, rewrite assembly text, or widen compiler preflight.
The existing generic interface expresses the operation, so provider ABI revision 31 remains fixed.

## Change summary

- The isolated LLVM provider centralizes fixed-vector construction and uses exact i16 lane
  transport for Half vectors.
- Generic construction, broadcast, and surface-load producers reuse the same helper.
- Real-provider serialization checks require the i16 construction form and reject regression to
  native Half lane insertion.
- Five existing Half workloads gain permanent direct O0 lanes while retaining their O3 lanes.
- Separate frozen/discovery artifacts, a 30-gate measurement manifest, design record, capability
  ledger, completed plan, and this report retain the evidence.

## Concepts and vocabulary

**Runtime Half lane insertion** is LLVM `insertelement <N x half>` where at least one lane is a
nonconstant SSA value. CUDA 12.9 libNVVM verifies this shape but cannot compile it at O0.

**Bit transport** means changing only the LLVM type used to carry an already-defined bit pattern.
`bitcast half to i16` and the final vector bitcast preserve the exact 16-bit payload, including NaN
payloads and signed zero; they perform no numeric conversion.

**Mandatory representation legalization** is a bounded graph construction required by the
downstream consumer in every optimization mode. It differs from an optimization pass because it
does not simplify, fold, inline, or otherwise choose a faster form.

## Process report

The audit first captured Slang's intermediate NVVM assembly for `nvvm-half-values.slang` under O0
and O3. The files had identical SHA-256 hashes. Direct calls to CUDA 12.9 libNVVM verified the same
module in both modes, failed compilation at `-opt=0`, and succeeded at `-opt=3`. This disproved the
initial possibility that Slang emitted an O0-only malformed form and localized the distinction to
libNVVM's internal compilation pipeline.

Small self-contained modules then tested each operation family independently. O0 accepted Half2
add, comparison, vector fptrunc/fpext, vector fptosi, constant and dynamic extraction, scalar Half
load, bitcasts, and a Half2 helper parameter/result call. A module containing runtime
`insertelement <2 x half>` reproduced the exact generic failure. Equivalent two-, three-, and
four-lane modules that bitcast scalar lanes to i16, constructed `<N x i16>`, and bitcast the result
to `<N x half>` all verified and compiled at O0 and O3.

The exact input shape is canonical and intentionally allowed. Slang's vector constructor and
flattening producers supply ordered selected scalar Half values to the revision-31 generic
`emitVectorConstruct` callback. Typed surface lowering independently returns an ordered LLVM struct
of i16 physical lanes which `_emitSurfaceOperation` reconstructs as the selected semantic Half
vector. Scalar broadcast similarly needs an ordered vector of one repeated selected value. None of
these producers is malformed, and the compiler's typed semantic catalog is already the correct
source of truth. The consumer-specific representation belongs in the isolated LLVM provider.

The first implementation changed only `_emitVectorConstruct`. Four real workload pairs passed, but
`half-rw-texture-simple.slang` still failed at O0. Its preserved module exposed four native Half
insertions in the surface-load helper. This was not a new semantic feature; it was a second producer
of the same exact construction shape. The implementation was refactored into
`_createNVVMVectorConstruct`, and generic construction, broadcast, and surface reconstruction now
share it. Post-fix real modules contain `<2 x i16>` and `<4 x i16>` lane insertion followed by one
bitcast and contain no `<N x half>` insertion.

Several broader alternatives were rejected. Mapping O0 to libNVVM O3 would violate the requested
mode and maximal-debug contract. Adding LLVM optimization components or a pass pipeline was
unnecessary once the exact instruction was isolated and would make output changes hard to bound.
Scalarizing all Half arithmetic or changing the helper ABI would duplicate operations libNVVM O0
already supports. Text substitution would reconstruct typed syntax after serialization and bypass
LLVM verification. A compiler-side Half special case would put an LLVM 7 consumer quirk in the
wrong layer.

The retained helper is not a fallback: it is the single provider representation for an already-
validated fixed vector. Non-Half values follow the original native insertion path. Half values use
integer insertion unconditionally in both modes, so behavior does not depend on retrying a failed
compile or inspecting fixture names. Exact unit serialization proves no native Half insertion and
the expected i16 vector plus bitcast. Existing validation still rejects null, foreign, wrong-type,
wrong-count, and unavailable elements before mutation.

All five workload pairs pass their direct O0 and O3 runtime lanes (10/10), and the selected NVVM
prefix passes 433/433. Frozen corpus v1 retains exactly 452 workloads/427 healthy references and
advances from 396/400/396 to 400/400/400 O0/O3/both, with exactly four healthy gains and zero
old-correct loss. The fifth raw O0 gain is `half-vector-calc`, whose NVRTC lane remains an
infrastructure failure because CUDA 12.9's generated `__half4` has no `.xyz` member. Discovery
retains exactly 82/72 at 66/66/66 with no gain or loss.

The 30-gate measurement run produced 150 rows and 150 assembled cubins. At direct O0 SM70, the Half
value gate measured 260.2 ms and emitted 5,228-byte PTX; its direct O3 lane measured 265.1 ms and
1,696 bytes, versus 463.1 ms and 14,033 bytes through NVRTC O3. The Half surface gate measured
263.1 ms and 14,547-byte PTX at direct O0 and 273.8 ms and 1,709 bytes at direct O3, versus 472.7 ms
and 13,357 bytes through NVRTC. Direct O3 PTX assembled with CUDA 12.9 for SM70, SM80, and SM90.
These one-repetition measurements remain exploratory.

The final special-case inventory contains one new helper and one exact branch. Both survive:
`_createNVVMVectorConstruct` is the provider's single construction source of truth, and its Half
branch is proved by reduced consumer probes plus generic and surface real workloads. No compiler
fallback, optimization-mode remapping, provider callback, ABI revision, fixture-name check, syntax
reconstruction, textual manipulation, operand-graph search, or downstream malformed-IR patch was
introduced.
