# Support BF16 register vectors and internal helper values

## Motivation

Frozen `hlsl-intrinsic/scalar-bf16.slang#cuda-1` stops at a BF16 vector helper signature after the scalar239 implementation. Consider this reduced use of the same canonical value boundary:

```slang
[noinline]
vector<BFloat16, 3> choose(
    vector<BFloat16, 3> a, vector<BFloat16, 3> b, bool flag)
{
    if (flag)
        return a;
    return b;
}

let made = vector<BFloat16, 3>(b0, b1, b2);
let splat = vector<BFloat16, 3>(b0);
let selected = choose(made, splat, inputFlag);
let expanded = float3(selected);
let narrowed = vector<BFloat16, 3>(float3(f0, f1, f2));
```

Research240 qualified these canonical vector values against every BF16 encoding and Float32 boundary records. The current slice implements that value contract. It does not resolve the original frozen workload's separate source-ordered dot boundary. All six material cells already compile/assemble; absent binding, texture/LUT, input and expected-output semantics continue to prevent material runtime or performance claims. This is the recorded support/correctness cadence override.

## Proposed solution

Admit exact widths2/3/4 through explicit register and internal by-value helper roles, preserving semantic BF16 independently of IEEE Half. Represent those values physically as LLVM i16 vectors. Match BF16/Float32 conversion lanes and reuse the accepted scalar conversion recipe in each lane. Existing canonical bitcast lowering already decomposes source uint32/BF2, uint64/BF4 and ushort3/BF3 transport; no new source-level or provider bitcast representation is needed. Builder ABI39 records the expanded descriptor contract.

Keep recursive numeric/copyable/helper/storage classifiers unchanged. BF3 storage differs from its LLVM vector allocation, and BF4's CUDA layout producers still need a separate correction. No new local pointers, aggregate/resource/global/parameter-group storage, external CUDA helper interoperability, arithmetic/comparison/dot, integer/Half/double conversion or explicit vector Select is claimed.

## Change summary

- `slang-emit-nvvm-type-lowering.{h,cpp}` adds exact BF16/register-vector qualifiers and explicit Value/HelperValue/HelperParameter/HelperResult roles, reusing cached vector lowering.
- `slang-emit-nvvm.cpp` uses that register qualification for construction/extraction, semantic descriptors, helper signatures, phi parameters and value availability.
- The compiler-core semantic catalog validates exact BF16/Float32 lane counts and widths; the provider maps BF16 vectors to i16 lanes and extracts the existing scalar conversion recipe. The builder API revision becomes39.
- Real-provider units cover all widths and malformed/excluded descriptors; frontend-valid negatives preserve BF vector pointer, aggregate and resource rejection.
- One new discovery fixture checks all widths, nonuniform payloads, Float32 boundaries, splats, bit transport, dynamic internal helpers, mixed constructors, swizzles and runtime indexing with independent expectations. Existing identities and oracles are unchanged.

The final full checkpoint is accepted after independent parent review. Full241 is the current accepted checkpoint; implementation cadence resets to zero.

| Final-source gate                       | Actual result                                                                                                 |
| --------------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| Runtime smoke                           | 4/4 correct                                                                                                   |
| New fixture / ordinary exported helpers | 3/3 each                                                                                                      |
| Exhaustive public value projections     | 9 launches, 73,190 records per width/mode; 31,618,089 words checked                                           |
| Separate raw controls                   | 6 launches with original full oracle; 21,078,726 words checked                                                |
| Independent total-buffer checks         | 52,696,815 words; 20,712,770 active and 31,984,045 preserved; zero mismatches                                 |
| Units                                   | 483 passed; one pre-existing Windows skip                                                                     |
| Toolkit / runner contracts              | 18/18 and 6/6                                                                                                 |
| Frozen checkpoint                       | 452 identities / 1,356 cells; 1,343 correct, 13 unchanged unresolved classifications                          |
| Discovery checkpoint                    | 110 old identities / 330 cells plus new fixture / 3 cells; 303 correct, 30 unresolved                         |
| Material support                        | 6/6 fresh compile/assembly cells; no runtime/performance claim                                                |
| Preservation                            | All 1,643 old correct cells, 43 unresolved histories, 14 resolved histories and 558 old input hashes retained |

Total1,689 fresh cells:1,646 correct and43 unresolved; no inherited final-source corpus cells. Three correct cells are additions, zero old cells resolve. The only two outcome-field deltas are frozen BF16 O0/O3: the diagnostic moves from a BF4 helper parameter to `GenericAsm assembly=_slang_vector_dot, signature=BFloat16(vector<BFloat16,4>, vector<BFloat16,4>)`, and canonical shape changes from `helper function parameter` to `GenericAsm`. Classification, return code and complete execution counts are unchanged. Each complete prior failure record is retained in diagnostic history. Discovery's330 old cells match exactly.

Tested source base `3e218a92a6c3b3c2cdc02fdf854f03b9712e6dda`, builderABI39,39 tested-source hashes,12 artifact hashes and559 runtime-input hashes are captured before every gate and after validation. Compiler SHA256 `79f46ef0dba116bdfdce8a03b40f7d3bb2c7b481529b452b483d650cf0958e43`; provider SHA256 `116df24297dddc55b9c8f2f4f45f30e614e3185dafc618b3b67caf6df14a2bfe`. Native Ubuntu24.04/L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14 and matching RelWithDebInfo. Suites used at most four CPU workers total, sequential GPU dispatch and30-minute bounds. Frozen/discovery runner exit2 represents retained measured gaps, not missing execution.

