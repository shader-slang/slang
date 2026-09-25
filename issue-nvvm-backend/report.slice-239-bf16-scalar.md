# Implement scalar BF16 transport and Float32 conversion

## Motivation

Consider this scalar projection of the frozen BF16 workload:

```slang
[noinline]
BFloat16 choose(BFloat16 value, bool condition)
{
    BFloat16 result = BFloat16(3.0f);
    if (condition)
        result = value;
    return result;
}

[noinline]
void replaceValue(inout BFloat16 destination, BFloat16 value)
{
    destination = value;
}

BFloat16 value = bit_cast<BFloat16>(uint16_t(rawBits));
BFloat16 stored = BFloat16(1.0f);
replaceValue(stored, choose(value, condition));
uint expanded = asuint(float(stored));
uint narrowed = uint(bit_cast<uint16_t>(BFloat16(asfloat(floatBits))));
```

The frontend intentionally builds BFloat16Type and FloatCast/BitCast operations. Before this slice,
direct NVVM rejected the helper's BFloat16 result. BF16 differs from IEEE Half despite their equal
width: BF16 has eight exponent and seven fraction bits. Adding it to the existing width16 IEEE
classifier would select incorrect arithmetic and conversions. Accepted research238 establishes the
SM80 conversion recipe and exhaustive bit transport oracle. Full vector/dot support remains a
separate obligation; moving the original frozen source's diagnostic is not this slice's proof.

## Proposed solution

ABI38 adds an explicit scalar BF16 semantic kind. The compiler preserves canonical BFloat16Type
and admits only scalar value, helper argument/result, mutable local pointer and local storage roles.
The provider transports these values as i16 because libNVVM12.9 rejects native LLVM bfloat.
The shared catalog admits exact scalar BF16↔Float32 conversion, same-width integer bit transport
and typed selection. It leaves BF16 outside all IEEE arithmetic/conversion families.

Narrowing uses SM80 `cvt.rn.bf16.f32`; widening zero-extends, shifts left16 and bitcasts to Float32.
This preserves every source BF16 payload on the qualified target. Runtime narrowing NaNs are checked
by classification, not a universal encoding promise. Constants reuse core FloatToBFloat16 to recover
the value already canonicalized by IRBuilder::getFloatValue, without applying dynamic CUDA NaN policy
to existing IR literals. Integer, Half and double conversions remain rejected.

## Change summary

- Builder API/catalog: ABI38 semantic kind and exact BF16 conversion/bitcast/select signatures.
- Type lowering: exact scalar predicate, bounded role classification and physical i16 cache entry;
  local helper-pointer and alignment reuse without widening recursive aggregate/resource predicates.
- NVVM emitter: selected scalar availability, constant bits, helper signatures and local layout proof.
- Provider: distinct descriptor→i16 mapping and qualified Float32 narrowing/expansion.
- Tests: real-provider rejection/serialization unit, six pre-provider negative source cases,
  registered readable fixture with independent bit expectations and runtime-dependent selection.
- Evidence: full checkpoint, unchanged research238 scalar projection and raw i16 replay, plan/report,
  design and STATUS. No frontend, standard library or runner changes.

Final evidence is in [runtime-validation.slice-239.json](runtime-validation.slice-239.json).

| Final gate                         | Result                                                              |
| ---------------------------------- | ------------------------------------------------------------------- |
| Smoke / registered fixture         | 4/4 and 3/3                                                         |
| Units / toolkit / runner contracts | 482/482 plus existing Windows-only skip; 18/18; 6/6                 |
| Frozen                             | 452 identities / 1,356 cells; 1,343 correct                         |
| Discovery                          | 109 old identities / 327 exact old cells plus 3 new correct cells   |
| Total runtime                      | 1,686 fresh / 1,643 correct / 43 unresolved / 14 resolved histories |
| Scalar research replay             | 5 launches × 73,190 records; 5 SM80 assemblies                      |
| Replay output / untouched words    | 1,097,850 / 4,757,355; 5,855,205 total                              |
| Material support                   | 6/6 compile/assembly; no runtime/performance claim                  |

All 1,640 prior correct cells remain correct. The only old-cell changes are direct O0/O3 frozen
BF16 diagnostics: `helper function result type: BFloat16` becomes
`helper function parameter: vector<BFloat16,4>`. Canonical shape changes correspondingly; preflight
classification, return code1 and complete execution counts remain exact. Both whole prior failure
records remain in diagnostic history, and all43 first-known/reproduction records and14 resolved
histories are retained. There are no omissions, duplicates, extra cells, inherited final cells or
baseline reset.

Tested base is d7732c6ba2e2978811b628ab5740bdd25e9a273d plus the recorded patch. Final compiler-library
SHA256 is a6cd5bd057defd8fc896ebd75813d8b7e5737fe33a095e20840fc73b72233412; provider SHA256 is
cefb3cd3cb44fb0d2c6a201f210ea3c98e1913c2fcac554ea5ad912d6afbcfd7. Every final gate matches37 source
paths,12 artifacts and558 input hashes, preserving all557 old inputs. Native Ubuntu24.04, L4SM89,
target80, driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14 and RelWithDebInfo remain unchanged.
The separate audit validates compact references, accepted research236/238 artifacts and complete
replay buffers. Independent parent acceptance passes:699 compact references and1,073 including
accepted research artifacts, complete prior failure records, all old outcomes and every replay word.
Full checkpoint239 is accepted; implementation cadence resets to zero.
The original frozen vector/dot BF16 source remains unsupported; its exact diagnostic transition is
recorded without adding resolved histories. Material's six compile/assembly cells are support
checks only; absent bindings/textures/LUT/input/output oracle still block material runtime claims.

