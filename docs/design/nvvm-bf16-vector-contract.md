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

Research240 also identified a producer/model issue before BF4 external storage work: both AST
and IR CUDA layout producers gave BF4 alignment8 instead of the actual prelude alignment2.
Research253 qualified its reflection and runtime-query consequences; slice254 repairs both
producers while preserving canonical BF16 identity, as described below. Future storage support
must still use existing role-specific lowering, aggregate layout validation and component conversion
machinery. Do not bypass `_getNVVMAggregateStorageLayout` checks or treat LLVM vector alignment as
the CUDA component-struct ABI.

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

## Source-ordered dot (slice242)

Builder ABI40 adds a dedicated BF16 dot semantic for two matching BF16 vector operands of widths
2, 3 or 4 and an exact scalar BF16 result. `hlsl.meta.slang::dot` already produces the intentional
one-block CUDA GenericAsm `_slang_vector_dot`; the existing canonical helper validator and spelling
table map it to this operation. The shared catalog owns signature qualification. Neither the generic
IEEE arithmetic classifier nor any storage or external helper ABI admission changes.

Consider this concrete cancellation case:

```slang
let a = vector<BFloat16, 2>(BFloat16(-1.0), BFloat16(1.0078125));
let b = vector<BFloat16, 2>(BFloat16(1.015625), BFloat16(1.0078125));
BFloat16 result = dot(a, b);
```

The CUDA prelude starts with BF16 positive zero. The first product is -1.015625. The exact second
product is 1.01568603515625, which rounds separately to BF16 1.015625, so addition returns positive
zero. Fusing that second multiplication with the accumulated negative value instead returns
2^-14 (BF16 bits0x3880); accumulating products in Float32 also returns that nonzero value. Appending
zero lanes preserves the counterexample for widths3 and4.

`_emitBFloat16Dot` extracts the already-qualified physical i16 vector lanes in order. On SM80,
`fma.rn.bf16(a,b,-0)` implements each multiplication and `fma.rn.bf16(product,1,sum)` implements each
addition, exactly as CUDA's installed `__hmul`/`__hadd` recipes. Negative zero is necessary for
multiplication's signed-zero behavior. Every inline assembly call is a separate BF16 rounding
boundary; it cannot become an LLVM Float32 reduction or contract with another call. Native BF16
add/mul require SM90 and remain unused. NaN outputs promise classification, not a universal payload.

Before promotion, source NVRTC and raw physical-i16-vector LLVM O0/O3 controls each qualified all
three widths on targetSM80. They preserve research238's679 records and append the reversed
cancellation case above, retaining the original records and all inactive buffer words. Production
replays use those same inputs and independent exact-rational expectations. Provider tests check
both LLVM dialects, the2N separate BF16 FMA calls, exact constants and malformed semantic/physical
operands. General BF16 arithmetic/comparison, integer/Half/double conversion, explicit vector Select,
storage and external CUDA helper interoperability remain outside this dot contract.

## Research253: reflection, CUDA queries and stored records

[Research253](../../issue-nvvm-backend/report.slice-253-bf16-storage-layout.md) makes the recorded
BF4 layout mismatch observable on accepted252. For `struct W { uint16_t prefix;
vector<BFloat16,4> value; uint16_t suffix; }`, public CUDA reflection and canonical IR CUDA rules
report size/alignment24/8 and value/suffix offsets8/16. The actual CUDA prelude gives12/2 and2/10.
A holder containing `W values[3]` followed by uint16_t reports80/8 with tail offset72 instead of
38/2 with tail offset36. All33 neighboring rows (scalar BF16, BF2/BF3, Half and ordinary vectors)
match the actual ABI; only the three BF4 rows disagree.

Source NVRTC pointer and StructuredBuffer controls each read three records. Actual ABI packing
preserves all18 logical values; reflection-based packing yields17 wrong fields. Every input and
sentinel byte is preserved. These are isolated research controls, with no existing input/oracle
change or new storage support. StructuredBuffer uses its actual16-byte pointer/count parameter.

