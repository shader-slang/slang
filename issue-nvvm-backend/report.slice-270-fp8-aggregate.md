# Qualify substandard-float records through dynamic dispatch

Status: accepted full validation; development loop continuing with local commits only.

## Motivation

`compute/dynamic-dispatch-substandard-float.slang` passes NVRTC but direct NVVM rejects its generated
helper result `A` before execution. The source registers two concrete interface implementations:

```slang
[anyValueSize(4)]
interface IFoo { float calc(); }
struct A : IFoo
{
    FloatE4M3 value0;
    FloatE5M2 value1;
    float calc() { return float(value0) - float(value1); }
}
struct B : IFoo
{
    vector<BFloat16, 2> data;
    float calc() { return float(data.x) / float(data.y); }
}
```

`createDynamicObject<IFoo>` selects A or B from an input payload. The existing independent oracle
expects -1.0 from FP8 values1/2 and0.75 from BF16 values3/4. Scalar FP8 support does not establish
whole-record transport, and existing local BF16 record support does not establish helper results.

## Proposed solution

The reviewed gate preserves the canonical record and field keys while qualifying a finite internal
value/local-storage domain: flat integer scalar fields with FP8, scalar BF16 or BF16vector2, at least
one substandard leaf. Validate actual local CUDA layout separately from AnyValue's payload packing.
Keep general copyable/helper predicates and the parameter-decomposition policy unchanged.
The original now passes all three modes with its unchanged oracle. Full validation preserves every other accepted result.

## Change summary

The type-lowering classifier and emitter qualify internal values, local storage and the BF2 field
component addresses required by AnyValue unpacking. A focused GPU fixture and emitter units cover
the admitted boundary; the durable contract lives in `docs/design/nvvm-substandard-record-contract.md`.
The original source/oracle and both corpus inventories remain unchanged. The new fixture is outside
discovery's128-source cap.
Raw traces, reductions, baseline identities and accepted-layout copy are under
`build/nvvm-fp8-aggregate270`.

## Concepts and vocabulary

AnyValue is a fixed-size payload whose generated pack/unpack helpers translate concrete values by
canonical field key. Natural/Scalar and CUDA are separate layout-rule cache keys; a size/alignment
annotation for one rule is not an annotation for every use. Internal register values, local memory,
resource memory and exported helper ABI are separate qualification boundaries.

## Process report

The fresh final IR retains `unpackAnyValue4` returning A from a local `Ptr<A>`: it obtains each field
address, extracts its byte from the payload word, casts to UInt8, bitcasts to the canonical FP8 format,
stores that field, then loads and returns the whole record. B follows the same path using halfword
extraction, UInt16/BF16 bitcasts and vector component addresses. `generateUnpackingFunc` and
`emitMarshallingCode` intentionally create these shapes; the producer is valid and needs no repair.
Eight reduced direct cells reproduce A/B helper-result rejection at O0/O3.

The metadata `SizeAndAlignment(Natural,4,2)` on B initially raises an alignment question because a
CUDA BF2 local has alignment4. `getSizeAndAlignment` and
`findSizeAndAlignmentDecorationForLayout` key annotations by rule. Existing local-record layout
validation recomputes CUDA offsets/alignment before allocation; AnyValue independently packs its
Natural halfwords. The implementation preserves both facts and uses actual allocation alignment for whole loads.
Independent provider-layout inspection and PTX offsets establish the numeric layout; guarded runtime
neighbors test access integrity. Both value/storage cache orders are exercised for the mixed Payload.

Running existing SSA promotion again would not resolve these locals: `isPromotableVar` intentionally
rejects address chains ending in partial-field stores. An aggregate promotion/scalarization change
would be a separate optimization, not a prerequisite for representing these valid records.
Likewise, broadening `isNVVMSupportedHelperValueType` would silently authorize device pointers,
references, shared/global storage and slice267 decomposition. The new domain must remain explicit
at the value/local boundary, with external signatures and unrelated storage retaining their gates.

