# Admit canonical FP8 scalar transport

## Motivation

Consider the existing frozen folding test:

```slang
RWStructuredBuffer<int> outputBuffer;

int pack8<T:ICoopElement>(T val)
{
    return bit_cast<uint8_t>(val);
}

int pack16<T:ICoopElement>(T val)
{
    return bit_cast<uint16_t>(val);
}

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = pack8(FloatE4M3(1.25f));
    outputBuffer[1] = pack8(FloatE5M2(1.25f));
    outputBuffer[2] = pack16(BFloat16(1.25f));
}
```

The direct backend rejects `pack8`'s FloatE4M3 parameter, although the source needs only a finite
constant and byte transport. The expected full values are58,61,16288; the original CHECK prefix1628
is retained verbatim. Research243 proved scalar/internal-helper i8 transport on SM80; accepted244
repaired shared finite/subnormal literal producers. This slice promotes that bounded transport
contract. It does not require runtime Float32 narrowing or a literal-overflow policy change.

## Proposed solution

Keep FloatE4M3 and FloatE5M2 as distinct canonical semantic descriptors. Represent each scalar as
i8 only in register and internal by-value helper roles. Reuse equal-width BitReinterpret, typed
Select and generic phi/helper emission. Recover finite literal bits through the existing shared
format helper used by IRBuilder::getFloatValue. Nonfinite FP8 literals remain diagnosed because
shared overflow/NaN/infinity semantics and CUDA SATFINITE construction are not yet harmonized.

## Change summary

- Provider API/catalog: ABI41 and two distinct scalar FP8 descriptor kinds; scalar selection and
  equal-width bit transport only, including signed/unsigned8 and cross-format reinterpretation.
- Type lowering/emitter: register/internal helper roles, i8 lowering, checked finite literal bits,
  availability validation and explicit external-helper exclusion. Recursive storage predicates are
  unchanged.
- Provider: descriptors map to scalar LLVM i8; existing operation recipes perform transport.
- Tests: one distinct discovery fixture checks all256 byte encodings in both branch choices for
  both formats and12 finite literals. Provider and preflight units constrain the adjacent boundary.
  A supplemental whole-buffer GPU replay includes both cross-format directions.
- Plan, design contract, manifest/census, report and STATUS preserve acceptance and failure history.

## Concepts and vocabulary

- **Canonical FP8 literal:** IRFloatLit whose checked value has already been rounded by the shared
  producer; recovering its encoding is different from a dynamic FloatCast.
- **Semantic descriptor:** the format/width/lane identity used to qualify an operation before the
  provider receives physical LLVM types.
- **Register/internal by-value role:** a scalar SSA value or helper argument/result. Admission here
  does not promise CUDA memory layout or external helper interoperability.
- **Bit reinterpretation:** preserves all encoding bits, including NaN payloads, without interpreting
  the numeric value. E4M3/E5M2 reinterpretation therefore differs from numeric conversion.

## Process report

Starting identity exactly matches accepted246:117 source/generated/test paths,12 build artifacts
and561 runtime inputs. Before smoke4 passes. The unchanged new1036-word fixture passes NVRTC but
fails both direct modes at `helper function result type: FloatE4M3`. This proves the register/helper
boundary without depending on a later runtime conversion. The finite literals include both signed
zeros, minimum subnormals, E4M3 exponent15, finite maxima and the frozen1.25 values.

The canonical source declaration in core.meta.slang produces FloatE4M3Type or FloatE5M2Type.
`_validateNVVMHelperTarget` checks the direct call signature; `_getNVVMSemanticType` constructs its
format-specific descriptor. `NVVMTypeInfo::supports` admits exactly Value, HelperValue,
HelperParameter and HelperResult. `lowerType` obtains i8. The existing semantic family resolver
permits equal-width BitReinterpret and same-format Select; `_getSemanticLLVMType` retains the
semantic validation while choosing physical i8. Generic helper and phi emission then preserve bits.
No new LLVM arithmetic or conversion recipe is involved.

For finite constants, SCCPContext::evalCast and IRBuilder::getFloatValue are the semantic source of
truth. `_getLoweredNVVMValue` reuses FloatToFloatE4M3/FloatToFloatE5M2 to recover exactly their finite
encoding, then passes its signed8 bit representation to the existing integer-constant builder. The first
focused run caught a contract mismatch for negative bytes: the provider accepts signed in-width
values, so byte0x80 must be passed as-128. `bitCast<int8_t>(bits)` preserves the checked byte, just
as ordinary unsigned integer literal emission normalizes its signed API argument. The failed
attempt and exact118 source snapshots are retained. This is representation at the builder boundary,
not another rounding operation. It neither reconstructs source syntax
nor saturates a nonfinite producer result. `_validateFloatingPointValue` rejects nonfinite FP8
literals before provider discovery. NaN and infinity _encodings transported from bytes_ remain valid
and must survive unchanged; this does not introduce a numeric literal policy.

