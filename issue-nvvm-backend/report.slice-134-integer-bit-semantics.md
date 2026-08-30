# Slice 134: Canonical integer bit semantics

## Result

Slice 134 converts the largest coherent post-min/max ordinary-intrinsic family into one reusable
typed representation. All 11 measured `countbits`, `reversebits`, `firstbithigh`, and `firstbitlow`
workloads now compile and compare correctly through direct NVVM at O0 and O3. Exact workload-set
comparison against Slice 133 finds no previously correct regression.

The provider ABI advances from 25 to 26 by adding four generic operation IDs. The existing
descriptor-driven support query and emission callback are unchanged. No fixture-name check,
operation-specific callback, feature flag, syntax reconstruction, or fallback was added.

## Measured selection

The Slice-133 post-census inventory contained 58 ordinary-intrinsic first failures. Grouping exact
final `IRGenericAsm` assembly and specialized helper signatures identified this largest coherent
family:

| Canonical operation | First-blocker rows | Measured operand shapes |
| --- | ---: | --- |
| `$P_countbits($0)` | 5 | signed/unsigned 16-, 32-, and 64-bit integer paths |
| `$P_reversebits($0)` | 2 | UInt32 and UInt64 |
| `$P_firstbithigh($0)` | 2 | Int32 and Int64, including negative values |
| `$P_firstbitlow($0)` | 2 | UInt32 and Int64, including zero |
| Total | 11 | scalar selected integers |

The eleven workload IDs are `countbits`, `countbits16`, `countbits64`, `countbits8`,
`firstbithigh`, `firstbithigh64`, `firstbitlow`, `firstbitlow64`, `reversebits`, `reversebits64`,
and `scalar-uint`. All eleven become correct; none merely moves to another first blocker.

## Canonical producer and ownership

Consider `firstbithigh(value)` for a signed negative integer. The CUDA branch in
`source/slang/hlsl.meta.slang` selects the exact `$P_firstbithigh($0)` helper. Intrinsic expansion,
target-switch specialization, and linking leave a one-block `IRFunc` whose only ordinary
instruction is that `IRGenericAsm`. Its concrete parameter and result types are the canonical
producer output seen by direct-NVVM preflight.

The compiler now recognizes that final shape, not the source spelling or fixture. The specialized
signature supplies signedness and width. The shared semantic resolver admits only these contracts:

- `reversebits` returns the same selected scalar integer type as its operand;
- `countbits`, `firstbithigh`, and `firstbitlow` return scalar UInt32 from one selected scalar
  integer operand; and
- vectors, floating operands, wrong result types, wrong arity, and nonselected widths remain
  unsupported.

Signed `firstbithigh` complements a negative operand before counting leading zeroes. A zero scan
returns UInt32 all-ones. `firstbitlow` also returns UInt32 all-ones for zero. Forming scan results in
UInt32 before subtraction or selection preserves those sentinels for narrow operands. These rules
are semantic requirements of the canonical helper, so typed compiler/provider lowering owns them.

## Implementation

Forward-only ABI revision 26 adds `COUNT_BITS`, `REVERSE_BITS`, `FIRST_BIT_HIGH`, and
`FIRST_BIT_LOW` to `SlangNVVMValueOperation`. The callback remains the same generic
`SlangNVVMValueOperationDesc` query/emit interface.

The shared catalog introduces one `IntegerBit` family. The compiler replaces the min/max-specific
recognizer with a generic one-block value-helper recognizer used by both Slice-133 min/max and the
four new bit operations. Preflight, requirement collection, provider support query, and emission all
use the same descriptor.

The isolated provider constructs LLVM's typed `ctpop`, `bitreverse`, `ctlz`, and `cttz` intrinsics.
LLVM 7 already defines these generic intrinsics; its older NVVM-specific names are auto-upgraded to
them. LLVM 14 adds optimization attributes and an `immarg` marker to scan declarations. The
existing strict NVVM IR 2.0 writer was generalized from its prior `cttz.i32` rule: it validates the
intrinsic identity, selected width, exact signature, attributes, and parameter attributes before
removing only LLVM-14-only syntax. This is one deterministic dialect serialization path, not a
fallback or a post-hoc semantic rewrite.

