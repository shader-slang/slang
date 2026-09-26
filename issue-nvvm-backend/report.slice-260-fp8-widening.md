# Slice260: scalar FP8-to-Float32 widening

Status: accepted full checkpoint on 2026-09-26, provider ABI42. The development loop stops after
this slice at the maintainer's request. Fresh
agent delegation is unavailable at the thread limit; one local writer uses separate source, oracle
and mechanical acceptance audits. No fresh independent-agent review is claimed.

## Motivation

Consider this complete dynamic conversion:

```slang
StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputBuffer;

[noinline]
float expand(FloatE4M3 value)
{
    return float(value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let value = bit_cast<FloatE4M3>(uint8_t(inputs[0]));
    outputBuffer[0] = asuint(expand(value));
}
```

Input byte1 represents2^-9, so the output must be Float32 bits0x3b000000. The same byte in FloatE5M2
represents2^-16 and must produce0x37800000. Input0x80 must preserve Float32 negative zero. E4M3
byte0x7e is finite448, while E5M2 byte0x7c is positive infinity. The formats share physical i8
transport after 249, but this does not give that byte the meaning of an integer or IEEE Half.

The new regression freezes all 256 encodings per format before production changes. Its512 expected
observations use exact Float32 bits for finite values, signed zero and infinity, and an explicit
classification marker only for NaNs. On accepted259, NVRTC matches every expected value; both direct
modes stop at E52017 floatCast. Separate E5M2 traces retain its canonical FloatCast and the same
preflight rejection at O0/O3. The before runtime smoke gate also passes.

Research243 qualified exact runtime widening with an independent rational oracle;244 repaired shared
finite/subnormal producers and 249 admitted scalar bit transport. This slice promotes just widening.
The frozen dynamic-object workload also needs record A/B and any-value marshalling, so its first
record-result rejection remains a separate obligation. Material257/258 was reconsidered: the getter
experiment missed its thresholds and was discarded, and material runtime inputs remain absent.
Proceed under the documented capability cadence exception without a speculative optimization.

## Proposed solution

Keep canonical FloatE4M3 and FloatE5M2 semantic identity and the existing physical i8 register/helper
representation. Add exactly scalar FP8→Float32 FloatCast to the shared semantic catalog. A provider
recipe decodes the encoding: normal finite values construct exact Float32 bits; subnormals use exact
small-integer times power-of-two arithmetic; format-specific nonfinite encodings retain infinity
or NaN classification. The sign is applied to finite/subnormal bits afterward, including zero.

Provider ABI42 negotiates the added semantic contract with the compiler. No type role, storage ABI,
API struct layout, external helper ABI, reverse conversion or shared literal policy is changed.

## Change summary

- `slang-nvvm-semantic-catalog.h` reuses exact scalar FP8 descriptor classification and adds the
  Float8Widen family for FloatConvert to kFloat32. The same catalog governs host and provider.
- `slang-nvvm-ir-builder-api.h` advances the negotiated revision41→42.
- `slang-llvm-nvvm.cpp` adds one named widening recipe and its family dispatch. Existing format
  descriptors, physical type validation and generic IR construction remain authoritative.
- `unit-test-nvvm-builder.cpp` updates the existing descriptor matrix's one newly supported cell
  and adds real-provider positive/malformed/neighbor coverage for both formats. Rejected operations
  return no value, and the completed function must serialize/verify in both supported text dialects.
- `unit-test-nvvm-emitter.cpp` moves the former widening rejection into positive GPU coverage and
  removes the obsolete widening-negative subcase. Other existing
  narrowing, integer construction, storage/record/vector/nonfinite-literal/export exclusions remain.
- `nvvm-fp8-widening.slang` and one discovery row add the independently expected512-word contract.
  Plan/report, design, compact results and STATUS record final acceptance.

## Concepts and vocabulary

**Semantic descriptor** identifies the FP8 format independently of physical i8. **Widening** preserves
an already-representable FP8 numerical value in Float32 without rounding; it is distinct from byte
transport and narrowing. **NaN classification** promises a NaN result without a particular sign or
payload. **SATFINITE** is CUDA's separate narrowing overflow policy; this slice never selects it.

## Process report

The input shape is correct. Core FP8 declarations and ordinary source lowering produce canonical
FloatE4M3Type/FloatE5M2Type and FloatCast to Float. The before E5M2 IR explicitly retains that cast.
`_getNVVMSemanticType` already supplies the distinct descriptor, and `_getNVVMValueOperation` already
maps FloatCast to FLOAT_CONVERT. The shared resolveValueOperationFamily previously rejected this
valid pair. No checked expression, type or literal producer needs reconstruction or repair.

The catalog reuses `isFloat8Operand`, which requires one exact scalar8-bit format descriptor, and
requires the exact scalar Float32 result and FloatConvert opcode. This adds no other width, lane,
format or cast direction. The provider resolves through that same catalog before the existing
_emitValueOperationFamily validates module/insertion point, result type, operand count, physical
operand type and availability. `_emitFloat8Widen` therefore receives a qualified i8 and Float32 type.
No separate emitter admission or duplicate semantic classifier is necessary.

