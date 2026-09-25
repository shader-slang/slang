# FP8 scalar research contract

Slice243 qualifies a research representation and CUDA12.9 runtime conversions on SM80. It adds no
production support. Accepted implementation/full checkpoint242 and providerABI40 remain unchanged.
See [report243](../../issue-nvvm-backend/report.slice-243-fp8.md) and its compact semantic evidence.

## Representation and ownership

`FloatE4M3` and `FloatE5M2` are distinct canonical `PackedFloatType` IR types declared in
`core.meta.slang`. Their Float32 constructors and widening constructors produce `FloatCast`;
integer constructors produce `CastIntToFloat`. CUDA emission names `__nv_fp8_e4m3` and
`__nv_fp8_e5m2`, enabling `cuda_fp8.h`. A future direct backend can use physical `i8` for each scalar
register and internal by-value helper argument/result, but must retain the format in semantic
planning/descriptors. Equal physical width does not permit conversion semantics to lose the format.
LLVM14 native float8 and SM89 FP8 instructions are unnecessary and unqualified here.

The transport prototype uses i8 parameters/results, a branch and phi, and UInt8 truncation/extension.
All256 encodings, including signed zeros and every NaN bit pattern, survive unchanged. Each encoding
also appears with both branch choices, selecting the original or complemented byte. Source
`bit_cast` lowering preserves the same-size scalar bitcast; it must not become numeric conversion.
This proof admits neither external CUDA helper interoperability nor pointer, local, aggregate,
resource, global, parameter-group or vector storage roles.

## CUDA runtime Float32 conversion

The installed CUDA12.9 `cuda_fp8.hpp` constructors use `__NV_SATFINITE` and round to nearest with ties
to even. For SM80, `__nv_cvt_float_to_fp8` uses the software path: canonicalize NaN, promote finite
Float32 exactly to double, then convert. E4M3 has bias7, three fraction bits, minimum subnormal2^-9,
maximum448 (byte0x7e), and no infinities; only absolute byte0x7f is NaN. E5M2 has bias15, two fraction
bits, minimum subnormal2^-16, maximum57344 (byte0x7b), infinity0x7c and NaNs0x7d–0x7f. Both signs are
supported. Narrowing finite overflow and infinities saturates to signed maximum finite. Signed zeros
are preserved. The portable research contract promises NaN classification, not sign/payload.

Widening every finite FP8 value is exact in Float32. E5M2 infinities stay infinite; NaNs stay NaN.
CUDA's software widening uses `__nv_cvt_fp8_to_halfraw`, then half-to-Float32. The prototype instead
extracts sign/exponent/fraction and constructs normal Float32 bits; for subnormals it multiplies the
exact small integer fraction by a power of two. This is exact, far from Float32 underflow. The
prototype's NaN bits differ from NVRTC, as allowed by the classification-only contract.

The raw narrowing recipe uses Float32 integer bits, not Float32→Half→FP8 (which can double round).
It handles NaN, saturation and tiny signed-zero results first. Remaining values have bounded shift
counts. Subnormals round the explicit significand on the minimum-subnormal grid; normals round it
to the destination fraction width. Remainder greater than half, or equal to half with an odd retained
LSB, increments the result. A format-specific exponent adjustment supplies the final encoding.
The exact retained `.ll` recipe and parent review are required evidence before any later promotion;
a new implementation must reproduce this contract with focused production tests and full checkpoint.

## Constant folding is a separate producer contract

Consider this example:

```slang
float input = asfloat(inputBits); // inputBits is loaded at runtime and encodes 256.0f.
uint literalBits = bit_cast<uint8_t>(FloatE4M3(256.0f));
uint dynamicBits = bit_cast<uint8_t>(FloatE4M3(input));
```

On the unchanged accepted compiler, literalBits is0x7e (448), while dynamicBits is0x78 (256).
SCCP calls `IRBuilder::getFloatValue`, which calls `FloatToFloatE4M3` and then
`FloatE4M3ToFloat` in `slang-math.h`. The narrowing helper clamps all exponent15 normal values to448.
It also lacks correct subnormal rounding; both widening helpers scale subnormals incorrectly.
Byte1 widens to Float32 bits0x37000000 for E4M3 and0x34800000 for E5M2 in these shared helpers,
instead of0x3b000000 and0x37800000. These are producer defects, not shapes for NVVM to repair.

Overflow must be treated separately: existing math unit tests explicitly require E4M3 overflow to
NaN and E5M2 overflow to infinity, whereas CUDA constructors saturate finite. Changing this shared
policy is not justified merely by matching CUDA. A folded E5M2 infinity can even yield saturated
byte0x7b through CUDA construction while a separately folded widening returns Float32 infinity.
Do not reconstruct or reinterpret literals in the backend to hide either distinction.

The frozen folding workload uses the ordinary exact value1.25, represented as E4M3 byte0x3a and
E5M2 byte0x3d. A later transport/literal/bitcast slice can remain narrowly bounded to existing
canonical values only after the producer issues are addressed or explicitly scoped. General runtime
casts require a separate semantic operation with the format and CUDA saturation policy intact.

## Boundaries and next action

Repair and exhaustively test demonstrably incorrect finite/subnormal shared producers first,
preserving the already-tested overflow policy unless a separate decision changes it. Then consider
scalar transport/literals/bitcasts and qualified Float32 casts at their owning backend boundary.
Do not promote this research by moving a preflight diagnostic alone.

`dynamic-dispatch-substandard-float` remains a separate aggregate/dynamic-object problem. Its
`createDynamicObject<IFoo>` and registered conformances require any-value marshalling helpers
returning `A`, containing both FP8 fields, plus `B` containing BF16vector2. Scalar qualification does
not establish those storage/aggregate contracts. Integer/Half/double constructors, arithmetic,
vectors, external helper ABI and material runtime remain excluded.