The core `__sizeOf<T>()`/`__alignOf<T>()` queries are compile-time metadata. NVVM's
`_getNVVMCUDALayoutQueryValue` resolves them through the CUDA rule before runtime type admission.
A72-value scalar-output control runs in all three modes: NVRTC matches the actual ABI, while each
direct mode returns five wrong BF4 values (vector alignment, wrapper size/alignment, holder
size/alignment). The saved accepted250/pre252 compiler emits byte-identical PTX, so these failures
predate constructor inlining. This supplies a runnable producer-repair regression without needing
BF16 aggregate/storage admission.

Unqualified Slang `sizeof`/`alignof` instead selects Natural rules in `PeepholeContext::processInst`.
For example, BF2 has Natural alignment2 and wrapper size8, while its CUDA ABI is4 and12. Preserve
that distinct observable policy; matching Natural BF4 values does not validate CUDA layout.

On the researched252 source, `_createTypeLayout` passes BaseType::Void for non-basic BF16,
so the CUDA vector rule cannot distinguish it from other non-basic elements. This identified the
producer boundary repaired by254 below, without introducing another BF16 AST type, using UInt16/Half
as a semantic substitute, patching reflection output or weakening downstream aggregate checks.

No new physical LLVM storage representation is qualified by253. The role-specific boundaries and
component-array/vector distinctions recorded240 remain requirements for a separate storage slice.

## Slice254: canonical CUDA BF16 layout

The internal AST vector/matrix query now retains its canonical element Type*. CUDA rules recognize
`BFloat16Type` directly and preserve the prelude's BF3/BF4 component layout. The IR CUDA rule uses
canonical `kIROp_BFloat16Type` for the same two widths. BF2 remains native4/4; BF3 is6/2 and BF4 is8/2.
Half3/Half4 remain8/4, ordinary ushort4 remains8/8, and Natural rules retain their separate behavior.
Other target layout rules continue ignoring the element-semantics argument; varying scalar rules
retain their existing BaseType input. No second type or scalar-kind mapping is introduced.

Consequently, the wrapped BF4 record above is12/2 with offsets2/10 and its holder is38/2 with tail36
in both public reflection and explicit CUDA queries. Row-major BF matrices inherit the corrected
row-vector layout through existing array construction:2x4 is16/2 and3x4 is24/2. The qualified4x2
neighbor remains16/4 and4x3 remains24/2. This does not change matrix orientation policy.

The executable96-value regression covers all16 scalar/vector/matrix neighbors, their wrappers and
three-record holders at NVRTC O3 and NVVM O0/O3. Public-reflection coverage additionally checks
strides and field offsets. Separate canonical IR controls preserve all36 Natural rows. Host bytes
packed from corrected reflection equal the previously qualified CUDA-packed inputs; original
incorrectly packed inputs remain recorded negative controls. The12 direct storage rejection shapes
are unchanged. Correct layout metadata is a prerequisite for future storage support, not admission
of a physical LLVM vector as stored CUDA BF16 data.

## Research255: physical record storage candidates

With254's metadata correction in place, fresh LLVM14.0.6 DataLayout and GPU transport controls
qualify `<2 x i16>` as BF2 storage and `[3 x i16]`/`[4 x i16]` as BF3/BF4 storage at guarded record-array
and local-reference boundaries. Their wrapped records are12/4,10/2,12/2, with value offsets4/2/2
and suffix offsets8/8/10. Three-record holders plus a uint16 tail are40/4,32/2,38/2 with tails36/30/36.
The BF2 scalar-array alternative still misaligns embedded fields; BF3/BF4 register vectors still
inflate record layout. Allocation alignment cannot repair either type-level mismatch.

All9 source NVRTC O3/raw NVVM O0/O3 controls preserve603980928 bytes. Each width tests all65536
encodings at both runtime permutation flags, with complete coverage for every source/destination
lane and record. The byte oracle includes global destination vector writes, local inout roundtrips,
source records, prefix/suffix/tail guards and all padding/sentinels. There is no arithmetic or numeric
conversion, so every NaN payload and signed-zero encoding is preserved exactly.

Actual PTX retains global and local memory accesses and helper calls. NVRTC scalarizes some local
helper parameters while keeping caller local loads. Raw helpers use generic pointers and volatile
loads/stores. Source and raw register-helper ABIs differ for component structs; this does not
qualify cross-module CUDA helper interoperability. Local-frame extent is not a stored-record ABI
measurement or a kernel-speed result.

