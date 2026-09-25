# Slice256: local BF16 vector storage and mutable references

Status: accepted full checkpoint, provider ABI41. Delegation was unavailable at the agent limit;
one parent owned implementation and separate oracle/mechanical acceptance. No fresh independent-agent
review is claimed. No push or system/driver change occurred.

## Motivation

Consider this complete data path inside a compute kernel:

```slang
[noinline]
vector<BFloat16, 3> replace(
    inout vector<BFloat16, 3> destination,
    vector<BFloat16, 3> value)
{
    let previous = destination;
    destination = value;
    return previous;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let x = bit_cast<BFloat16>(uint16_t(tid.x));
    vector<BFloat16, 3> local = vector<BFloat16, 3>(x);
    let previous = replace(local, vector<BFloat16, 3>(x));
    outputBuffer[tid.x] = uint(bit_cast<uint16_t>(local.x));
}
```

The canonical vector and by-value helper already work, but the mutable reference is rejected.
The new regression covers widths 2/3/4, differing lanes, replacement results and out parameters.
Before the change, the NVRTC FileCheck test passes and both NVVM modes reject the first helper
parameter as `BorrowInOutParam<vector<BFloat16, 2>>`. Its source, input and independent oracle remain
unchanged afterward. The full output audit compares integer values, supplementing FileCheck.

Accepted255 qualified the physical representation. This slice admits only bare local vectors and
internal mutable references, taking the bounded next step without claiming record/resource support.
It is capability work; rolling 252/254/256 still includes252's measured material compile-time slice.
Material bindings/textures/LUT/input/output contracts remain unavailable.

## Proposed solution

Keep canonical `Vec(BFloat16Type,N)` as the semantic source of truth. Select native `<2 x i16>`
for width 2 local storage and `[N x i16]` component arrays for widths 3/4. Preserve `<N x i16>`
register and internal by-value helper representations. Allocation and reference-pointee lowering
select the existing Storage role; whole-value loads/stores convert between physical arrays and
register vectors without numeric conversion. Width2 is already identical in both roles.

Do not widen the recursive helper/copyable/aggregate classifiers. Their users include resource,
parameter-group, shared and external pointer contracts that this local capability does not prove.
CUDA-exported BF16 vector references remain explicit preflight failures.

## Change summary

- `slang-emit-nvvm-type-lowering.{h,cpp}` admits the exact one-operand Generic local/mutable helper
  pointer family, adds explicit BF vector Storage support and selects the qualified physical type.
  Existing storage, value and helper-pointer representation maps remain separate.
- `slang-emit-nvvm.cpp` validates local CUDA size/alignment, selects local-memory alignment, performs
  symmetric register/storage conversion and rejects CUDA-exported BF vector reference parameters.
- `unit-test-nvvm-emitter.cpp` checks both local allocations, their physical types/alignment and
  conversions at each width. The former inout BF2 rejection migrates to positive coverage, with an
  exported-reference rejection replacing that negative subcase. Existing aggregate/resource/cast
  negatives remain intact.
- `nvvm-bf16-local-vectors.slang` contributes432 independently expected raw-bit outputs at each of
  three modes. The discovery manifest gains one source; frozen 452 and every old fixture remain intact.
- Plan, compact validation, census results, design contract and STATUS carry the acceptance record.

## Concepts and vocabulary

**Register representation** is the first-class LLVM vector used for BF16 values and internal
by-value helper parameters/results. **Storage representation** is the physical pointee allocated
for a local or passed by mutable reference. They represent the same canonical Slang vector.
**Role cache** records a provider type for a particular use without replacing canonical IR identity.
BF3/BF4 component arrays have2-byte alignment even though LLVM register vectors prefer8-byte
allocation. An optimized PTX stack frame can have stronger alignment without changing that contract.

## Process report

The producer is correct. Source checking/lowering builds canonical BF vectors; mutable parameter
lowering produces one-operand `BorrowInOutParamType`/`OutParamType`, and local variables produce
one-operand Generic `PtrType`. `asNVVMSupportedLocalHelperValuePointerType` previously rejected this
valid pointee. Adding only the exact vector family preserves the distinct four-operand user-pointer,
read-only-reference, recursive aggregate and resource gates. No syntax reconstruction, substitute
semantic type or new equivalence relation is introduced.

`NVVMTypeInfo::supports` checks the use before cache lookup. Explicit Storage admission therefore
cannot authorize ParameterGroupStorage or StructuredBufferStorage through a previously cached type.
`NVVMTypeLoweringContext::lowerType` selects component arrays only for BF3/BF4 Storage. The existing
helper-pointer branch lowers those pointees using Storage, and `_lowerPointerType` already includes
the pointee use in its cache key. The allocation path selects the same role. By-value function
parameters/results keep their register type. The positive fake-provider test checks physical local
handles; real-provider tests exercise the caller/callee boundary and both lowering orders.

