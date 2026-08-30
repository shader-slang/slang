# Slice 141: Lower integer truthiness and bitfields through typed recipes

## Motivation

The Slice 140 healthy-MVP Pareto contained eleven workloads whose first direct-NVVM stop was an
ordinary numeric or bit operation. Four first stopped at integer-to-Boolean `kIROp_IntCast`, two
at `kIROp_BitfieldExtract`, and five at `kIROp_BitfieldInsert`. Consider:

```slang
bool active = value;
uint inserted = bitfieldInsert(base, payload, offset, count);
int extracted = bitfieldExtract(signedValue, offset, count);
```

Checked integer truthiness intentionally remains an `IRIntCast`. The intrinsic declarations in
`core.meta.slang` produce the two bitfield instructions, and
`slang-ir-any-value-marshalling.cpp` uses the same instructions while transporting packed values.
These are canonical, producer-owned IR forms used beyond any one fixture. The direct emitter was
missing their semantic recipes and stopped deterministically in preflight.

## Proposed solution

The compiler now classifies scalar selected-integer-to-Boolean casts as truthiness and emits a
typed comparison against zero. It classifies bitfield extraction and insertion only when the
result and data operands have the same selected integer scalar/vector type and offset/count are
scalar UInt32.

A compiler-owned descriptor records the complete operation closure. Insertion creates an unsigned
shifted mask, clears the corresponding base bits, masks the shifted payload, and combines them.
Extraction first shifts an unsigned mirror logically, positions the requested field at the high
bit, and finishes with either a logical or signed arithmetic shift. Scalar counts and constants
are structurally splatted for vector data. Preflight and emission consume the same descriptor.

All steps use revision 29's generic constants, aggregate construction, conversions,
reinterpretation, comparison, shifts, and bit operations. The provider ABI remains unchanged.

## Change summary

- `source/slang/slang-emit-nvvm.cpp` adds exact truthiness and bitfield descriptors, resolvers,
  requirement collection, scalar-to-vector splats, and emission recipes.
- `source/compiler-core/slang-nvvm-semantic-catalog.h` admits selected integer vectors for the
  existing generic bitwise-not operation needed by vector mask complements.
- Focused fake-provider tests verify the typed graph and deterministic adjacent rejection; real
  provider tests verify signed/unsigned scalar/vector differential behavior and `ptxas`.
- Seven existing fixture files, representing eight workload identities, gain explicit direct O0
  and O3 runtime lanes.
- The fixed census/Pareto artifacts, this report, the committed plan, and durable design and
  capability documents record the measured result.

## Concepts and vocabulary

- **Selected integer type**: the bounded scalar/vector integer family admitted by the shared NVVM
  semantic catalog.
- **Unsigned mirror**: the same physical integer width and lane count with unsigned semantics,
  used so the initial bitfield shifts cannot sign-extend.
- **Recipe closure**: every typed provider operation required by an admitted canonical IR shape,
  collected before provider discovery or module construction.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC O3 lane is
  correct.
- **Selected prefix**: focused direct-NVVM unit tests; a regression score, not the corpus coverage
  denominator.

## Process report

### Canonical producers, rather than fixture names, define admission

`IRBuilder::emitBitfieldExtract` and `emitBitfieldInsert` create the ordinary bitfield
instructions used by both user intrinsics and AnyValue marshalling. Final linked IR consistently
uses equal result/data types and scalar UInt32 offset/count operands, including when the data is a
vector. This is intentional canonical input, so the target emitter owns its lowering.

`_resolveNVVMBitfieldOperation` validates that entire contract. It does not inspect source syntax,
function identity, or a fixture path. Floating, Boolean, matrix, aggregate, mismatched, and
vector-count shapes remain deterministic unsupported-IR diagnostics. Removing the shared resolver
returns every promoted workload to its recorded first bitfield preflight stop.

Checked scalar integer truthiness reaches the same consumer as a canonical `IRIntCast` with a
Boolean result. `_resolveNVVMIntegerTruthiness` accepts exactly that result/operand contract and
emits `value != typed-zero`. Floating-point truthiness remains owned by the separate unsupported
`castFloatToInt` family; the focused negative source proves that this branch does not widen it.

### Bit transport is explicit and sign-correct

Bitfield insert and extract are physical bit operations even when their source type is signed.
The resolver therefore derives an unsigned mirror with the same width and lane count. Signed data
is bit-reinterpreted before the initial shift. Insertion performs all mask algebra on that mirror
and reinterprets the combined result back to the exact signed type.

Extraction logically shifts the unsigned input by `offset`, computes `width - count`, and shifts
the selected bits to the physical high end. Unsigned extraction shifts them back logically.
Signed extraction reinterprets only at that boundary and shifts back arithmetically, making the
requested field's high bit the sign bit. This matches the ordinary LLVM emitter's semantic recipe
without asking the provider to infer signedness from source syntax.

