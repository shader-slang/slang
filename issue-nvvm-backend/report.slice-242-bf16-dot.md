# Preserve CUDA BF16 dot rounding in direct NVVM

## Motivation

The frozen `hlsl-intrinsic/scalar-bf16.slang#cuda-1` passes through NVRTC but both direct modes
stop at `GenericAsm assembly=_slang_vector_dot, signature=BFloat16(vector<BFloat16,4>, vector<BFloat16,4>)`.
Consider its relevant expression:

```slang
let values = bit_cast<vector<BFloat16, 4>>(uint64_t(0x4080404040003f80));
let weights = vector<BFloat16, 4>(
    BFloat16(0.5), BFloat16(0.5), BFloat16(1.0), BFloat16(1.0));
BFloat16 result = dot(values, weights); // 8.5
```

Accepted scalar239/vector241 already transport these values correctly. Dot ranks ahead of exact
integer construction and storage because it is the concrete next boundary in two frozen failures
and research238 establishes the source contract. Storage additionally needs layout-producer repair
and its own ABI evidence. The material entries were reconsidered: six compile/assembly cells are
supported, but bindings, textures/LUTs, inputs and an expected-output contract remain absent. This
slice explicitly continues support/correctness work; it does not claim material runtime or speed.

## Proposed solution

Add one dedicated BF16 dot semantic, qualified for scalar BF16 result and two equal BF16 vector
operands of widths2/3/4. Canonical helper planning uses the existing exact spelling/signature map.
The real provider follows the CUDA prelude's positive-zero accumulator, lane order and separately
rounded products/sums. On targetSM80, two BF16 FMA instructions per lane implement the installed
CUDA header's multiplication/addition recipes. ABI40 explicitly negotiates the expanded contract.
Generic floating/integer arithmetic classifiers, type roles and frontend/library producers remain
unchanged. The exact raw LLVM recipe had to pass an isolated prototype before production promotion.

## Change summary

- The builder API names `BFLOAT16_DOT` and advances ABI39 to40. The shared semantic catalog requires
  exact BF16 format/width/result and equal operand lanes2..4.
- The canonical GenericAsm spelling table maps `_slang_vector_dot` to that operation. Existing
  canonical-helper validation and operation planning continue to own the producer boundary.
- `_emitBFloat16Dot` in the LLVM provider extracts lanes, starts i16 zero and emits separate
  BF16 multiplication/addition FMA calls in order.
- `nvvmIRBuilderBFloat16DotContract` checks actual provider support/emission, malformed descriptors,
  physically mismatched Half values and both serialization dialects. One registered discovery
  fixture covers dynamic dot2/3/4 and independently derived finite/zero/subnormal/inf/NaN results.
- Plan, design, census/addition, compact validation and STATUS retain the bounded decision, full
  checkpoint and complete prior failure histories. Raw generated evidence stays under ignored build.

The full checkpoint is accepted after independent parent review; latest accepted full is242.

| Final-source gate                           | Actual result                                                                                  |
| ------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| Runtime smoke                               | 4/4 correct                                                                                    |
| New dot fixture / ordinary exported helpers | 3/3 each                                                                                       |
| Prototype                                   | 3 sourceNVRTC +6 rawLLVM launches;9 SM80 assemblies;97,929 words checked                       |
| Production replay                           | 3 sourceNVRTC +6 directNVVM launches;9 SM80 assemblies;97,929 words checked                    |
| Independent full-buffer total               | 18 launches /195,858 words:12,240 active and183,618 preserved; zero mismatches                 |
| Units                                       | 484 passed; one pre-existing Windows skip                                                      |
| Toolkit / runner contracts                  | 18/18 and6/6                                                                                   |
| Frozen checkpoint                           | 452 identities /1,356 cells;1,345 correct and11 retained failures                              |
| Discovery checkpoint                        | 111 old identities /333 cells plus one new fixture /3cells;306 correct and30 retained failures |
| Material support                            | 6/6 fresh compile/assembly cells; no runtime/performance claim                                 |
| Preservation                                | All1,646 old correct cells,559 old input hashes and14 old resolved histories retained          |

Total1,692 fresh corpus cells:1,651 correct and41 unresolved. Three correct cells are additions;
two old failures resolve. Exact comparison of classification, return_code, complete execution_counts,
diagnostic and canonical_shape finds only the original frozen BF16 O0/O3 preflight→correct changes.
Their complete prior records, including earlier scalar/helper/dot diagnostic histories, move into
the16-entry resolved ledger. All41 remaining failure histories stay visible; there are no missing,
duplicate, ignored new or inherited final-source corpus cells. Discovery's333 old cells match exactly.

Tested source base `d6c26eb4cf5960ac07feff8158d15163cc2757fc`, ABI40,40 source paths,12 artifacts
and560 runtime-input hashes are captured before every final gate and afterward. All559 old inputs
are byte-identical. Compiler executable SHA256 `79f46ef0dba116bdfdce8a03b40f7d3bb2c7b481529b452b483d650cf0958e43`; provider SHA256 `c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773`.
The compiler executable is unchanged because compiler implementation lives in the rebuilt shared
library, whose final hash is included among the12 artifacts. Native Ubuntu24.04/L4SM89 driver580.126.09,
targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14 and matching RelWithDebInfo remain unchanged.
Four CPU workers maximum total, sequential GPU suites and30-minute bounds were used.
Frozen/discovery exit2 records remaining measured gaps, not acceptance by exit code alone.