See [the compact validation](runtime-validation.slice-241.json), [plan](plan.slice-241-bf16-vector-values.md), [frozen census](census.slice-241.tsv) and [discovery census](discovery-census.slice-241.tsv). Completed worker and independent parent buffer oracles agree. Parent acceptance verifies703 compact and1,303 total evidence references before adding six parent references, all88 indexed current artifacts, exact corpus/history preservation and52,696,815 replay words. The local commit is authorized; no worker commit or push occurred.

## Concepts and vocabulary

- **Register vector:** canonical semantic lanes represented as LLVM i16 lanes, independent of external storage padding and alignment.
- **Type role:** the `NVVMTypeUse` contract that determines whether a canonical type is admitted as a value, helper parameter/result or storage object.
- **Component conversion:** one scalar narrowing/expansion per vector lane; the semantic descriptor retains BF16 identity even though LLVM transport uses integers.
- **Value-only projection:** research240's immutable inputs and source with its pointer/local helper removed; removed output columns retain their original sentinels.

## Process report

The explicit exported-helper result and parameter rejection branches also survive. Reachable `[CudaDeviceExport]` functions otherwise use the same helper path; provisional BF3 probes emitted eight-byte/alignment-eight visible PTX signatures. Accepted research240 establishes CUDA BF3 as six-byte/alignment-two, so `_validateNVVMHelperTarget` rejects those valid but unqualified ABI roles. Two frontend-valid negatives cover both boundaries; ordinary exported integer helpers retain their existing gate. No producer fix is appropriate. Provisional compiler-hash capture failed before the rebuild and is explicitly unavailable; the retained PTX is investigation evidence, while final tests and identities establish acceptance.

The new helper inventory contains `asNVVMBFloat16VectorType`, `asNVVMRegisterVectorType` and `_emitBFloat16ConvertLane`. All survive; no fallback, custom equivalence, syntax reconstruction or producer repair is added. The first checks the canonical BFloat16Type and literal widths2..4. The second combines it with already-qualified ordinary vectors only at register consumers. The third extracts the existing scalar provider recipe so scalar and vector conversion have one implementation.

In the example above, the core library intentionally builds MakeVector/MakeVectorFromScalar, typed Swizzle, FloatCast and vector helper parameters/results. `_getNVVMVectorConstruction` retains every ordered scalar or vector operand, `_getNVVMSequentialElement` checks the element type/index, and `_getNVVMSemanticType` records BF16 rather than Half. These are valid canonical shapes. The frozen fixture failed on accepted binaries before provider discovery, establishing a missing backend contract rather than malformed producer data. Its NVRTC result already matched the independent oracle.

Helper result/parameter validation now recognizes BF vectors explicitly. Canonical branch joins produce block parameters; their preflight admits BF vectors without changing the executable storage-alignment classifier. The normal dominance/availability check and existing generic phi emission preserve the value. `NVVMTypeInfo::supports` allows only Value/HelperValue/HelperParameter/HelperResult before the role-specific cache is consulted. Existing vector lowering then lowers the canonical BF scalar to i16 and constructs `<N x i16>`. Recursive copyable/numeric/helper classifications remain unchanged, preventing nested aggregate, pointer, global or resource admission.

The shared catalog accepts only FLOAT_CONVERT between BF16 width16 and IEEE Float32 width32 with equal lane counts1..4. `_getSemanticLLVMType` preserves the BF descriptor while choosing i16 lanes. The provider extracts each source lane, calls `_emitBFloat16ConvertLane`, and inserts the result into an undef vector. Narrowing uses `cvt.rn.bf16.f32`, which rounds once including subnormals. Expansion zero-extends and shifts BF bits to the Float32 high word, preserving signaling-NaN payloads on SM80. No LLVM bfloat spelling, IEEE Half arithmetic or intermediate rounding is introduced. The real-provider test rejects wrong physical operand types as well as malformed descriptors, explicit vector Select and unqualified operations.

`BitCastLoweringContext::processBitCast/readObject` remains the producer for equal-size source vector transport. It reconstructs the destination from canonical component-sized pieces. Consequently BF2/uint32, BF4/uint64 and BF3/ushort3 need no i48 semantic type or alternate emitter reconstruction. Construction/extraction reuse is checked with mixed vector operands, reverse swizzle and dynamic indexing as well as ordinary scalar lanes; the final linked IR in `slice-241-after/trace/fixture.log` retains mixed BF3/BF4 MakeVector operands, a four-lane Swizzle and dynamic GetElement. The exhaustive controls retain vector merge parameters for each width in `trace/helper-shapes.json`, while `trace/shape-proof.json` checks the internal PTX signatures and both direct optimization modes.

The BF2 inout negative replaces the former BF2 by-value helper negative because the latter is now intentionally supported. Nested BF3 struct and BF4 structured-buffer negatives ensure the added register qualifier does not leak into storage. Scalar239 negatives and positive coverage remain. Canonical vector Select stays outside the semantic catalog; helper branch/phi is a distinct producer form and does not justify a Select admission.

The exhaustive production replay removes research240's `replace(inout ...)`, mutable vector local and writes29..29+N-1. Those columns must remain the original input sentinel; all other source expressions and all original input/expected files are preserved. A separately stored projected expected buffer changes only those output columns. The independent worker checker re-derives Float32 narrowing via integer ties-to-even arithmetic, reconstructs every lane permutation and checks every output, inactive word, input and header. Narrowing NaNs use classification only; widening requires exact payload bits. Six raw controls retain the original complete oracle and are reported separately from nine production launches.

Raw logs, buffers, source/IR/PTX and identities live in `build/nvvm-loop/slice-241-before` and `slice-241-after`. The full checkpoint is mandatory because shared type/providerABI changes cannot inherit unexecuted final-source corpus outcomes. Parent independently accepted the full checkpoint and owns the local commit; the worker did not commit or push.