## Concepts and vocabulary

- **Semantic format:** a provider descriptor distinguishes BF16 from IEEE Half and integer values,
  independently of their physical 16-bit storage.
- **Type role:** value, local storage, helper signature and external storage have independently
  qualified representations; an admitted scalar does not automatically admit its resource container.
- **Canonical literal:** IRBuilder already rounded the host value to the Slang type; emission
  transports that semantic value instead of inventing a second constant representation.
- **Double rounding:** integer→Float32→BF16 can differ from exact integer→BF16; 16842753 is a measured
  counterexample (0x4b80 versus 0x4b81).

## Process report

The helper/fallback inventory contains one new exact type predicate, isNVVMBFloat16Type, and no
fallbacks, source reconstruction or alternative equality relations. It survives because canonical
BFloat16Type must be recognized without classifying it as IEEE Half. The descriptor kind, bounded
role branches, constant branch and provider conversion branch are explicit format ownership.

For the motivating code, the standard constructor produces FloatCast, bit_cast produces BitCast,
and the noinline mutable parameter becomes BorrowInOut<BFloat16>. These are valid canonical forms;
there is no producer bug to repair. `_getNVVMSemanticType` describes BF16 explicitly and
`resolveValueOperationFamily` selects only the proven scalar Float32 pair. `_getSemanticLLVMType`
returns i16 for exactly width16/lane1 BF16. `_emitValueOperationFamily` lowers the selected conversion
without interpreting that physical i16 as a numeric integer or IEEE Half. The exhaustive replay
fails before this change and verifies all65,536 BF16 payloads afterward, including NaN upper-word
expansion. Additional Float32 patterns check ties, underflow, normal/subnormal boundaries and overflow.

For the helper's constants, IRBuilder::getFloatValue already calls
BFloat16ToFloat(FloatToBFloat16(float(inValue))). `_asExecutableFloatingPointConstant` admits that
literal; `_getLoweredNVVMValue` uses the same core conversion to recover its canonical bits and creates
an i16 constant. This code preserves the existing semantic source of truth. It does not rebuild
syntax or normalize runtime NaNs to a chosen constant.

For local storage, `asNVVMSupportedLocalHelperValuePointerType` admits exactly a generic ordinary
local pointer or mutable output/borrow pointer to scalar BF16. `_hasNVVMCompatibleHelperValueLayout`
checks CUDA and LLVM size/alignment at that already-qualified local boundary. The existing pointer
cache, local allocation, load/store and helper call paths then carry i16 with alignment2. Recursive
copyable/helper predicates remain unchanged, so struct/array/device/resource BF16 containers do not
inherit unproven support. This is a backend role qualification, not a workaround for malformed IR.

The registered fixture's original before-proof passed NVRTC and rejected direct O0/O3. Parent review
requested an input-dependent branch; the fixture was revised once before final testing, its old
source/log retained, and before-proof rerun using a preserved accepted compiler library with the
verified a89e…19f hash. At that point the provider had rebuilt ABI38, but both direct cases still
rejected E52017 before provider discovery. That mixed provider state is not a successful ABI claim.
The final fixture hash is frozen from this second before-proof onward.

The research projection removes only unsupported integer-construction outputs7/8 from the accepted
convert.slang. The 73,190-record original input and expectation buffers remain unchanged. Active
columns4/5/6 check raw transport, expansion and narrowing; columns7/8 retain their input sentinels.
Public NVRTC/O0/O3 and accepted raw-i16 O0/O3 controls use this same domain. A separate checker
recomputes finite narrowing by upper-word nearest-even carry and verifies every output and untouched
word. Accepted research238 artifacts are hash-checked, never overwritten.

Rejected alternatives are native LLVM bfloat (parser rejects), treating BF16 as IEEE width16, using
generic FP32 conversion for integer construction (measured double-rounding errors), and widening
recursive helper/storage classifiers for a scalar fixture. No unreachable fallback was added.
An equivalent isFloat16-cache refactor was removed after review because it fixed no failing case.
The formatter ran only explicit C++ paths; unrelated historical formatting hunks were restored
with the retained exact patch, then final build/gates followed. No Slang fixture was clang-formatted.

Initial final gates passed smoke4, fixture3 and all5 replay launches, but the negative unit matrix
contained two frontend-invalid sources: BF16-to-int conversion was ambiguous (E30080), as was BF16
addition (E39999). Those are not backend preflight evidence and were removed from that matrix;
provider descriptor rejection coverage remains. The resource case's exact rejection is `struct field
address result`. All failed attempt1 evidence is retained. Only unit source changed, then formatting,
unit rebuild and all gates reran with final identities. No producer or library change was made.

Self-review also identified that the GPU branch proves phi transport, not explicit Select.
The generic `select(bool,T,T)` intrinsic in core.meta.slang intentionally produces this valid
scalar IR shape. The existing provider's CreateSelect owns its transport; the catalog must admit
exact BF16 operands/result. The real-provider unit now uses a dynamic i1 condition and two i16
values, verifies query/emission/serialization, and rejects wrong-Half/arity descriptors. Removing
BF16 from catalog selection causes this named unit to fail. This strengthens validation without
production/header changes. The incomplete frozen attempt2 was safely stopped and retained;
its cells are not counted as passing evidence. Unit-only rebuild precedes another complete final run.