The local preflight checks CUDA size2*N and alignment 4 forBF2 or2 forBF3/BF4. It deliberately does
not call the ordinary helper leaf's CUDA-versus-LLVM-vector layout proof: that proof concerns a
same-representation ABI, whereas this local path has an explicit component-array representation.
Research255 already measured its exact LLVM allocation and CUDA layout. Aggregate validation remains
unchanged rather than gaining a bypass for BF leaves.

The helper/branch inventory is:

- `_getNVVMLocalBFloat16VectorPointer` survives. It combines the existing exact pointer classifier
  with canonical BF vector recognition, so the memory conversion does not depend on load-result
  type alone. `_validatePointerValue` still proves availability and exact pointee identity.
- `_getNVVMBFloat16VectorStorageAlignment` survives. Its qualified native/component rule is used
  consistently for local layout validation, allocation, load and store; it does not modify the
  recursive executable-value alignment predicate.
- `_emitNVVMBFloat16LocalStorageConversion` survives. It extracts every array/vector lane and
  reconstructs the opposite representation using existing generic builder operations. BF2 returns
  the original handle because both roles already share one physical type. The existing sequential
  extraction helper is reused. The structured-buffer converter cannot be called here: its recursive
  admissibility and Boolean rules describe a different boundary.
- Explicit Storage/pointer-cache, local allocation/load/store and exported-reference branches
  survive. Each owns an actual representation or ABI boundary reached by the regression; none is
  a fallback for malformed IR. Recursive classifiers and unrelated memory consumers stay unchanged.

Removing local pointer admission reproduces the before helper-parameter rejection. Omitting the
storage-role selection would make BF3/BF4 helper pointees disagree with their allocation; omitting
either memory conversion would pass an array where the provider expects a vector, or vice versa.
The unit checks these physical types and both conversions; exhaustive GPU output checks semantics.
No alternate helper or consumer patch was retained to conceal a bad producer.

The first new fake-provider test attempted a dynamic BF16 bitcast. Its legacy type classifier does
not classify semantic BF16 operation results as physical integer lanes, so fake vector construction
failed. The structural test now starts with a canonical BF16 constant; the real-provider regression
and exhaustive controls retain dynamic data and exact encodings. No production code or output oracle
was changed to satisfy that fake limitation. The failed attempt remains in raw evidence.

Nine initial exhaustive source controls cover 131072 packets per width: all 65536 encodings under
both permutation flags. Every old/replaced/copied output lane covers all encodings, including signed
zeros, subnormals, infinities and every NaN payload. Complete buffers, including input/header/gaps,
are compared against an independently reconstructed byte oracle:150996096 bytes/37749024 words.
The fixture adds 1296 checked words. Reversing helper visitation order provides a separate cache
preservation control using identical input/expected bytes; its final results are in compact validation.

PTX retains actual reference loads/stores in replace and initialize. Some O3 readLocal helpers accept
scalarized values while callers retain local loads. BF4 NVVM O3 raises frame alignment to 8; NVRTC
BF2 uses hidden return storage. Neither observation changes the component storage ABI or establishes
external helper compatibility. Existing six research 255 Ptr<H> compilations retain exact E52017
rejections. No raw LLVM candidate is counted as a newly passing production source workload.

The second visitation order also passes all 9 controls with identical input/expected buffers. Together
both orders check 301992192 bytes/75498048 words; PTX declarations confirm storage-first versus
value-first helper visitation in both direct optimization modes.

Full frozen 1356 retains 1347 correct and 9 unresolved. Discovery351 preserves every 348 old outcome
and adds 3 correct cells, reaching321 correct and 30 unresolved. Combined1707/1668correct/39unresolved
preserves 1704 old five-field outcomes and 18 resolved histories, with no missing/duplicate cells.
All 1061 prior unit IDs and 1129 semantic IDs/outcomes remain exact; the new structural unit brings
units to 1049 pass/13 skip. Semantics1052 pass/77 skip, runtime 4, focused 26, toolkit 18, contracts 6 and
material 6 compile/assembly checks pass. No runtime speedup or material runtime claim is made.

Final identity covers 133 source files, 12 artifacts and 565 runtime inputs, including all 564 unchanged
prior inputs. Compiler SHA256 is 1fc2311e9e0c332f2bff54d65dedb1a225218f740042a2c23c5210e76245c85f;
provider remains 5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab on ABI41.
See `runtime-validation.slice-256.json` for exact identities, commands and preservation records.
Raw evidence lives under `build/nvvm-loop/slice-256-before` and `slice-256-after`.

Evidence closure verifies 3753 indexed raw artifacts, 133 final source snapshots and 347 final compact
references. The final read-only closure includes its two audit references without rewriting the
accepted audit result.
