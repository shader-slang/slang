# Preserve signed BF16 literal bits at the builder boundary

## Motivation

Consider this complete scalar transport example:

```slang
RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint pack(BFloat16 value)
{
    return uint(bit_cast<uint16_t>(value));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = pack(BFloat16(-1.25f));
}
```

The expected encoding is 49056 (0xbfa0). Accepted249 rejects the direct path with E52018,
`canonical BF16 constant bits`, result -2147024809. The unchanged12-word regression fixture passes
NVRTC O3 and fails NVVM O0/O3. The minimal predecessor evidence also reproduces the diagnostic
with the preserved accepted244 compiler library, so this is a pre-existing correctness gap.

## Proposed solution

Pass the recovered BF16 encoding to the existing integer builder as its signed16 bit representation.
For example, uint16 0xbfa0 becomes int16 -16480, and the provider creates the same i16 bits.
The checked literal producer and its rounding remain the semantic source of truth. No API, ABI,
BF16 operation, storage or vector admission changes; provider ABI 41 remains unchanged.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: adapt the existing BF16 constant argument with
  `bitCast<int16_t>` and explain the signed builder contract using -1.25.
- `tests/cuda/nvvm-bf16-signed-literals.slang`: preserve canonical constants through a noinline
  helper, checking both signs of 1.25, zero, minimum/maximum subnormal, minimum normal and maximum
  finite. The values are exactly representable and impose no new rounding policy.
- Discovery manifest: append one distinct source, preserving all 114 old source rows and frozen IDs.
  The addition requires a full checkpoint. Design documentation records the physical API boundary.
- Plan, report, validation250, census/discovery summaries and STATUS retain fresh evidence and
  historical preservation obligations.

## Concepts and vocabulary

- **Canonical BF16 literal:** an IRFloatLit already rounded to BF16 by IRBuilder::getFloatValue.
- **Signed in-width argument:** the builder's int64 argument must be representable as signed i16;
  an unsigned payload with bit15 set needs bit reinterpretation before widening to int64.
- **Internal helper transport:** a checked BF16 value crosses a by-value noinline helper as physical
  i16. This test does not establish an external CUDA helper ABI or a storage layout.

## Process report

SCCPContext::evalCast passes the checked floating value to IRBuilder::getFloatValue. For
BFloat16Type, that builder uses BFloat16ToFloat(FloatToBFloat16(value)) to preserve the canonical
rounded number. `_getLoweredNVVMValue` receives that valid IRFloatLit and recovers its bits with the
same FloatToBFloat16 helper. `_getIntegerConstant` accepts signed in-width integers through
llvm::isIntN and creates them with ConstantInt::getSigned. The old emitter passed an unsigned short
straight into that signed API; negative BF16 encodings therefore exceeded signed16 range.

The input-shape audit establishes that the producer is correct: final IR retains all 12 BF16 literals,
including separate -0/0 and both subnormal signs. Both direct PTX outputs pass the12 independently
expected bit encodings into `pack`. The emitter owns this physical API adaptation. Widening the
provider's contract, changing canonicalization, or converting checked values back to syntax would
address the wrong layer. The test fails at this exact boundary with the old argument and passes
with the new one. The entire fixture is byte-identical before and after implementation.

Self-review inventory: no new production helper, fallback, guard or special case. The existing BF16
branch survives with one argument adaptation and an explanatory comment. No custom equivalence,
structural graph walk, second semantic representation, reconstructed syntax or default value is
introduced. Shared rounding/overflow/nonfinite behavior and builder validation are unchanged.
The new `pack` function is solely a test helper that forces the literal to cross the changed boundary.

Before implementation, accepted249 identity matched 118 source/generated/test paths,12 artifacts and
562 runtime inputs. The before smoke gate passed 4/4. The final emitter was formatted explicitly
before building and testing; no Slang fixture or unrelated document was reformatted. All suites use
native RelWithDebInfo, L4SM89 targetingSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14 and ABI 41. GPU suites
run sequentially, with at most 4 corpus workers or 2 unit servers and 30-minute bounds.

Final acceptance results are recorded in `runtime-validation.slice-250.json`; independent parent
acceptance passed. The standalone output audit compares all words for the new fixture and five
BF16/FP8 neighbors to exact expectations: 3444 final words plus 12 before words. The literal oracle
uses the BF16 sign/exponent/fraction formula and independent Float32 bit packing; FP8 finite values
use the representable grid and ties-to-even results. NVRTC differential coverage supplements these
expectations. BF16 neighbor tests retain their original dynamic inputs and independent predicates.

The full checkpoint preserves all 1698 old five-field outcomes and all 1659 prior passes, adds
exactly 3 new passing cells, and retains 39 unresolved histories and 18 resolved histories. There are
no missing/extra/duplicate requested cells or unexpected outcome/diagnostic/execution transitions.
Every registered material cell is reassessed; all 6 pass compile/assembly checks. These remain
support checks only.

| Gate                                     | Fresh result                                                        |
| ---------------------------------------- | ------------------------------------------------------------------- |
| Runtime smoke                            | 4/4                                                                 |
| Focused BF16/FP8 fixtures                | 18/18                                                               |
| NVVM/routing/reporter/math/literal units | 513/513, one unchanged skip; all prior IDs retained                 |
| Toolkit / runner contracts               | 18/18 and 6/6                                                       |
| Frozen                                   | 1356 cells: 1347 correct,9 unchanged unresolved                     |
| Discovery                                | 345 cells: 315 correct,30 unchanged unresolved;3 additions          |
| Combined                                 | 1701 cells: 1662 correct,39 unresolved; all 1698 old outcomes exact |
| Material support                         | 6/6 compile/assembly only                                           |

Final source/binary/input identities cover 119/12/563 paths and match across every gate. The provider
binary is unchanged; compiler library SHA256 is
`ae6fe92965ed066a02ff9eab550093294b916c6b29f122f02e8a3f2ab8b74f60`. The raw artifact index retains
2132 entries, including immutable before/final source snapshots.

The materials’ runtime binding/texture/LUT/input/output contract remains unavailable. This correctness fix is independently motivated rather than material-driven; after
acceptance the parent re-ranks material compile-time work using a fresh profile, without making
unsupported material-runtime claims. Profiling is outside this slice.

Earlier exhaustive FP8 replay/phi, AST timing, texture and column-major research remain inherited
with their original accepted identities. The new slice does not relabel those experiments as fresh
or change the original wrong-output histories. No new independent blocker was discovered in the
focused BF16 domain. Raw evidence lives under `build/nvvm-loop/slice-250-before` and
`slice-250-after`, with before/final snapshots, identities, commands, logs, output/IR audits and an
artifact index. No driver/system changes, reboot, push or local commit were performed by this worker.

Independent parent acceptance verified the production boundary, all 1698 old five-field outcomes,
three additions, all unresolved/resolved histories, 3444 final output words, 12 before words, final
IR and PTX, every unit identity, source/binary/input identities, 119 snapshots and 2132 indexed
artifacts. The parent audit script and result are retained by hash in validation250. Full250 is
accepted, with targeted233 and implementation cadence0.