The helper inventory retains `asNVVMSupportedSubstandardRecordType` for the flat keyed-field
proof and `asNVVMSupportedLocalSubstandardRecordType` for its union with the established BF3/BF4
local-only family. Leaf alignment additions reuse the existing walker behind a bounded public gate. Value
construction/extraction and local layout paths consume the same canonical type; no syntax or
parallel semantic representation is introduced. The producer-consumer failure belongs at this
backend capability boundary.

Review found an unintended newly admitted local-record pointer result through the existing helper
result predicate. The selected gate excludes that new role; record values and local/reference
parameters are sufficient. Exported record values/references/pointer results remain unsupported.
`_getNVVMSequentialElementPointer` reuses `_getNVVMLocalBFloat16VectorPointer` to require a
qualified local record FieldAddress and exactly two BF16 lanes. AnyValue's component stores are
valid inputs here; the component type alone does not establish memory provenance. Candidate2 implements these restrictions and passes the unchanged original in all three modes.

The guarded `{uint16_t, vector<BFloat16,2>, uint16_t}` fixture distinguishes CUDA offsets0/4/8,
size12/alignment4 from Natural offsets0/2/6, size8/alignment2. Provider-layout inspection and emitted
PTX establish the numeric offsets; runtime guards establish access integrity. Six raw buffers of
25,165,952bytes independently check all 65,536 BF16 encodings, all FP8 bytes, independent branch/lane
choices, changed and untouched lanes, integer neighbors and sentinels. The mutation XOR0x5555 differs
from both initial lanes. Earlier correlated/no-op controls remain recorded as weaker attempts.

Opposite helper visitation orders exercise mixed Payload's Value/Storage caches in both orders;
AlignedPair remains storage-first. Focused units4/4, original3/3, fixture/controls9/9 and neighbors33/33
pass. The final fixture uses one return after conditional assignment: final O0/O3 IR contains a
Payload block parameter with distinct branch arguments. Refined fixture3/3, six supplemental raw
buffers and six assemblies pass with the unchanged integer oracle. Production and main inputs stay
frozen; pre-phi/full and final-focused fixture identities are separate.

The new fake-provider unit initially rejected FP8 semantic bitcasts represented physically as i8:
the fake's integer predicate accepts only signed/unsigned semantic operation descriptors. The exact
combined source passes nine real-provider GPU cells across three formats/modes. The fake is restored
unchanged; the retained unit uses finite1.25 construction to test record admission, while GPU/raw
fixtures prove dynamic encoding and out/inout transport. Malformed negative-test syntax and a
BF3/BF4 neighbor that requested excluded component pointers were corrected; the latter now loads
vector values before indexing registers. Earlier build, fixture and formatting failures remain in the
structured incident history. No production change accommodates a mock or harness limitation.

The synthetic Generic local-record pointer-result exclusion is code-reviewed, not execution-tested:
public source pointer returns have UserPointer types and do not reach this exact shape. Both helper
preflight and `NVVMTypeInfo::supports` reject the newly added role before cached handles can admit it;
exported local-record pointer results have a separate rejection. Lead acceptance keeps this role
closed without adding a test-only compiler API or synthetic IR harness. This evidence limit does not
apply to the supported Value/Storage cache orders, which the raw GPU experiments execute.

Full validation compares all 1,740 cells:1,738 are exactly preserved and only the original NVVM
O0/O3 failures become correct. All576 main input hashes remain unchanged. The accepted totals are
1,703 correct and 37 unresolved, with 20 resolved histories preserving the prior two failures. Units
preserve 1,099 identities and add one passing unit (1,087 pass/13 skip); semantics preserve 1,248
identities (1,170 pass/78 skip). Runtime4, material compile/assembly6, toolkit18 and runner contracts
3/7/14+1skip/16 pass. The original review-required comparison is retained. Material runtime and
performance are not measured here; earlier timing/AST evidence retains its original identity.
