# Slice 143: Separate structured-buffer storage from semantic values

## Motivation

The leading Slice 142 aggregate/layout cohort contained five workloads whose canonical values were
already meaningful but whose LLVM value representation was not the CUDA structured-buffer storage
representation. Consider the Boolean fields from `bugs/gh-7441.slang`:

```slang
struct TestType
{
    uint value;
    bool f_bool;
    bool1 f_bool1;
    bool pad1;
    bool2 f_bool2;
    bool pad2;
    bool3 f_bool3;
    bool4 f_bool4;
    uint END;
}

RWStructuredBuffer<TestType> buffer;
```

The semantic Boolean values are LLVM i1 values, but CUDA external storage assigns one byte to a
Boolean lane and gives bool2/bool4 their natural two-/four-byte vector alignment. Numeric vector3
has the complementary problem: it is a first-class LLVM vector value, while CUDA buffer storage
uses a compact three-scalar stride. Fixed arrays, structs, and legalized matrices recursively carry
those differences.

The old direct path used the ordinary semantic provider type as the raw-buffer pointer pointee.
Preflight correctly rejected a mismatched pointer stride or field layout, so the five workloads
stopped at `structured-buffer element layout` or at the enclosing conventional-global field
address. Changing the canonical IR type or weakening that check would have hidden an external ABI
error. The required invariant was a separate physical provider representation at the exact buffer
boundary, with explicit conversion back to the unchanged semantic value.

## Proposed solution

The compiler now owns a finite recursive `StructuredBufferStorage` type use. Selected numeric and
Boolean scalars/vectors, fixed arrays, and nonempty structs compose through it. Boolean leaves are
i8; numeric vector3 and bool3 are scalar arrays; bool2/bool4 are i8 vectors; other selected leaves
retain their proven LLVM representation. One-field `PhysicalType` matrix wrappers use the same
storage identity.

Raw structured-buffer views and writable element pointers lower their pointee under this physical
use. Direct loads and ordinary pointer-chain loads reconstruct the semantic value recursively;
stores decompose it in the inverse direction. The conversion uses the provider's existing generic
integer comparison/conversion, vector extraction/construction, aggregate extraction/construction,
pointer, load, and store operations.

Before provider discovery, the compiler proves every explicit array stride and direct struct field
offset against CUDA layout. Final provider size must equal CUDA size. Loads and stores carry CUDA's
explicit conservative alignment, so a stronger provider-preferred root alignment is allowed only
when it changes neither pointer stride nor an addressable field byte.

Resource-containing elements such as `MyImpl { Texture2D tex; }` remain a separate established
family whose ordinary value representation already has an exact CUDA/LLVM layout. The compiler
selects storage versus ordinary value representation from the canonical element type; it does not
fall back after a failure. Provider ABI revision 29 already expresses every operation and remains
unchanged.

## Change summary

- `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` adds the recursive external-storage
  classifier, a dedicated type map/use, use-sensitive pointer caching, physical bool/vector/array/
  struct lowering, and exact structured-buffer pointee selection.
- `source/slang/slang-emit-nvvm.cpp` adds the CUDA/provider layout proof, complete Boolean operation
  requirements, exact structured-buffer pointer-root classification, and recursive storage/value
  conversion shared by direct and pointer-based loads/stores.
- Matrix aggregate construction converts compact vector operands before constructing the existing
  physical array wrapper. Reachable-type collection retains the resulting aggregate declaration
  closure.
- Focused fake-provider expectations now describe recursive aggregate reconstruction and explicit
  CUDA memory alignment. The existing incompatible-layout negative remains deterministic.
- Five existing corpus fixtures gain direct O0/O3 runtime lanes. The committed 452-row census,
  Pareto cluster file, plan, this report, and durable design documents record the measured result.

## Concepts and vocabulary

- **Semantic value**: the canonical final linked-IR type and value seen by ordinary SSA, helpers,
  control flow, and source-level aggregate operations.
- **Structured-buffer storage**: the provider-only physical pointee/value spelling selected at an
  external typed-buffer boundary.
- **Physical wrapper**: a `PhysicalType` struct created by matrix legalization whose one array field
  already carries the external storage spelling.
- **Pointer stride proof**: equality of final provider/CUDA element size, explicit nested array
  strides, and direct field offsets; preferred root alignment is not itself an addressable byte.
- **Healthy MVP reference**: one of the 427 MVP workloads whose native CUDA/NVRTC O3 lane is
  correct.
- **Selected prefix**: focused direct-NVVM unit tests; a regression score, not the coverage
  denominator.

## Process report

### The canonical producer is correct; the boundary representation was missing

`collectGlobalUniformParameters` places each resource in the synthesized keyed `GlobalParams`
block. Resource lowering preserves the specialized element type in `NVVMRawBufferType` and emits
canonical `StructuredBufferLoad`, `RWStructuredBufferGetElementPtr`, `GetStructuredBufferPtr`,
`Load`, and `Store` operations. For example, `StructuredBuffer<Payload>.Load` still produces an
exact `Payload` value:

```slang
struct Payload
{
    float3 value;
    uint flags;
}

StructuredBuffer<Payload> input;
Payload item = input.Load(0);
```

That shape is intentional. Helpers, phis, and aggregate operations consume `Payload`, not a target
storage surrogate. The defect was in `NVVMTypeLoweringContext::_lowerRawBufferType`, which formerly
used `NVVMTypeUse::Value` for the data-pointer pointee. The fix therefore lives at type lowering and
memory emission, not in resource lowering or semantic IR construction.

`isNVVMSupportedStructuredBufferStorageType` accepts only a finite recursive algebra. It rejects
cycles, empty structs, unsized arrays, non-literal counts, unsupported leaves, and explicit strides
that cannot be represented. `PointerTypeKey` now includes the pointee use so an ordinary semantic
pointer handle cannot make the same canonical pointer appear physically valid at a structured-
buffer boundary.

Removing the dedicated pointee use reproduces all five original layout/preflight failures. The
promoted array, bool aggregate, float3 aggregate, existential/resource aggregate, and matrix lanes
prove that the representation is selected by canonical shape rather than fixture identity.

### Physical layout is derived and proven recursively

`_getNVVMStructuredBufferStorageLayout` mirrors the exact provider type construction:

- Boolean scalar storage is one-byte i8.
- Numeric vector3 and bool3 storage are three-element scalar arrays.
- Bool2 and bool4 storage are i8 vectors, which give LLVM the same two-/four-byte alignment as
  CUDA.
- Fixed arrays use the recursively derived element stride and require an explicit canonical stride,
  when present, to equal it.
- Nonempty structs align each physical field, compare that offset with CUDA's canonical field
  offset, and compute final pointer stride from the physical fields.

An early prototype required equal provider/CUDA preferred aggregate alignment. The established
fake `Thing { uint, float, half4 }` graph disproved that requirement: both layouts have size 16 and
field offsets 0/4/8, but the provider prefers alignment eight while CUDA reports four. Pointer
arithmetic and every field address are still identical. Emission now passes CUDA alignment four to
the final memory operation. An incompatible field offset or explicit stride still fails preflight;
no padding guess or byte-copy path is retained.

The exact input-shape audit is therefore:

1. The final type comes from the structured-buffer resource producer and remains canonical.
2. The physical representation is derived only after that producer establishes an external buffer
   boundary.
3. Provider size, nested stride, and field offsets must match CUDA before module construction.
4. Semantic values are never rebuilt from syntax or replaced in linked IR.
5. `gh-7441`, `gh-8121`, `dynamic-dispatch-{16,17}`, `make-matrix`, and the adjacent incompatible
   layout test prove this layer owns the distinction.

### One recursive conversion serves direct and derived pointers

`_getNVVMStructuredBufferStoragePointerValueType` recognizes only pointer chains whose canonical
root is `RWStructuredBufferGetElementPtr` or the raw structured-buffer data-pointer operation. It
uses the existing `getRootAddr` address utility and the exact semantic pointee type. Unrelated
locals, parameter groups, shared values, and ordinary device pointers cannot acquire this storage
interpretation.

Preflight checks that every load/store semantic type equals that pointee and records all nested
Boolean operations before provider mutation. Emission recursively extracts physical aggregate
elements or compact vector lanes, converts Boolean i8 with `!= 0`, and constructs the exact
semantic result. Stores traverse the same declarations in reverse and convert Boolean values to
i8. Direct `StructuredBufferLoad` uses the identical converter after its pointer-offset/load
sequence. Field order comes from canonical struct fields; no syntax, fixture name, or positional
witness data participates.

The fake provider intentionally maps every integer width to one fake scalar handle, so it cannot
faithfully distinguish semantic i1 from storage i8. Its existing copyable-struct graph still proves
recursive extraction/reconstruction and final CUDA alignment. `gh-7441` supplies authoritative
real-provider/runtime coverage for bool, bool1, bool2, bool3, and bool4 storage in one aggregate.

### Matrix legalization exposed the only construction-side boundary

`cuda/make-matrix.slang` stores a canonical `uint4x3`:

```slang
uint idx = 1;
uint4x3 mat1 = uint4x3(idx, idx, idx, idx, idx, idx, idx, idx, idx, idx, idx, idx);
outputBuffer[0] = mat1;
```

Final legalization produces a `PhysicalType` wrapper around an explicit-stride array. Its
`MakeArray` operands remain semantic vector3 values, while the array field is compact scalar-array
storage. `_getNVVMAggregateConstruction` now records the result type use; construction converts
each vector operand before building the array. The one-field physical wrapper is already the
storage value and shares its provider handle between ordinary value and buffer pointee roles.