See [compact validation](runtime-validation.slice-242.json), [plan](plan.slice-242-bf16-dot.md),
[frozen census](census.slice-242.tsv) and [discovery census](discovery-census.slice-242.tsv).
Worker/parent independent numerical oracles agree on both stages. Raw controls, complete buffers,
sources/IR/PTX, commands, logs and identities are under `build/nvvm-loop/slice-242-before` and
`slice-242-after`; accepted238/240/241 indexed artifacts remain unchanged. No GPU loss, system or
driver change, reboot, worker commit or push occurred.

## Concepts and vocabulary

- **Semantic BF16:** the canonical Slang BFloat16Type and distinct provider format, not IEEE Half
  or an integer despite its physical16-bit representation.
- **Rounding boundary:** each source product and accumulator addition rounds to BF16 independently.
  A single fused product/add skips a required boundary.
- **Canonical helper:** the target-selected one-block GenericAsm body and its checked function
  signature, recognized by the existing helper validator rather than arbitrary source parsing.
- **Full-buffer oracle:** independently derived output values plus exact unchanged header, input
  and sentinel words; NaNs use classification rather than payload equality.

## Process report

The complete production helper/branch inventory has five entries: the dedicated ABI operation,
one canonical spelling-table row, one exact catalog family, `_emitBFloat16Dot`, and its provider
dispatch. All are necessary for `nvvm-bf16-dot.slang` and the real-provider dot contract unit.
No fallback, alternate type representation, generic numeric admission or syntax reconstruction was
added. The plan records each admission and its failing test. Provider negatives reject each wrong
operand role, lane0/1/5/mismatch, BF width32, Half/Float32/signed/unsignedinteger formats, vector or
wrong-format results, arity0/1/3, null operand descriptors and physicalHalf mismatch. Existing
scalar/vector tests keep general arithmetic, comparison, conversion, Select and storage boundaries
closed. Existing explicit exported BF-vector negatives and ordinary integer exports remain relevant.

The input-shape audit finds valid producer output. `hlsl.meta.slang::dot` intentionally chooses
CUDA `_slang_vector_dot`. `_isCanonicalNVVMIntrinsicValueHelper` confirms that this is its entire
executable body. `_resolveNVVMSemanticValueOperation` obtains exact checked types through
`_getNVVMSemanticType`; the shared catalog alone qualifies the BF16 overload. Existing provider
operand validation requires matching physical i16 vectors before dispatch. Nothing is malformed
upstream, so frontend/library changes would be wrong. Target arithmetic emission owns this recipe.

Consider the case that rules out a tempting fused implementation:

```slang
let a = vector<BFloat16, 2>(BFloat16(-1.0), BFloat16(1.0078125));
let b = vector<BFloat16, 2>(BFloat16(1.015625), BFloat16(1.0078125));
BFloat16 result = dot(a, b);
```

The first product/add gives -1.015625. The exact second product is1.01568603515625, but source
multiplication rounds it to BF16 1.015625 before adding. The result is positive zero. Fusing the
second product with the accumulator produces2^-14/BFbits0x3880, as does Float32 accumulation.
Appending zero lanes preserves the counterexample for widths3/4. The original research238 record2
also separates FP32 accumulation, while record1 exposes accumulation order for widths3/4.

The installed CUDA SM80 `__hmul` recipe uses `fma.rn.bf16(a,b,-0)`, preserving signed multiplication
zero; `__hadd` uses `fma.rn.bf16(product,1,sum)`. The provider uses precisely these operations and
constants. Each inline-assembly call forms a distinct rounding boundary. Native BF16 add/mul require
SM90 and were never introduced; native LLVM bfloat had already failed research238's parser gate.
Float32 accumulation or unrestricted reduction would violate the demonstrated source contract.

Before production edits, the isolated publicNVRTC and rawLLVM O0/O3 controls passed all three
widths:9 launches,9 SM80 assemblies,680 records each,97,929 full words (6,120 active/91,809 preserved).
All679 original record payloads are byte-identical; the reversed cancellation case above is the one
appended record, with header680. Worker integer/rational expectations and an independent parent
nearest-neighbor rational oracle agree. An initial generated constructor missed its closing angle
bracket and rejected during parsing; that attempted source/log/script is retained separately. It
never reached GPU execution and did not change the semantic recipe. Accepted research238/240/241
raw evidence remains immutable.

The readable fixture then passed NVRTC with the accepted binaries and rejected both direct modes
at the exact dot2 GenericAsm boundary. Its input/oracle/source hash was frozen before implementation.
The implementation uses the same recipe that passed the prototype; final source IR, real-provider
both-dialect tests, SM80 assemblies and complete public GPU buffers establish the production path.
General BF16 arithmetic/comparison, integer/Half/double conversion, explicit vector Select, storage,
resources/aggregates/globals/parameter groups and external CUDA helper interoperability remain
excluded. BF3 CUDA6/2 versus LLVM8/8 and BF4 layout producer repair stay separate work.

The original frozen workload now completes at both direct optimization levels; no next blocker is
exposed in that source. Remaining measured independent boundaries are retained in the failure ledger
(for example dynamic-dispatch's FP8-containing helper result A and texture GetDimensions GenericAsm).
They were not investigated in this slice. Exact BF16 integer construction is separately qualified by
research238's double-rounding counterexample but remains unimplemented, and vector storage still
requires the documented layout producer repair. The next slice must re-rank these against actual
remaining corpus gaps and reconsider the absent material runtime contract.

Independent parent acceptance verifies all final hashes, exact five-field corpus outcomes and histories,
707 compact and1,341 total evidence references before its own seven references, and all129 current
indexed artifacts. The parent independently rebuilt every prototype/production expected buffer and
all48 fixture expectations; no output mismatch or unexpected corpus delta remains.