Consider byte0x81 in E4M3. The recipe extracts a negative sign and magnitude1, then exponent0 and
fraction1. The subnormal branch computes1*2^-9 exactly. Its Float32 bits are ORed with the sign,
producing0xbb000000. For byte0x80 the fraction is zero, and the same final sign application produces
0x80000000. For normal values, adding127-bias to the encoded exponent and moving the fraction to
Float32's mantissa positions is exact. E4M3's magnitude126 remains finite; magnitude127 becomes a
quiet NaN. E5M2 magnitudes124..127 select signed infinity for zero fraction and NaN otherwise.

The helper/fallback inventory is:

- Float8Widen catalog family and dispatch: retained after focused, exhaustive and full checkpoint gates. Removing admission restores the
  captured before floatCast rejection. One shared table owns legality; ordinary IEEE float and
  integer conversion classifications remain unchanged.
- `_emitFloat8Widen`: retained after focused, exhaustive and full checkpoint gates. This is the physical conversion boundary for valid
  canonical FP8 values, not a workaround for malformed producer output. LLVM FPExt cannot interpret
  physical i8 as either FP8 format. The regression and all-encoding raw-output controls test this
  exact boundary through dynamic internal helpers.
- Format constants and the E5M2 special-value branch: retained after focused, exhaustive and full checkpoint gates. These express actual
  format semantics, including the E4M3 exponent15 finite range and E5M2 infinity distinction. Every
  byte and both signs are covered. The generic research template's unreachable E4 infinity branch
  is not copied into production.
- ABI revision: retained after focused, exhaustive and full checkpoint gates. Matching host/provider artifacts negotiate new supported
  semantics. Descriptor layouts and operation IDs stay unchanged; no second semantic representation
  or generic storage admission is introduced.

Every speculative select operand is defined. The zero-extended byte is0..255, magnitude0..127 and
fraction0..7 or0..3. Rebiased normal exponents are120..135 or112..143, including harmless unselected
zero-exponent values. Fixed shifts are all less than32. Subnormal multiplication is exact and far
above Float32 underflow. There is no division, variable shift, integer overflow, poison construction
or unsupported native FP8 instruction. The recipe works with the existing SM80 target.

Shared constant folding remains independent. Finite producer244 behavior is preserved; nonfinite
FP8 literals still reject under249's policy. Transported NaN bytes are valid conversion inputs, so
this widening recipe returns NaN classification without interpreting an overflowed source literal
or silently applying CUDA saturation. Reverse Float32 conversion and its policy stay excluded.

The source fixture normalizes only actual NaN observations to 0x7fc00000 for deterministic FileCheck;
finite, zero and infinity mismatches remain visible. Supplemental controls retain raw Float32 output
bits and check the classification mask directly at the independently enumerated NaN indices before
comparison. Both branch flags cover every byte of each format; byte echo and untouched packet,
header and sentinel fields require exact preservation. The expected buffers use rational values,
not the provider's bit-rebias recipe. Six launches pass:24768 words/99072 bytes, with 48 raw NaN classification observations.
The source fixture additionally checks 1536 words across all 3 modes. PTX is assembled for SM80;
execution uses CUDA Driver PTX JIT, not the separately retained cubins.

The first GPU fixture run passes all 3 modes, and both provider contract units pass. A proposed
replacement source negative used FP8-to-UInt, which is ambiguous in the frontend and never reaches
NVVM. The invalid new replacement was removed; the obsolete widening rejection is now covered by the
all-encoding positive fixture, while every other existing source negative remains. The failed
unit attempt and exact source snapshots are retained. Production and frozen fixture/oracle bytes are unchanged.

The full checkpoint preserves every 1710 prior five-field corpus outcome and adds3 correct cells.
Frozen1356 has 1347 correct and 9 unresolved; discovery357 has 327 correct and 30 unresolved. Combined
1713 cells/1674 correct/39 unresolved retain18 resolved histories, with no missing or duplicate cells.
All 1063 prior unit IDs retain their outcomes; one new unit gives1051 passed/13 ignored. Semantic
regressions retain all 1129 identities,1052 passed/77 ignored. Runtime4, focused6, toolkit18, runner
contracts6 and all 6 material compile/assembly cells pass. Material PTX and cubins are byte-identical
to 259; there is no material runtime or performance claim.

The interruption left four full gates with recorded successful exits. The frozen runner had written
results but its wrapper exit was unavailable. That whole attempt is preserved under `interrupted-full-checkpoint`; only the uncertified frozen
gate was repeated before completing discovery/material. Source and
artifact identity remain equal across prototype2, every full gate and final capture. The small gates
are final-source evidence within this slice, not inherited passes from an earlier implementation.

The tested base is dbb9bfb9015618ebf730fe54fde5b9233658e475 plus this diff. Final identity covers 137
source snapshots,12 artifacts,567 runtime inputs and 18 unchanged submodule pins. All 566 prior input
hashes remain unchanged. Compiler SHA256 is `4b59d082df60a6862e684f63f099105228d0b820113aba456e83abc3661aca7d`; provider ABI42
SHA256 is `2e54768ba323ba86cb7767a0a4405c9d4212e36f67e62786fd812dd59532e9f1`.
See runtime-validation.slice-260.json for exact commands, hashes, per-cell outcomes and histories.

Raw evidence is under `build/nvvm-loop/slice-260-before` and `slice-260-after`, including the initial
failed unit attempt and interrupted corpus attempt. Completed plan/report and compact results are
committed with the implementation. The development loop stops after this local commit at the
maintainer's request; no next slice is selected and no push is authorized.

Evidence closure verifies 4,854 raw artifacts, 137 final source snapshots and 351 compact references.