This is a valid producer-side shape: matrix legalization deliberately records physical storage in
the wrapper and explicit stride. Patching the final store would leave helper construction and any
other producer inconsistent. Removing construction-side conversion makes the matrix workload fail
provider type validation, while the other four promoted workloads continue to pass.

### The first census caught a valid pre-existing representation family

The initial full run gained all five targets but regressed the representative
`dynamic-dispatch-bindless-texture` workload. Its element is:

```slang
struct MyImpl
{
    Texture2D tex;
}

StructuredBuffer<MyImpl> gCb;
```

`MyImpl` is canonical and intentionally valid; its texture handle already has an exact ordinary
CUDA/LLVM value representation. It is not a numeric/Boolean storage aggregate and must not enter
the recursive converter. `_getNVVMStructuredBufferElementTypeUse` now classifies these two complete
families before type lowering. `_hasNVVMCompatibleRawBufferElementLayout` applies the matching
layout proof for the chosen family. There is no retry or compatibility fallback.

The bindless-texture gate passes with the five target workloads after this correction. A final
452-row census reports zero old-correct loss. Self-review also proposed retaining more unused
resource declarations and conditionally widening unrelated aggregate construction; a revert drill
found no measured owner for either change, so both speculative edits were removed.

### Fixed-denominator coverage and Pareto result

The corpus remains 452 eligible workloads from 448 sources: 430 MVP and 22 extension workloads.
Native CUDA/NVRTC O3 is correct for 449; three rows remain infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 346 | 8 | 91 | 7 | 354 |
| Direct O3 | 351 | 8 | 91 | 2 | 359 |

Both direct modes gain exactly five workload identities and lose none from Slice 142. Against 427
healthy MVP references, O0 correctness is 344/427 (80.6%), O3 correctness is 348/427 (81.5%), and
both-mode correctness is 344/427 (80.6%). This crosses the bounded MVP's initial 80% differential
threshold, but feature-family and productionization gates remain part of the MVP definition.

`bugs/gh-5776` advances from `helper function parameter` to the later `struct field address`
blocker. It remains preflight in both modes and is counted only as a root-cause reclassification.
The leading healthy-MVP clusters are:

| Root-cause cluster | O0 blocked | O3 blocked |
| --- | ---: | ---: |
| Preflight other | 11 | 11 |
| Residual target marker/undefined value | 10 | 10 |
| Atomic/wave operation | 10 | 10 |
| Aggregate/pointer/layout transport | 8 | 8 |
| Helper ABI/type contract | 8 | 8 |
| Wave/reconvergence GenericAsm | 8 | 8 |
| Function identity | 6 | 6 |
| Raw-buffer view access | 4 | 4 |

O0 additionally has four healthy unoptimized-Half provider failures. The next slice should
decompose the three tied eight-row semantic clusters by exact producer before choosing another
vertical representation.

### Representative and productionization gates

All three representative release gates remain differentially correct. Median standalone compile
time and generated PTX size from three final samples are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 385.8 ms / 8,889 B | 267.1 ms / 6,102 B | 270.0 ms / 919 B |
| Parameter-block layout | 370.9 ms / 8,839 B | 245.2 ms / 917 B | 251.6 ms / 793 B |
| Shared control/barriers | 372.2 ms / 9,190 B | 254.8 ms / 1,940 B | 259.6 ms / 1,404 B |

Across all census lanes, startup-inclusive compile/load/execute/compare median/p90/mean times are
5799/6394/5904.9 ms for NVRTC O3, 5733.5/6249/5755.8 ms for direct O0, and
5841.5/6560/5900.6 ms for direct O3. These are not kernel-only runtime measurements.

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local SM120 GPU. CUDA 13 tooling and physical SM70/SM80/SM90 workers remain
productionization gaps. The isolated LLVM 14 provider remains compiler-matched at ABI revision 29.

### Validation and self-review

- Release compiler/unit-test and isolated provider builds pass outside the sandbox.
- The focused aggregate graph, raw i32 direct path, and adjacent structured-buffer negative tests
  pass after their physical-representation expectations are updated.
- The regression gate plus all five promoted fixture groups pass 25/25 relevant lanes; three
  unrelated backend lanes are ignored by their existing conditions.
- The final 452-workload census has five both-mode gains and zero old-correct regressions.
- The selected direct-NVVM prefix passes 421/421.
- Representative direct O3 PTX assembles for SM70, SM80, and SM90.
- Pinned formatting and `git diff --check` pass.

The new-helper/special-case inventory is the finite storage classifier; structured-buffer type use
and pointer cache key; element type-use classifier; recursive layout proof; exact pointer-root
classifier; Boolean requirement closure; recursive conversion; compact matrix construction case;
and resource-containing ordinary-value branch. Each item above names its canonical producer,
explains why the shape is valid, and names coverage that fails without it. No fixture-name check,
syntax reconstruction, custom semantic type equivalence, retry fallback, arbitrary operand walk,
silent padding guess, malformed-upstream patch, or provider ABI widening is retained.
