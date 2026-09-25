# Qualify BF16 vector transport and component conversions

## Motivation

The accepted scalar BF16 implementation leaves frozen `hlsl-intrinsic/scalar-bf16.slang#cuda-1`
unresolved. Its original source contains:

```slang
uint64_t bitsBF16 = 0x4080404040003F80;
let vecBF16 = bit_cast<vector<BFloat16, 4>>(bitsBF16);
let v4f = float4(vecBF16);
vector<BFloat16, 4> other = vector<BFloat16, 4>(
    BFloat16(0.5), BFloat16(0.5), BFloat16(1.0), BFloat16(1.0));
BFloat16 dotResult = dot(vecBF16, other);
```

Both direct modes now reject `helper function parameter: vector<BFloat16,4>`. Vector values and
Float32 casts are the next concrete producer-to-consumer boundary. Source-ordered dot and exact
integer construction remain separate research238 contracts. Material6 already compiled/assembled
at239; absent bindings, textures/LUTs, inputs and expected outputs prevent runtime work. This is an
explicit research cadence override, with no speculative material changes.

## Proposed solution

Qualify widths2/3/4 using source NVRTC controls and matched raw LLVM physical candidates, without
changing production code, ABI38, binaries, corpus or oracles. Preserve semantic BF16 and use physical
`<N x i16>` only in qualified value/by-value internal helper roles. Reuse established scalar
conversion semantics per lane. Keep new local pointers/storage out of the next implementation slice:
measured component arrays are candidate storage representations, but BF2 alignment and BF4's layout
producer disagreement need separate role-specific work. Do not widen the ordinary numeric classifier.

The bounded next slice should cover `makeVector`, `MakeVectorFromScalar`, extraction, input-dependent helper branch/phi selection, by-value helper arguments/results and matching-lane-count BF16/Float32 casts.
Existing `lowerBitCast` already turns tested source uint32/BF2, uint64/BF4 and ushort3/BF3 transport
into canonical scalar transport and construction. It needs no special reconstruction. Preserve
all unsupported arithmetic/integer/Half/double/storage roles with focused negatives, then perform
a full checkpoint for shared type/provider changes. Dot remains unresolved even if this slice passes.

## Change summary

Only completed plan, this report, compact semantic evidence, a dedicated
[design contract](../docs/design/nvvm-bf16-vector-contract.md) and STATUS change. Raw artifacts are
under ignored `build/nvvm-loop/slice-240-bf16-vectors/`.

| Fresh evidence        | Actual result                                                                                      |
| --------------------- | -------------------------------------------------------------------------------------------------- |
| Runtime smoke         | 4/4 correct                                                                                        |
| Original frozen BF16  | 3 exact cells; NVRTC correct, direct O0/O3 unchanged preflight                                     |
| Source controls       | 3 CUDA-source emissions, 3 NVRTC O3 compilations/assemblies/runs; 6 direct rejections              |
| Raw LLVM controls     | 6 compilations/assemblies/runs: widths2/3/4 × O0/O3                                                |
| Exhaustive domain     | 73,190 records per run; every record appears in every lane                                         |
| Complete output audit | 13,613,340 active + 18,004,749 preserved = 31,618,089 words; zero mismatches                       |
| Layout                | LLVM14 query for 6 types; 7 CUDA compile-time assertions, including wrapped-field offsets          |
| Preservation          | All37 sources,12 artifacts,558 inputs match239 before/after; all117 indexed238 artifacts unchanged |

The input projection copies immutable238 `convert.input.bin`/`convert.expected.bin`, preserving all
65,536 BF16 encodings and 7,654 additional Float32 boundary patterns. Lane `j` uses accepted record
`(i + 19711*j) % 73190`, a permutation of the entire domain. Each record is48 words; active positions
are explicit in `controls/input-summary.json`. Width2 has14 outputs, width3 has21, width4 has27.
Every input, inactive lane, header and remaining sentinel word is checked. Narrowing NaNs use a
classification predicate; observed0x7fff is not a payload contract. SM80 widening checks exact bits.

Full/implementation239 and targeted233 remain the accepted checkpoints, implementation cadence0.
Only3 of1686 corpus cells are fresh;1683 are inherited. Full239 remains1643 correct/43 unresolved,
with14 resolved histories. Units482 plus one pre-existing Windows skip, toolkit18, runner contracts6,
scalar fixture3 and material compile/assembly6 are inherited from239, not rerun. No support delta,
new corpus identities, build or full-checkpoint reset. Base946f3f3b1dfb6ce2e468afdd707aced032843e2e,
compiler a6cd5bd057defd8fc896ebd75813d8b7e5737fe33a095e20840fc73b72233412 and provider
cefb3cd3cb44fb0d2c6a201f210ea3c98e1913c2fcac554ea5ad912d6afbcfd7 remain unchanged.

## Concepts and vocabulary

- **Value vector:** canonical semantic lanes, physically represented by LLVM i16 lanes; not a promise
  about padding, external memory offsets, or CUDA-call interoperability.
- **Allocation size:** the stride LLVM reserves for a type, which can exceed its value/store bytes.
- **Role:** `NVVMTypeUse` separates values, helper signatures and storage representations. Qualification
  of one role does not recursively admit other roles.
