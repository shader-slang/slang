# Slice 133: Canonical scalar minimum and maximum

## Result

Slice 133 turns the largest coherent part of the ordinary-intrinsic census into reusable typed
semantics. The pre-slice 66-workload `GenericAsm` cluster contains 48 exact first-failure pairs when
grouped by final assembly text and linked helper signature. Scalar minimum/maximum accounts for 17
first blockers, more than any other coherent operation family.

Eight existing workloads now compile and compare correctly at both O0 and O3. Nine more pass their
minimum/maximum blocker but stop at a later exact ordinary intrinsic, so they remain outside the
success numerator. No previously correct workload regresses.

| Mode | Correct | Runtime mismatch | Slang preflight | NVVM/provider failure | Infrastructure |
| --- | ---: | ---: | ---: | ---: | ---: |
| NVRTC O3 | 449 | 0 | 0 | 0 | 3 |
| Direct NVVM O0 | 226 | 7 | 214 | 5 | 0 |
| Direct NVVM O3 | 222 | 15 | 214 | 1 | 0 |

The denominator remains 452 workloads: 430 MVP and 22 extension. Among the 427 MVP workloads with
a healthy native reference, 225 (52.7%) compare correctly at O0, 220 (51.5%) compare correctly at
O3, and 217 (50.8%) compare correctly at both.

## Exact semantic census

The diagnostic now records `GenericAsm assembly=<text>, signature=<result>(<parameters>)` before
provider discovery. The original 66 O0 and O3 rows have identical first blockers:

| Operation spelling | Workloads | Signature variants |
| --- | ---: | --- |
| `max` | 15 | Int32 (6), Float32 (4), Float64 (4), UInt32 (1) |
| `min` | 2 | Int32 (1), Float64 (1) |
| `tan` | 5 | Float32 (3), Float64 (2) |
| `countbits` | 4 | UInt32, UInt64, UInt16, Int32 |
| `asuint` | 3 | Double plus two UInt32 output parameters |
| `abs` | 3 | Float16, Float32, Float64 |
| `asdouble` | 2 | two UInt32 parameters |
| `isnan` | 2 | Float32 |
| `exp2` | 2 | Float32, Float64 |
| `firstbithigh` | 2 | Int32, Int64 |
| `firstbitlow` | 2 | UInt32, Int64 |
| `reversebits` | 2 | UInt32, UInt64 |
| Other exact spellings | 22 | `log`, `log10`, `exp`, `ceil`, `frac`, `floor`, `trunc`, `sqrt`, `sincos`, `log2`, `pow`, `sign`, `rsqrt`, `frexp`, `acos`, `asin`, `atan`, `atan2`, and four exact Half conversions |

These counts cover all 66 rows and 48 exact assembly/signature pairs. Raw per-row inventories and
logs remain under ignored `build/nvvm-census/slice133-ordinary-inventory`.

## Canonical producer and representation

Consider the CUDA-prelude helper produced for `max(double, double)`. Intrinsic expansion and
`StmtLoweringVisitor::visitIntrinsicAsmStmt` create a helper whose final linked body is one
`IRGenericAsm` terminator with text `$P_max($0, $1)`. Specialization fixes its result and two
parameters to `Double`. `_validateNVVMFunction` sees this final canonical representation; it does
not recover a source intrinsic name or rebuild syntax.

`_resolveNVVMGenericAsmMinMax` accepts only that one-block, two-parameter, same-result-type shape.
It converts the final IR type to one `SlangNVVMValueTypeDesc` and asks
`resolveValueOperationFamily` for the complete typed descriptor. Selected scalar signed/unsigned
integers use the generic integer-binary family. Scalar Float32 and Float64 use the floating-binary
family and carry an explicit CUDA-device-library requirement.

Emission lowers the two canonical helper parameters and calls the existing generic
`emitValueOperation` callback. Integer minimum/maximum becomes a signed or unsigned LLVM compare
followed by `select`. Floating minimum/maximum must not use compare/select because that would
change NaN and signed-zero behavior; the provider emits exact `__nv_fminf`, `__nv_fmaxf`,
`__nv_fmin`, or `__nv_fmax` declarations and calls. The same libdevice helper now owns the existing
unary mappings and the new binary mappings, so operation-to-symbol selection has one provider
source of truth.

Forward-only builder ABI revision 25 adds `SLANG_NVVM_VALUE_OP_MIN` and
`SLANG_NVVM_VALUE_OP_MAX`. The typed query/emit callback, type algebra, and serialization interfaces
are unchanged. This is the smallest provider revision justified by the concrete canonical
operations; compiler-side classification and existing generic builder operations do all other
work.