The offset/count values are scalar UInt32 in canonical IR. For 8-, 16-, or 64-bit data they are
converted to the matching unsigned scalar width, and for vector data the compiler builds an exact
structural splat. Constants use the same structural path. No implicit provider broadcast or
target-dependent host conversion is involved.

### The one catalog widening is an existing generic operation

Vector insertion needs the complement of a vector shifted mask. The shared semantic catalog
admitted `SLANG_NVVM_VALUE_OP_BIT_NOT` for scalar selected integers only, although the generic
provider implementation already creates LLVM `not` for selected integer vectors and other binary
bit operations already admit that family. The catalog row now admits selected integer
scalar/vectors. Without that widening, the exact canonical vector insertion closure fails before
provider discovery; `layout-8bit-vectors.slang` and the focused `uint2` insertion source prove that
this layer owns it. Negation and absolute value remain scalar-only.

No callback was added because the provider can already express every primitive. The resolver also
constructs and requests logical/signed right-shift steps only for extraction, rather than making
insertion depend on unused capabilities.

### The cluster probe exposed later blockers without speculative widening

The bounded probe advances all eleven original rows past their first unsupported shape. Eight
become differentially correct at both direct modes. Three reveal independent later failures:

| Workload | Later canonical stop |
| --- | --- |
| `bugs/gh-4533` | `LoadFromUninitializedMemory` |
| `language-feature/conversions/conversion-to-bool` | floating `castFloatToInt` |
| `language-feature/dynamic-dispatch/layout-mixed-bitwidths` | `makeUInt64` |

Those operations belong to residual-marker, numeric-conversion, and aggregate/helper work,
respectively. They are reclassified in the Pareto instead of attracting a compatibility fallback
or a fixture-specific exception in this slice.

The promoted fixtures cover ordinary scalar extract/insert, AnyValue 8-bit packing, vector and
enum-underlying-type dynamic-dispatch layout, struct reinterpretation, and two native CUDA
workload identities in `wave-active-count-bits.slang`. All seven CUDA fixture runs pass; the two
bitfield files' unrelated WebGPU bind-group-layout failures do not affect their explicit direct
CUDA lanes.

### Fixed-denominator coverage and Pareto result

The fixed corpus remains 452 workloads from 448 sources: 430 MVP and 22 extension. Native
CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 338 | 8 | 99 | 7 | 346 |
| Direct O3 | 343 | 8 | 99 | 2 | 351 |

Both direct modes gain eight exact success identities and lose none from Slice 140. All eight are
MVP. Among 427 healthy MVP references, O0 correctness is 336/427 (78.7%), O3 correctness is
340/427 (79.6%), and both-mode correctness is 336/427 (78.7%). The ordinary-numeric/bit cluster is
eliminated because its three non-success rows now report their later root causes.

The leading remaining healthy-MVP clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Helper ABI/type contract | 16 | 16 |
| Aggregate/pointer/layout transport | 14 | 14 |
| Residual target marker/undefined value | 10 | 10 |
| Preflight other | 8 | 8 |
| Wave/reconvergence GenericAsm | 8 | 8 |
| Atomic/wave operation | 8 | 8 |
| Function identity | 6 | 6 |
| Raw-buffer view access | 4 | 4 |

O0 additionally has four healthy provider failures in the unoptimized half-operation cluster.
This measured ranking, rather than slice count or selected-prefix size, drives the next slice.

### Representative and productionization gates

All three representative release gates remain differentially correct. Median standalone compile
time and generated PTX size from three samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 383.8 ms / 8,889 B | 262.7 ms / 6,102 B | 271.2 ms / 919 B |
| Parameter-block layout | 366.5 ms / 8,839 B | 247.2 ms / 917 B | 250.3 ms / 793 B |
| Shared control/barriers | 374.1 ms / 9,190 B | 250.0 ms / 1,940 B | 254.9 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
4782.5/5095/4854.2 ms for NVRTC O3, 4570/4817/4600.9 ms for direct O0, and
4611.5/4926/4644.1 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 workers
remain productionization gaps. The isolated LLVM 14 provider remains compiler-matched at ABI
revision 29.

### Validation and self-review

- Release compiler and unit-test builds pass outside the sandbox; the isolated provider is rebuilt
  even though its ABI is unchanged.
- Focused typed-graph, adjacent floating-truthiness rejection, real differential, and real
  `ptxas` tests pass.
- All sixteen promoted direct O0/O3 lanes pass through CUDA validation.
- The final three-mode census has eight gains and zero old-correct regressions.
- The selected direct-NVVM prefix passes 413/413.
- Representative direct O3 PTX assembles for SM70, SM80, and SM90.

The new-helper inventory is the two descriptors/resolvers, their requirement collectors, scalar
and constant splat materializers, count lowering, and two emitters. Each consumes an exact
canonical instruction or emits one piece of its typed recipe. The shared catalog widening is
proven by canonical vector mask algebra. No fixture-name check, syntax reconstruction,
compatibility fallback, arbitrary operand-graph walk, or provider ABI widening is retained.