No production support changes in255: all6 direct source storage controls still reject the typed
Ptr<H> helper parameter. A later bounded local/reference slice should preserve the existing
NVVMTypeUse role checks and representation caches, use native width2 storage and component arrays
for widths3/4, and convert explicitly at the existing memory/value boundary. Keep aggregate layout
validation strict. Broader device-pointer/resource/parameter-group admission, FP8 aggregates and
matrix orientation require their own contracts. Read
[report255](../../issue-nvvm-backend/report.slice-255-bf16-physical-storage.md) and
[semantic255](../../issue-nvvm-backend/semantic-evidence.slice-255.json).

## Bare local vector storage (slice256)

Internal mutable BF2/BF3/BF4 references and local variables use an explicit physical Storage role.
BF2 remains `<2 x i16>` with alignment4; BF3/BF4 use `[3 x i16]`/`[4 x i16]` with alignment2.
Register and internal by-value helper roles retain `<N x i16>`. Whole-value loads/stores convert
symmetrically by extracting and constructing lane bits, without numeric conversion or NaN changes.

The exact one-operand Generic Ptr/OutParam/BorrowInOutParam classifier owns this local admission.
Recursive helper/copyable/aggregate classifiers remain closed for BF vectors. Storage cache entries
cannot authorize another role: admission precedes lookup, and helper pointer cache keys include the
pointee use. CUDA-exported BF vector references remain rejected. Research255's qualified physical
record layouts do not yet imply production record, array, resource or device-pointer support.

Validation uses a432-word three-mode fixture and exhaustive all-encoding local/reference controls,
including reverse cache visitation order. Optimized PTX can scalarize read-only helper arguments or
raise local frame alignment; caller memory operations and full byte output are checked separately.
Frame extents and external helper ABI remain distinct from this local storage contract. See
[plan256](../../issue-nvvm-backend/plan.slice-256-bf16-local-vectors.md) and
[report256](../../issue-nvvm-backend/report.slice-256-bf16-local-vectors.md) for current acceptance.

## Flat local BF16 record fields (slice259)

A local record can contain integer scalars and BF16 scalar/vector2/3/4 fields, with at least one BF16
field. Exact Generic one-operand Ptr/OutParam/BorrowInOutParam roots select the existing Storage role.
The same canonical IRStructType and keyed fields remain authoritative. BF2 storage is native
`<2 x i16>`; BF3/BF4 fields use component arrays and reuse256's leaf vector/array conversion when
loaded or stored. Whole-record make/extract/load/store/phi and by-value helper roles remain gated.

IRBuilder::emitFieldAddress intentionally produces an explicit pointer with access, address-space
and layout operands. The emitter qualifies a BF vector field through its actual FieldAddress and
admitted local-record root; pointee type alone cannot confer this permission. The local flag does
not propagate through nested records, which the finite root classifier rejects. Default aggregate,
resource, shared, device and parameter-group classifications remain unchanged. Exported CUDA record
references and readonly references remain unqualified.

The existing aggregate-layout walk receives an explicit local permission and composes the qualified
leaves, comparing local record size/alignment against CUDA layout before allocation. Guarded
`{uint16, BF, uint16}` scalar/2/3/4 records have sizes6/12/10/12 and alignments2/4/2/2; value/suffix
offsets are2/4,4/8,2/8,2/10. Mixed `{uint8,BF16,uint32,uint64,BF3,uint16}` is24/8 with offsets
0/2/4/8/16/22. Provider storage/value caches remain separate, and role checks precede lookup.

Both type visitation orders preserve all65536 raw encodings per scalar/vector lane under both
control flags, integer guards and untouched buffer bytes. O3 can scalarize read helper arguments;
caller local accesses are inspected separately. Original255 device Ptr<H> controls and the frozen
FP8 record A result remain unsupported. This local contract does not establish external helper ABI,
resource storage, record arrays or material runtime behavior. See
[report259](../../issue-nvvm-backend/report.slice-259-bf16-local-records.md).