## Workload transitions

The eight promoted workloads are:

- `hlsl-intrinsic/min-max-iarithmetic.slang#cuda-1`;
- both CUDA variants of `hlsl-intrinsic/packed/pack-unpack.slang`;
- `scalar-double-clamp`, `scalar-double-max`, `scalar-double-min`,
  `scalar-double-saturate`, and `scalar-double-smoothstep`.

Each fixture has direct O0 and O3 runtime-comparison directives. The nine non-success transitions
are retained as failures: `simple-cross-compile` reaches `fmod`; two unpack-float fixtures reach an
exact Half conversion; packed float reaches `round`; four signed-integer fixtures reach `abs`; and
the UInt32 scalar fixture reaches `countbits`. This is why 17 first blockers produce eight coverage
gains rather than an inflated 17-success claim.

## Post-slice Pareto

The leading remaining MVP failure clusters are identical at O0 and O3 before O3-only correctness
clusters:

| Root-cause cluster | Workloads | Cumulative failures covered |
| --- | ---: | ---: |
| Ordinary intrinsic `GenericAsm` semantics | 58 | 58 |
| Common wave/reconvergence semantics | 31 | 89 |
| Helper ABI type contract | 28 | 117 |
| Aggregate/pointer/layout transport | 23 | 140 |
| Ordinary numeric/bit operation | 16 | 156 |

The ordinary cluster remains the largest, but it is now semantically measured. Its next reusable
families include common unary/binary libdevice math, bit-count/scan/reverse operations, and exact
Half conversions. Slice 134 should use both the post-min/max first-blocker counts and the nine newly
exposed blockers rather than treating all 58 rows as one feature.

## Representative gates and measurements

The resource-aggregate/helper, parameter-block-layout, and shared-control/barrier gates remain
correct through NVRTC O3 and direct O0/O3. Direct O3 PTX for each assembles with CUDA 12.9.86
`ptxas` for SM70, SM80, and SM90. Runtime execution remains on the local SM120 RTX 5090; CUDA 13
and physical SM70/80/90 workers remain infrastructure gaps.

| Workload | Route | Compile median | PTX bytes | Census compile/load/run/compare |
| --- | --- | ---: | ---: | ---: |
| Resource aggregate/helper | NVRTC O3 | 385.2 ms | 8,889 | 4,019 ms |
|  | Direct O0 | 275.2 ms | 6,102 | 3,780 ms |
|  | Direct O3 | 271.8 ms | 919 | 3,960 ms |
| Parameter-block layout | NVRTC O3 | 376.5 ms | 8,839 | 3,955 ms |
|  | Direct O0 | 256.1 ms | 917 | 3,827 ms |
|  | Direct O3 | 260.4 ms | 793 | 3,679 ms |
| Shared control/barriers | NVRTC O3 | 373.2 ms | 9,190 | 4,776 ms |
|  | Direct O0 | 251.9 ms | 1,940 | 4,127 ms |
|  | Direct O3 | 258.1 ms | 1,404 | 4,012 ms |

These are startup-inclusive compiler and end-to-end harness measurements, not kernel-only runtime
benchmarks. Production claims still require a CUDA toolkit/GPU CI matrix and kernel-only runtime
measurements.

## Validation

- Release host and isolated LLVM 14 provider builds succeed; the provider negotiates ABI 25.
- Focused fake-provider coverage observes signed/unsigned integer and Float32/Float64 descriptors
  and exact libdevice demand.
- Real-provider tests serialize integer compare/select and Float32/Float64 libdevice declarations
  in LLVM 14 and NVVM IR 2.0 text; adjacent Half/vector floating overloads remain unsupported.
- The original 66 rows complete at O0/O3 with eight correct and 58 exact later failures.
- The full 452-row NVRTC/direct O0/direct O3 census has zero old-correct regressions.
- All three representative gates remain differentially correct, and direct O3 PTX assembles for
  SM70, SM80, and SM90.
- The selected NVVM regression prefix, promoted fixture directives, pinned formatter, and final
  diff checks run before commit.
- The direct NVVM lanes in the fixture-wide minimum/maximum run pass. Its unrelated existing WGPU
  lane still fails Dawn bind-group validation; the exact promoted direct indices pass independently.

The committed census TSV and cluster JSON are authoritative. Generated mirrors, logs, PTX,
cubins, and measurement samples remain under ignored `build/nvvm-census/`.