Self-review inventory: the sole new production helper `isNVVMFloat8Type` classifies the two canonical
IR type opcodes and survives. The new type-role branch, descriptor mapping and finite materialization
branch survive because this backend owns physical representation of valid checked scalars. The
nonfinite and external-export exclusions survive as explicit unqualified contracts. Existing
isNVVMSupportedHelperValueType/copyable/numeric classifiers remain unchanged, preventing accidental
aggregate, pointer, resource, vector or IEEE arithmetic admission. No custom equivalence, structural
walk, fallback, reconstructed syntax or producer repair is introduced.

Final validation and independent parent acceptance passed:

| Gate                                  | Result                                                          |
| ------------------------------------- | --------------------------------------------------------------- |
| Runtime smoke                         | 4/4                                                             |
| Focused registered fixtures           | 6/6                                                             |
| Main NVVM/routing/reporter/math units | 512 passed, one inherited skip                                  |
| Supplemental literal unit             | 1/1; all 511 prior pass identities plus two new units preserved |
| Toolkit / runner contracts            | 18/18 and 6/6                                                   |
| Frozen corpus                         | 1,356 cells; 1,347 correct, nine unchanged unresolved           |
| Discovery corpus                      | 342 cells; 312 correct, 30 unchanged unresolved                 |
| Complex material compile/assembly     | 6/6; no material runtime claim                                  |
| Supplemental transport and phi        | Six GPU launches; 7,722 exact words                             |

All 1,695 old cells retain their outcomes except the two intended folding resolutions. All 1,654
old passes survive. The new distinct discovery fixture adds three correct cells: final 1,698 cells,
1,659 correct and 39 unresolved. All 41 old failure histories remain, with the two resolutions moved
into history alongside the 16 existing resolved histories. No diagnostic, execution, classification
or return-code delta is otherwise permitted. Original texture/matrix failures remain unchanged.

The supplemental phi source uses a loop-carried FP8 value and non-inline signed-byte pack/unpack
helpers, preventing a paired bitcast from simply disappearing before the backend. Final preflight
IR proves both formats have actual FP8 SSA join parameters at O0/O3, as well as the original
selects, branch arms, internal helper values and cross-format reinterpretation. GPU checks compare
all words exactly, including every NaN encoding. The combined final buffer audit checks 10,866 words:
3,108 registered-fixture words, 36 original folding words, 4,647 transport/cross-format words and
3,075 phi/signed-helper words. The original folding output is exactly58,61,16288 and nine zeros in
all three modes. The before NVRTC output's 1,036 words are checked separately.

Final tested identity is base commit `7263a71a8761f61ee04eede22d69c75dab4d77d8` plus the recorded
uncommitted249 diff, with118 source/generated/test snapshots,12 artifacts and562 runtime inputs.
Provider ABI41; compiler library SHA256
`ce403e533104baa25276ec0dac28af1991bc60c6bd31f356fe92831897f06d9e`, provider SHA256
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`.
All final gates share this exact identity. Research243/244/245/246/247/248 indices independently
verify180/58/528/2830/158/158 unchanged artifacts; historical snapshots remain immutable. Their old
live-source hashes are not assertions about intentionally changed249 files. See
[runtime-validation249](runtime-validation.slice-249.json), the census files and raw roots
`build/nvvm-loop/slice-249-before` / `slice-249-after`.

One independent next boundary was recorded without expanding the implementation. A non-inline
`pack(BFloat16(-1.25f))` still fails at `canonical BF16 constant bits`: the existing BF16 branch passes
unsigned0xbfa0 to the signed-in-width i16 constant API. A minimal compile-only probe reproduces this
on the preserved accepted244 compiler/library and the249 candidate. No BF16 code is changed here.
The source, two diagnostics and compiler/provider/library identities are retained under
`next-bf16-constant/`. Further investigation belongs to the next slice.

Independent parent acceptance checked the production diff and input shapes, all old five-field
outcomes and failure histories, independently reconstructed output expectations, unit identities,
all final gate identities, 118 snapshots and 2,315 indexed artifacts. The retained parent audit
script/result and CUDA replay dependency hashes are linked from validation249.

No GPU loss, driver/system change, reboot or push occurred. The slice is accepted for local commit.
