# Slice 178: Canonical scalar numeric truthiness

## Motivation

The frozen `language-feature/conversions/conversion-to-bool.slang` workload stopped at canonical
`castFloatToInt`. Its 65 deterministic results cover Bool, every ordinary signed and unsigned
integer width, Half, Float32, and Float64, including signed zero and infinity. Integer-to-Bool was
already represented as a comparison with zero, but the same semantic from floating values fell
through to the ordinary floating-to-integer catalog family, whose result must be an integer.

## Proposed solution

Generalize the existing scalar integer-truthiness recipe to scalar numeric truthiness. Prove the
canonical operation and complete types, construct an exact same-typed zero, and invoke typed
`NOT_EQUAL`. Keep actual floating-to-integer conversion, vectors, BFloat16, and FP8 outside this
bounded rule. Reuse provider ABI revision 33 unchanged.

## Change summary

- The shared resolver accepts Bool-result `IntCast` from selected integers and Bool-result
  `CastFloatToInt` from selected floats.
- Requirement collection and emission consume the same comparison recipe.
- Emission selects an integer zero or a typed floating zero without introducing a provider API.
- Focused fake-provider coverage proves Half, Float32, and Float64 unordered-not-equal descriptors
  and zero bit patterns.
- `conversion-to-bool.slang` gains permanent direct O0/O3 differential lanes.
- Frozen/discovery artifacts, measurement manifest, plan, design, and capability ledger retain the
  complete validation evidence.

## Concepts and vocabulary

**Numeric truthiness** is the checked conversion whose result is false exactly for numeric zero and
true otherwise. For floating values, unordered comparison also makes NaN true.

**Typed zero** is a provider constant with the exact source scalar type and an all-zero bit pattern;
both positive and negative floating zero compare equal to it.

## Process report

Consider `bool(value)` in the promoted fixture. Checked IR lowering uses `kIROp_IntCast` for integer
sources and `kIROp_CastFloatToInt` for Half, Float32, and Float64 sources. In each relevant case the
instruction has one scalar numeric operand and scalar Bool result. This is canonical producer
output: the operation name identifies the broad numeric cast family, while the complete result type
identifies truthiness. There is no malformed IR to repair upstream.

`_resolveNVVMNumericTruthiness` now proves exactly that shape. It accepts selected scalar integers
only with `IntCast`, selected scalar floats only with `CastFloatToInt`, and rejects every non-Bool
result. It creates the existing typed `NOT_EQUAL` recipe. Consequently ordinary Float-to-Int
instructions continue through `_resolveNVVMValueOperation` and `FLOAT_TO_INTEGER`; the change does
not widen or reinterpret that catalog family.

`_emitNVVMNumericTruthiness` lowers the source once. Integer sources retain the established
integer-zero path. Floating sources lower their exact type and request the existing
`getFloatingPointConstant` callback with a zero bit pattern, then emit the common comparison step.
The provider's FloatCompare family maps `NOT_EQUAL` to LLVM `fcmp une`. This preserves false for
`+0.0` and `-0.0`, true for finite nonzero values and infinities, and true for NaN. The promoted
fixture proves every specified runtime result against native CUDA in O0 and O3.

The self-review inventory contains one generalized resolver, one renamed requirement helper, one
emission branch for typed floating zero, and one focused test source. The resolver and zero branch
survive because removing either restores the measured `castFloatToInt` preflight stop. The helper
rename keeps one source of truth rather than adding a parallel float path. The test derives Half
and Float64 locally from an established Float32 entry parameter so it isolates recipe ownership;
it proves all three descriptors and zero widths. No code checks a fixture name, reconstructs
syntax, walks arbitrary operands, weakens diagnostics, adds a fallback, or revises the ABI.

Frozen corpus v1 stays exactly 452 workloads/427 healthy references and advances from
415/415/415 to 416/416/416 O0/O3/both. `conversion-to-bool.slang#cuda-1` is the only gain; there
are no old-correct regressions. All-row direct totals are 430 correct, three runtime mismatches, and
19 preflight failures per mode. Discovery stays exactly 82 workloads/72 healthy references at
72/72/72 with no changed row. The selected prefix passes 435/435 and the permanent `nvvm` category
passes 86/86.

The representative gate assembles through CUDA 12.9 for native NVRTC, direct O0 SM70, and direct
O3 SM70/SM80/SM90. At SM70, standalone one-repetition measurements are 489.3 ms and 38,917 PTX
bytes native, 278.3 ms and 32,829 bytes direct O0, and 287.6 ms and 30,174 bytes direct O3. These
measurements remain exploratory.