- **Derived projection:** a newly stored lane arrangement of immutable accepted input/expected records;
  it is research data and adds no corpus identities.

## Process report

The production helper/fallback inventory is empty. Generated constructors, choose/replace helpers,
raw LLVM modules, host layout query and independent audit survive only as research evidence. No
production fallback, changed expected output or diagnostic shift is counted as support.

The core library intentionally produces `Vec(BFloat16Type,N)`, with `makeVector`,
`MakeVectorFromScalar`, typed `swizzle` and `floatCast`. The noinline `choose` signature has a BF
vector result, two BF vector arguments and Bool; the helper body retains input-dependent selection.
`replace` has `BorrowInOutParam(Vec(BFloat16Type,N))` and a BF vector value. Their producer shape
is valid. Direct preflight rejects the generated helper result type before provider use; all six
rejections are retained, distinct from the original frozen parameter diagnostic. No producer repair
is appropriate for these value shapes.

`BitCastLoweringContext::processBitCast` compares natural size, then `readObject` constructs the
result from source components. Whole-vector bitcasts in early IR become scalar UInt16/BF16 casts
with bit extraction/reassembly in final source and direct-preflight IR. The same-size ushort3
case is valid despite the LLVM value vector's padded allocation; source values contain48 meaningful
bits. Raw i48 bitcasts confirm register feasibility but are not necessary new Slang operations.

`CUDASourceEmitter::tryEmitInstExprImpl` maps construction and casts to prelude constructors and
component casts. Source NVRTC retains `choose`/`replace` calls, dynamic `selp` and local loads/stores.
Raw LLVM uses `<N x i16>` helper parameters/results and `phi`, `[N x i16]` local storage and noinline
pointer helper calls, with volatile lane stores/loads. O0/O3 PTX proves those paths survive. The
complete domain passes all three source and six raw launches. Branch/phi helper selection is qualified; no separate canonical vector `Select` catalog operation
is claimed. Any such implementation admission needs its own focused test. These latter six are physical
libNVVM controls, not production direct-Slang vector successes.

The source BF3 helper ABI is6 bytes/alignment2; raw LLVM BF3's helper ABI is8/alignment8. Each caller
and callee agree internally. No external helper ABI equivalence is inferred. LLVM14's exact provider
DataLayout reports BF2 vector store/allocation/alignment4/4/4, BF3 6/8/8 and BF4 8/8/8. CUDA prelude
BF2 is native4/4; custom BF3 and BF4 are6/2 and8/2. Arrays have size2N/alignment2. The raw BF2 local
array uses explicit alloca alignment4, which does not alter array type alignment in aggregates.
Wrapped-field assertions and LLVM queries show why those distinctions affect field offsets/stride.

The representation audit therefore rejects whole-register-vector storage as a blanket policy. The
BF4 discrepancy is also upstream: `CUDALayoutRules::calcSizeAndAlignment` and its generic vector
rule in `slang-ir-layout.cpp` special-case Half, not BF16. AST `_createTypeLayout` represents BF16
scalar layout as UInt16, passes `BaseType::Void` as its non-basic vector element tag, and
`CUDALayoutRulesImpl::GetVectorLayout` takes the generic width4 alignment8 rule. Actual prelude
BF4 alignment is2. A future storage slice must repair those layout producers before adding external
BF4 storage; it must not compensate by bypassing downstream layout checks. Scalar-array storage and
existing component conversion machinery are promising but not yet a general BF16 storage contract.

Reuse `NVVMTypeLoweringContext::lowerType` role caches, `_getNVVMSemanticType`, semantic catalog
validation and `_getSemanticLLVMType` for explicit value/helper admission. Reuse the existing scalar
`BFloat16Convert` operation component-wise and `_emitNVVMSequentialElementExtract`/builder vector
construction. `_emitNVVMStructuredBufferStorageConversion` and compact vector load/store paths are
future storage reuse points; their existing predicates and Half chunk representation must not be
blindly widened. New local-pointer/storage admission is deliberately excluded from the next slice.

Two layout setup failures remain under `attempts/`: the first used an incorrect BF feature macro;
the second found the NVCC prelude's `SLANG_MAKE_VECTOR` definition gated on RTC or Half. The final
isolated assertion compile enables Half and BF16 and uses the actual prelude definitions. No
production fix or NVCC BF-only support claim follows. All source NVRTC controls already passed. A proof checker initially required explicit `ld.local`
spelling in raw O0 PTX; those accesses use generic volatile instructions through a local `%SP`
pointer. The corrected checker follows that address provenance. No control or output changed.

The worker independently re-derived narrowing expectations with upper/lower-word ties-to-even,
checked every derived input/expected record, every returned word, original frozen mode outcomes,
and all before/after identities. Accepted238's117 indexed artifacts remain immutable. The parent
also independently checked the complete projection/output buffers before checkout release.
Native Ubuntu24.04/L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14/ABI38 and
matching RelWithDebInfo were unchanged. Suites ran sequentially under30-minute bounds, with no
system change, GPU loss, reboot, worker commit or push. Research stops here at the bounded handoff.

Parent acceptance verifies252 unique evidence references,132 indexed raw artifacts,12 primary
source hashes, all baseline identities and all31,618,089 returned words. Research240 is accepted
without changing full239, targeted233 or implementation cadence zero.