## Coverage delta

The fixed denominator remains 452 eligible workloads from 448 sources: 430 MVP and 22 extension
lanes.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Infrastructure |
| --- | ---: | ---: | ---: | ---: | ---: |
| NVRTC O3 | 449 | 0 | 0 | 0 | 3 |
| Direct NVVM O0 | 237 | 7 | 203 | 5 | 0 |
| Direct NVVM O3 | 233 | 15 | 203 | 1 | 0 |

Successful compile-and-run coverage, including runtime mismatches, is 244/452 at O0 and 248/452 at
O3. Differential correctness gains exactly 11 rows in each mode relative to Slice 133. Exact set
comparison finds zero old-correct regressions.

Among the 427 MVP workloads with a healthy native reference:

- 236 compare correctly at O0;
- 231 compare correctly at O3; and
- 228 compare correctly at both optimization levels.

The ordinary-intrinsic cluster falls from 58 to 47. Post-slice MVP first-failure Pareto counts are:

| Root-cause cluster | O0 | O3 |
| --- | ---: | ---: |
| Ordinary intrinsic semantics | 47 | 47 |
| Wave/reconvergence semantics | 31 | 31 |
| Helper ABI type contracts | 28 | 28 |
| Aggregate/pointer/layout transport | 23 | 23 |
| Ordinary numeric/bit IR operations | 16 | 16 |
| Residual target marker or undefined value | 9 | 9 |
| Atomic/wave operation families | 8 | 8 |
| Narrow integer runtime correctness | 0 | 8 |

Within the residual 47 ordinary-intrinsic rows, scalar `abs` is the largest exact spelling at seven
and `tan` follows at five. The remaining rows include broader scalar libdevice math, Half transport,
and multi-result bit reinterpretation. That exact inventory, rather than fixture order, supplies the
next vertical-slice choice.

## Representative workload gates

All three release-gate workloads remain differentially correct in native CUDA, direct O0, and
direct O3. Median standalone compile time and PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | --- | --- | --- |
| Resource aggregate/helper | 381.2 ms / 8,889 B | 260.5 ms / 6,102 B | 265.0 ms / 919 B |
| Parameter-block layout | 367.2 ms / 8,839 B | 239.4 ms / 917 B | 244.2 ms / 793 B |
| Shared control/barriers | 369.5 ms / 9,190 B | 248.7 ms / 1,940 B | 255.4 ms / 1,404 B |

Each direct O3 module assembles with CUDA 12.9 `ptxas` for SM70, SM80, and SM90. Runtime comparison
uses the local RTX 5090/SM120. The recorded census times include compile, load, launch, execute, and
comparison overhead and therefore are not kernel-only performance measurements.

## Validation

- Release host and isolated LLVM 14 provider builds succeed; exact ABI 26 negotiation passes.
- Focused fake-provider compilation observes all four exact descriptors and no libdevice demand.
- Real-provider tests construct and serialize all four operations for 8/16/32/64-bit scalars in
  LLVM 14 and NVVM IR 2.0 forms.
- Adjacent vector, wrong-result, 24-bit, malformed-arity, and vector-helper cases remain rejected.
- The focused 11-row differential census is 11/11 correct in NVRTC O3, direct O0, and direct O3.
- All 22 promoted direct fixture lanes pass.
- The full 452-row census reports the exact +11/+11 delta and no old-correct regression.
- The selected NVVM regression prefix passes 402/402.
- The pinned formatter, full diff check, representative metrics, and SM70/80/90 assembly gates run
  before commit.

CUDA 13 tooling and physical SM70/80/90 runtime workers remain infrastructure gaps. The committed
census TSV and cluster JSON are authoritative; generated mirrors, raw logs, PTX, cubins, and timing
samples remain under ignored `build/nvvm-census/`.
