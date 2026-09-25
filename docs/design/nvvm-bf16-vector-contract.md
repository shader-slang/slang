# BF16 vector value contract and storage boundary

Research slice240 qualified the value contract; implementation slice241 now supports the register and internal-helper roles described below with builder ABI39. The research evidence is
[semantic-evidence.slice-240.json](../../issue-nvvm-backend/semantic-evidence.slice-240.json), following
scalar ABI38 in [the backend design](nvvm-backend.md). The tested platform is LLVM14/libNVVM12.9,
SM80, with CUDA12.9.2/NVRTC12.9.86 on an L4. Requalify on target or dialect changes.

Consider this example, where the inputs can contain any BF16 encoding, including signaling NaNs:

```slang
[noinline]
vector<BFloat16, 3> choose(
    vector<BFloat16, 3> a, vector<BFloat16, 3> b, bool flag)
{
    vector<BFloat16, 3> result = b;
    if (flag) result = a;
    return result;
}

vector<BFloat16, 3> a = vector<BFloat16, 3>(
    bit_cast<BFloat16>(uint16_t(bits0)),
    bit_cast<BFloat16>(uint16_t(bits1)),
    bit_cast<BFloat16>(uint16_t(bits2)));
vector<BFloat16, 3> splat = vector<BFloat16, 3>(a.x);
float3 expanded = float3(a);
vector<BFloat16, 3> narrowed = vector<BFloat16, 3>(float3(f0, f1, f2));
vector<BFloat16, 3> selected = choose(a, splat, inputFlag);
```

The core library's BFloat16Type is the semantic source of truth. Vector construction intentionally
produces `makeVector` or `MakeVectorFromScalar`; component access is a typed `swizzle`; conversion
is matching-lane-count vector `FloatCast`. Helpers keep those exact vector parameter/result types. These are
valid canonical producer shapes, not malformed values for an emitter to repair.
`CUDASourceEmitter::tryEmitInstExprImpl` emits vector constructors and component casts using the
prelude's `make___nv_bfloat16N`. Each Float32 lane narrows separately. SM80 narrowing uses
`cvt.rn.bf16.f32`, and widening places the BF16 payload in the high 16 bits of Float32. Narrowing
NaNs are classification-only; exact widening includes all NaN payload bits on this target.

`bit_cast<vector<BFloat16,2>>(uint32)` and the width4/uint64 pair are valid equal-size source
operations. Width3 also supports `ushort3` in both directions. `BitCastLoweringContext::processBitCast`
checks natural size and `readObject` reconstructs the destination from component-sized loads of the
source value. Final generated CUDA and direct-preflight IR reduce these probes to scalar UInt16/BF16
bitcasts plus shifts, extraction and construction. Early IR retains the whole-vector bitcast.
Reuse this producer; no source i48 type or new alternate semantic bitcast representation is required.
The raw LLVM i48 roundtrip is an isolated feasibility control, not a proposed public operation.

## Register and helper values

Physical `<N x i16>` is a qualified register representation for semantic BF16 widths2/3/4. It
preserves every payload and supports `insertelement`, `extractelement`, input-dependent helper
selection and by-value noinline helper parameters/results at libNVVM O0/O3. The raw helper call ABI
is internally consistent, but does not match the CUDA source helper ABI for every width. For example,
raw BF3 helper parameters/results are `.align8 .b8[8]`; source CUDA BF3 uses `.align2 .b8[6]`.
There is no external CUDA helper interoperability claim. The selection control qualifies helper
branch/phi behavior; it does not establish a canonical vector `Select` semantic-catalog contract.
Any new explicit vector `Select` admission needs its own source/IR test in the implementation slice.

Slice241 admits these value/by-value helper roles explicitly using
existing `NVVMTypeUse` caches, vector construction/extraction, helper signature validation and
semantic operation planning. `_getNVVMSemanticType` preserves BF16 as a distinct descriptor;
`_getSemanticLLVMType` uses the qualified physical i16 vector. Matching-lane-count BF16/Float32 conversion
reuses the scalar `ValueOperationFamily::BFloat16Convert` semantics component by component,
with exact lane-count preflight. Do not widen the IEEE floating classifier or ordinary numeric
operations: BF16 arithmetic, comparisons, integer conversion, Half/double casts and dot remain
unqualified by this vector research. Existing scalar conversion/bitcast behavior must stay exact.

## Storage is a separate contract

The actual prelude/CUDA headers and LLVM14 provider DataLayout give the following byte sizes:

| Width | CUDA size/alignment | LLVM i16 vector store size / allocation size / ABI alignment | LLVM i16 array allocation size / ABI alignment |
| ----- | ------------------- | ------------------------------------------------------------ | ---------------------------------------------- |
| 2     | 4 / 4               | 4 / 4 / 4                                                    | 4 / 2                                          |
| 3     | 6 / 2               | 6 / 8 / 8                                                    | 6 / 2                                          |
| 4     | 8 / 2               | 8 / 8 / 8                                                    | 8 / 2                                          |

CUDA BF2 is native `__nv_bfloat162`; BF3/BF4 are the prelude's plain component structs. BF3 has no
CUDA tail padding. Treating its LLVM value vector as storage changes array stride and field offsets.
BF4's LLVM vector alignment likewise changes member offsets. A scalar array matches BF3/BF4 but
loses BF2's type alignment; the raw local BF2 array explicitly uses `alloca ... align4`. That
allocation alignment does not change the array type's ABI alignment inside aggregates.

The research measured source `replace(inout BFVector, BFVector)` and a raw component-array
roundtrip through noinline pointer helpers. All source and raw PTX retain local stores/loads and
calls; the local BF3 depot is six bytes/alignment2. This establishes a candidate local representation,
not permission to enable local pointers, aggregates, resources, globals or parameter groups through
a shared numeric classifier. Leave new local/storage admission outside the next value-only slice.

There is also a producer/model issue to resolve before BF4 external storage work. In
`slang-ir-layout.cpp`, `CUDALayoutRules::calcSizeAndAlignment` special-cases Half, then the generic
vector rule gives BF4 alignment8. In `slang-type-layout.cpp`, `_createTypeLayout` supplies the BF16
scalar layout and `CUDALayoutRulesImpl::GetVectorLayout` similarly takes the generic width4 rule;
it lacks a BF16 format distinction. The actual prelude BF4 alignment is2. A future storage slice
must make both layout producers agree with the target representation, then use existing role-specific
lowering, aggregate layout validation and component conversion machinery. Do not bypass
`_getNVVMAggregateStorageLayout` checks or declare the accidental eight-byte alignment canonical.

Existing `_emitNVVMStructuredBufferStorageConversion`, compact vector load/store paths and
`NVVMTypeLoweringContext::lowerType` show how values cross array/vector boundaries. Their current
admission is intentionally narrower: they are reusable patterns and operations, not an authorization
to admit BF16 structured buffers. Half's compact two-lane chunks have different padding and must not
be applied to BF3/BF4. A storage implementation needs its own offsets, stride, alignment, adjacent-field
and pointer evidence, including the producer fix above.

The unchanged frozen scalar-bf16 workload additionally needs source-ordered dot. Exact integer
construction remains separately gated by research238's double-rounding counterexample. Neither
is implied by vector transport or component conversions.

## Implemented value roles (slice241)

`asNVVMBFloat16VectorType` checks canonical BF16 widths2/3/4. `asNVVMRegisterVectorType` combines that classification with established ordinary vectors only for construction, extraction, value availability and semantic descriptions. It does not widen recursive numeric/copyable/helper/storage algebras. `NVVMTypeInfo::supports` admits Value/HelperValue/HelperParameter/HelperResult and rejects every vector storage role before consulting provider-handle caches. Helper branch joins use ordinary phi emission with explicit register-only preflight admission. Mixed vector constructor operands, multi-lane swizzles and dynamic extraction retain canonical IR and use the existing vector builders. SwizzleSet remains outside this BF contract.

The shared catalog requires matching lane counts1..4 and exact BF16/Float32 widths16/32. `_emitBFloat16ConvertLane` owns the scalar SM80 narrowing/expansion recipe; vector conversion extracts and reconstructs lanes through that same recipe. Explicit vector Select, arithmetic/comparison/dot and integer/Half/double casts remain rejected. Source whole-vector bit transport still uses canonical bitcast decomposition, not a new provider vector-bitcast operation.

Reachable `[CudaDeviceExport]` helpers require a separate external CUDA ABI. `_validateNVVMHelperTarget` rejects BF vector result/parameter types there: BF3's provider eight-byte/alignment-eight signature differs from CUDA six-byte/alignment-two. Ordinary exported integer helpers remain supported. The input is canonical; the target ABI is unqualified, so this boundary belongs in helper preflight, not a producer repair or layout workaround.

The registered `tests/cuda/nvvm-bf16-vector-values.slang` fixture combines independent boundary expectations with dynamic helper branches, bit transport and lane operations. Exhaustive value-only projections retain research240's73190 input records per width. They remove only pointer/local replacement and writes29..29+N-1, preserving those words as original sentinels. Nine production launches and six separate raw controls are checked against independent integer-oracle reconstruction. Neither this value evidence nor compiler diagnostic movement resolves frozen scalar-bf16's source-ordered dot.
