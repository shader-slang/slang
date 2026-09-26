# FP8 scalar contract

Slice243 originally qualified a research representation and CUDA12.9 runtime conversions on SM80.
Slices244 and249 implemented finite producer repair and scalar transport. Slice260 adds scalar
widening to Float32 under provider ABI42; narrowing and storage remain separate contracts.
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

On the historical accepted242 compiler, literalBits is0x7e (448), while dynamicBits is0x78 (256).
SCCP calls `IRBuilder::getFloatValue`, which calls `FloatToFloatE4M3` and then
`FloatE4M3ToFloat` in `slang-math.h`. Before slice244, the narrowing helper clamps all exponent15 normal values to448.
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

## Shared finite conversion repair

Slice244 repairs the existing four `slang-math.h` helpers at the producer boundary. E4M3 normal
exponent15 now covers256 through448. Narrowing handles subnormals on the2^-9/2^-16 grids using
nearest/even rounding; the carry into the minimum normal is intentional. Widening uses minimum
normal2^-6/2^-14 times the fraction divided by8/4, so each finite byte widens exactly. Signed zero
is preserved. All256 encodings per format and every finite representable value/midpoint with its
Float32 neighbors are covered by independently enumerated values, including both signs.

This does not harmonize overflow with CUDA SATFINITE. E4M3 inputs strictly above448 still become
signed NaN, even if rounding to448 would have been possible. E5M2 keeps its rounded overflow
boundary61440, infinities and existing NaN mapping. Out-of-range policy needs a separate decision.
The regression fixture performs `asuint(float(FloatE4M3(256.0f)))` and corresponding finite/boundary
casts. FP8 is eliminated before backend preflight; only ordinary Float/UInt values reach NVVM.
Consequently this producer correctness change admits no FP8 backend operation or storage role.
See [report244](../../issue-nvvm-backend/report.slice-244-fp8-finite-producers.md).

## Boundaries after producer repair (historical slice244)

Slice244 repairs and exhaustively tests the finite/subnormal shared producers, preserving the
already-tested overflow policy. Its full checkpoint is accepted. That repair qualified reconsidering
scalar transport/literals/bitcasts and Float32 casts at their owning backend boundary.
Do not promote this research by moving a preflight diagnostic alone.

`dynamic-dispatch-substandard-float` remains a separate aggregate/dynamic-object problem. Its
`createDynamicObject<IFoo>` and registered conformances require any-value marshalling helpers
returning `A`, containing both FP8 fields, plus `B` containing BF16vector2. Scalar qualification does
not establish those storage/aggregate contracts. Integer/Half/double constructors, arithmetic,
vectors, external helper ABI and material runtime remain excluded.

## Scalar transport implementation (slice249)

Provider ABI41 adds distinct E4M3 and E5M2 semantic kinds, both represented by scalar i8. Admission
is confined to SSA registers and internal by-value helper parameters/results. Existing same-width
bit reinterpretation admits signed/unsigned8 pairs and E4M3/E5M2 cross-format pairs; it preserves
bits, not numeric values. Select requires the same semantic format in both arms and the result.
Generic branch/phi and internal helper emission preserve the physical byte.

Canonical finite IRFloatLit values use the shared producer's format helper to recover exact bytes.
Nonfinite FP8 literals remain rejected before provider discovery: this keeps the unharmonized
shared overflow policy visible. Transported nonfinite bit encodings are fully supported because
transport introduces no numerical interpretation. General runtime conversions, arithmetic, FP8
vectors/storage/pointers/aggregates/resources and external helper ABI remain separate contracts.
Recursive numeric/copyable/helper-storage classifiers are unchanged.

The registered scalar transport fixture covers all256 encodings per format with both branch choices,
selection, unsigned/signed bitcasts and12 finite constants. Supplemental raw replay covers both
cross-format directions. Final acceptance evidence is owned by runtime-validation.slice-249.json.

## Scalar Float32 widening (slice260)

Consider a dynamic byte loaded from a UInt buffer, bit-cast to `FloatE4M3`, passed through an internal
helper, and converted with `float(value)`. Ordinary lowering produces canonical `FloatE4M3Type`
and `FloatCast` to Float. `_getNVVMSemanticType` preserves the format; the shared semantic catalog
admits exactly scalar E4M3/E5M2 input and Float32 output for FLOAT_CONVERT. Provider ABI42 negotiates
this additional operation, without changing descriptor layouts, operation IDs or physical i8 transport.

`_emitFloat8Widen` owns the physical conversion. It extracts sign, exponent and fraction, rebiases
normal values into Float32 bits, and converts subnormal fractions using exact multiplication by
2^-9 or2^-16. Applying sign afterward preserves negative zero. E4M3 magnitude126 remains finite448;
magnitude127 is NaN. E5M2 exponent31 represents signed infinity with zero fraction and NaN otherwise.
Finite values, signed zeros and infinities are exact; NaN payload and sign are unspecified. All
constant shifts and speculative select operands are defined throughout the byte domain, and the
recipe requires no native FP8 instruction on SM80.

The source fixture covers every256 encoding per format in NVRTC O3 and NVVM O0/O3. Supplemental
raw-output controls exercise both selection flags through dynamic helpers, checking NaN classification
before comparing the whole buffer, including byte echoes, guards and untouched fields. Expectations
come from rational enumeration independently of the provider's bit-rebias recipe. Provider tests also
reject malformed descriptors and adjacent unqualified conversions. See
[report260](../../issue-nvvm-backend/report.slice-260-fp8-widening.md) for acceptance evidence.

This does not admit Float32-to-FP8 narrowing, integer/Half/double conversions, arithmetic, storage,
vectors, records, resources or external helper ABI. Shared constant-folding overflow policy remains
separate. The dynamic-object workload still rejects record result A; scalar widening is not proof
of aggregate or any-value support.
