# Slice 135: Common scalar math semantics

## Result

Slice 135 turns the largest coherent part of the post-Slice-134 ordinary-intrinsic cluster into
one reusable typed representation. The measured family contains 34 workloads whose first direct
NVVM blocker was scalar absolute value, ordinary Float32/Float64 math, `isnan`, or `sign`.
Twenty-seven now compare correctly with native CUDA at both O0 and O3. Six reach a later canonical
blocker, and one compiles but exposes a runtime mismatch. No previously correct workload regresses.

The LLVM provider ABI advances from revision 26 to 27 only to add missing operation IDs. The broad
generic value-operation query/emit callback remains unchanged.

## Canonical producer and ownership

CUDA-prelude target specialization produces each admitted shape as a final linked one-block
`IRFunc` containing only one `IRGenericAsm` terminator. Consider `tan(double(i))`: the linked helper
body contains `$P_tan($0)`, while the specialized function type is `double(double)`. The assembly
selects tangent and the function type supplies the exact result, operand type, and arity.

`_resolveNVVMGenericAsmValueOperation` validates that producer shape, looks up the exact spelling
in one operation table, derives semantic types from the linked signature, and calls
`resolveValueOperationFamily`. The same resolver answers provider capability queries and owns
libdevice demand before module creation. Fixture paths, source function names, and syntax never
enter the decision.

The accepted contracts are:

- same-type selected signed-scalar integer `abs`;
- same-type scalar Half `abs`;
- same-type scalar Float32/Float64 unary math (`abs`, `acos`, `asin`, `atan`, `ceil`, `exp`,
  `exp2`, `floor`, `frac`, `log`, `log2`, `log10`, `round`, `rsqrt`, `sqrt`, `tan`, and `trunc`);
- same-type scalar Float32/Float64 binary math (`atan2`, `fmod`, and `pow`);
- scalar Float32/Float64 `isnan` returning Bool; and
- scalar Float32/Float64 `sign` returning Int32.

Vector descriptors, Half transcendental/classification descriptors, unsigned-integer absolute
value, wrong results, and malformed arity remain unsupported. A focused custom vector-tangent
helper retains deterministic E52017 before provider discovery.

## Provider representation

The isolated LLVM 14 provider maps the selected scalar math descriptors to exact libdevice symbols
for each Float32/Float64 width. `sqrt` remains a typed LLVM intrinsic, and the strict NVVM IR 2.0
writer now validates both Float32 and Float64 square-root declarations before applying only its
established LLVM-14-to-LLVM-7 attribute compatibility handling.

The operations without a standalone libdevice contract are constructed from typed LLVM values:

- `frac(x)` calls exact `floor(x)` and emits `x - floor(x)`;
- `isnan(x)` emits unordered `x, x` comparison;
- `sign(x)` emits ordered positive and negative comparisons, extends them to Int32, and subtracts;
- signed integer `abs(x)` selects `-x` when `x < 0`, with ordinary wrapping integer negation; and
- Half `abs(x)` clears the sign bit through exact Half/I16 bitcasts.

These are provider-side constructions of already classified canonical values, not downstream
repairs. They use existing generic builder operations internally and require no semantic text
rewriting.

## Focused 34-workload outcome

The following 27 workloads are correct at both optimization levels and receive explicit O0/O3
direct lanes:

- `cross-compile/simple-cross-compile.slang`, `hlsl-intrinsic/matrix-int.slang`, and
  `language-feature/dynamic-dispatch/layout-array-of-structs.slang`;
- `hlsl-intrinsic/scalar-int.slang`, `vector-int.slang`,
  `vector-int-runtime-index.slang`, and `vector-float.slang`; and
- the scalar-Double fixtures for `abs`, `acos`, `asin`, `atan`, `atan2`, `ceil`, `exp`, `exp2`,
  `floor`, `frac`, `ldexp`, `log`, `log2`, `log10`, `pow`, `rsqrt`, `sign`, `sqrt`, `tan`, and
  `trunc`.

Six workloads advance beyond the selected family but remain failures:

| Workload | Next canonical blocker |
| --- | --- |
| `bugs/gh-8185.slang` | ordinary `shr` |
| `hlsl-intrinsic/classify-float.slang` | `$P_isfinite($0) : bool(float)` |
| `hlsl-intrinsic/packed/pack-unpack-float.slang` | opaque-Half float packing |
| `hlsl-intrinsic/scalar-double.slang` | Float64 `sincos` output parameters |
| `hlsl-intrinsic/scalar-float.slang` | Float32 `sincos` output parameters |
| `hlsl-intrinsic/scalar-half.slang` | `$P_isnan($0) : bool(half)` |

`hlsl-intrinsic/matrix-float.slang` compiles at O0 and O3 but mismatches the fourth output
(`1094.042358` expected, `1044.611328` actual). Its first scalar-math blocker is gone; separately
passing scalar and vector math workloads prevent treating this combined matrix-runtime result as
permission for another semantic widening. It remains unpromoted and measured as `runtime-other`.

## Fixed-denominator coverage

The denominator remains 452 eligible CUDA workloads from 448 sources: 430 MVP and 22 extension
lanes. Native CUDA has 449 correct references and three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Infrastructure |
| --- | ---: | ---: | ---: | ---: | ---: |
| NVRTC O3 | 449 | 0 | 0 | 0 | 3 |
| Direct O0 | 264 | 8 | 175 | 5 | 0 |
| Direct O3 | 260 | 16 | 175 | 1 | 0 |

Successful direct compilation/runtime launch is therefore 272/452 at O0 and 276/452 at O3.
Compared with Slice 134, both modes gain exactly 27 correct workloads and lose no old-correct
workload. Among the 427 MVP rows with a healthy native reference, O0 correctness is 263 (61.6%),
O3 correctness is 258 (60.4%), and 255 (59.7%) are correct in both.

The leading remaining MVP failure clusters are:

| Root-cause cluster | O0 | O3 |
| --- | ---: | ---: |
| Wave/reconvergence generic-asm semantics | 31 | 31 |
| Helper ABI type contracts | 28 | 28 |
| Aggregate/pointer/layout transport | 23 | 23 |
| Ordinary intrinsic generic-asm semantics | 18 | 18 |
| Ordinary numeric/bit operations | 17 | 17 |
| Residual target marker/undefined value | 9 | 9 |
| Atomic/wave operations | 8 | 8 |
| O3 narrow-integer runtime correctness | 0 | 8 |

The ordinary-intrinsic cluster falls from 47 to 18. The committed census table and cluster JSON
retain every workload's first shape, producer, diagnostic, phase, and coverage tier.

## Representative workload gates

All three release-gate workloads remain differentially correct in native CUDA, direct O0, and
direct O3. Median standalone compile time and PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | --- | --- | --- |
| Resource aggregate/helper | 385.8 ms / 8,889 B | 262.0 ms / 6,102 B | 269.1 ms / 919 B |
| Parameter-block layout | 399.0 ms / 8,839 B | 251.5 ms / 917 B | 250.5 ms / 793 B |
| Shared control/barriers | 373.9 ms / 9,190 B | 242.7 ms / 1,940 B | 249.4 ms / 1,404 B |

Each direct O3 module assembles with CUDA 12.9 `ptxas` for SM70, SM80, and SM90. Runtime comparison
uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 runtime workers remain
infrastructure gaps.

## Validation

- Release host and isolated LLVM 14 provider builds succeed with exact ABI 27 negotiation.
- The real provider constructs every operation ID, rejects adjacent vector/Half/unsigned shapes,
  and serializes both LLVM 14 and strict NVVM IR 2.0 forms.
- Fake-provider compilation observes signed/Half absolute value, unary/binary libdevice math,
  Boolean classification, Int32 sign, and lazy libdevice demand before provider mutation.
- The focused family reports 27 correct, six preflight, and one runtime mismatch in both direct
  modes, against 34/34 correct native references.
- The selected direct-NVVM regression prefix passes 404/404; this remains a regression score, not
  the coverage denominator.
- All 81 promoted-fixture CUDA lanes pass: 27 native references plus 54 new direct O0/O3 lanes.
- The full 452-row census reports the exact +27/+27 delta and zero old-correct regression.
- Representative direct O3 PTX assembles for SM70, SM80, and SM90.

The first broad promoted-fixture run also exercised unrelated WebGPU directives and reproduced four
local Dawn failures; rerunning with `-api-only -api cuda` passes 81/81 and isolates the intended
CUDA validation.
