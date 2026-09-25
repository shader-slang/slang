# Slice259: field access through local BF16 record references

Status: accepted full checkpoint, provider ABI41. Fresh delegation is unavailable at the agent limit;
one local writer uses separate source, oracle and mechanical acceptance audits. No fresh independent-
agent review is claimed. No push or system/driver change is authorized by this slice.

## Motivation

Consider this complete local-record data path:

```slang
struct Record
{
    uint16_t prefix;
    vector<BFloat16, 3> value;
    uint16_t suffix;
}

[noinline]
void initialize(out Record destination, vector<BFloat16, 3> value)
{
    destination.prefix = uint16_t(17);
    destination.value = value;
    destination.suffix = uint16_t(19);
}

[noinline]
vector<BFloat16, 3> replace(inout Record destination, vector<BFloat16, 3> value)
{
    let previous = destination.value;
    destination.value = value;
    return previous;
}

RWStructuredBuffer<uint> outputBuffer;
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let x = bit_cast<BFloat16>(uint16_t(tid.x));
    Record local;
    initialize(local, vector<BFloat16, 3>(x));
    let previous = replace(local, vector<BFloat16, 3>(x));
    outputBuffer[tid.x] = uint(bit_cast<uint16_t>(previous.x));
}
```

The bare BF16 vector/reference works after256, and255 qualified physical record fields, but the
local record reference is still rejected. The new unchanged regression covers widths2/3/4, differing
lanes, out initialization, inout replacement/readback and integer guards. A mixed record additionally
covers scalar BF16, BF3, and unsigned8/16/32/64-bit neighbors. Before production changes, NVRTC passes;
both NVVM modes reject `helper function parameter: OutParam<Record2>` with E52017.

This is a new bounded capability, not a resolved old corpus failure. Original255 controls first stop
at device `Ptr<H>`. Frozen dynamic-dispatch-substandard-float first stops at FP8 helper result A;
its BF2 record B is behind that separate boundary. Those diagnostics and all original oracles remain
preservation obligations. Research258's material getter experiment failed its declared performance
criteria and was discarded; that current material review justifies proceeding without another
speculative optimization. The rolling implementation window254/256/259 is now capability/correctness
work, so reconsider material-driven work next. Material runtime inputs remain unavailable.

## Proposed solution

Admit a finite local-storage record family: integer scalar fields and canonical BF16 scalar or
vector2/3/4 fields, with at least one BF16 field. Keep its existing canonical IRStructType and field
keys. Exact Generic one-operand local Ptr, OutParam and BorrowInOutParam roots select Storage for the
pointee. BF2 fields use `<2 x i16>`; BF3/BF4 use component arrays, while loaded register values remain
vectors. Reuse256's leaf load/store conversions and existing keyed field-address resolution.

Prove local provider size/alignment against CUDA layout with the existing aggregate-layout walk,
using an explicit local permission. Its default global/resource proof stays unchanged. No recursive
record admission, whole-record value transport, readonly reference, resource/device storage or
external helper ABI is added. Provider operations and ABI41 are unchanged.

## Change summary

- `slang-emit-nvvm-type-lowering.{h,cpp}` defines the finite local-record classification and selects
  existing Storage lowering for admitted helper-pointer pointees.
- `slang-emit-nvvm.cpp` carries the local-record role through field-address resolution, local layout
  validation/allocation and reachable struct declarations. BF vector field accesses reuse existing
  register/storage conversion. CUDA-exported record reference parameters reject explicitly.
- `unit-test-nvvm-emitter.cpp` adds a structural positive unit for widths2/3/4 and four negative cases
  covering exported, readonly, nested and resource roles. Existing by-value negatives remain.
- `nvvm-bf16-local-records.slang` adds672 exact outputs per mode and one discovery source. Frozen
  selection, all old source tests and their output oracles are preserved.
- Plan, five-part report, design contract, compact validation/census results and STATUS record scope
  and acceptance. Generated controls, full buffers, logs, snapshots and binaries remain under build/.

## Concepts and vocabulary

**Local record role** authorizes physical fields only through an admitted local/mutable-reference
root. **Register/storage distinction** keeps one semantic vector with a vector value and, for BF3/4,
a component-array pointee. **Field key** is the canonical IRStructKey used to select the field;
record names and ordinal guesses are not semantic identities. **Representation cache** associates a
provider type with a use without replacing canonical IR identity or admitting a different use.

## Process report

The input shape is canonical and intentionally allowed. Parameter/local lowering already produces
OutParam<Record>, BorrowInOutParam<Record> and local Ptr<Record>. IRBuilder::emitFieldAddress
(`slang-ir.cpp`) finds the canonical field by key and builds an explicit pointer carrying access,
address-space and layout operands from the root. That four-operand spelling is valid. Making the
producer discard these operands would break its contract; the memory consumer must retain proof
that this field came from an admitted local record.

`asNVVMSupportedLocalBFloat16RecordType` is the sole new classifier. It checks direct field types,
requires a BF16 leaf, and rejects nested records/arrays and other unqualified field families.
`asNVVMSupportedLocalHelperValuePointerType` still requires the original one-operand Generic mutable
root shape. NVVMTypeLoweringContext::lowerType selects Storage for that pointee, and the existing
_lowerStructType lowers its fields in the same role. Ordinary helper-value/copyable/resource
classifications remain unchanged. Existing role admission precedes cache lookup; pointer cache keys
already include pointee use. All function signatures are lowered before bodies in
emitNVVMIRFromLinkedIR. In the value-first control, permute returns V before initialize lowers its
record pointer; storage-first initialize returns void and lowers the record-pointer parameter before
its V parameter. PTX declaration order confirms this traversal, with identical GPU inputs.

The helper/branch inventory and decisions are:

- The new local-record classifier survives: removing admission restores the original OutParam
  rejection. It is a role predicate over canonical fields, not a second semantic representation.
- NVVMStructField's local-record flag and the extension of _getNVVMStructFieldAddress survive.
  Existing root classification and keyed field lookup prove the owner and mutability. Only that
  root permits BF vector fields beyond the existing executable leaf family. Flat classification
  prevents recursive field permissions.
- The extended _getNVVMLocalBFloat16VectorPointer survives. The old bare-pointer check cannot
  recognize a canonical explicit field pointer. It now also requires an IRFieldAddress whose
  resolved owner carries the local-record flag. Removing only this route, rebuilding and compiling
  the unchanged fixture reproduces E52017 `load result type: vector<BFloat16,3>` at O0 and O3.
  Restoring the exact candidate source restores the successful path. This revert drill locates the
  producer/consumer boundary without changing the correct producer.
- The explicit local permission in _getNVVMAggregateStorageLayout and its compatibility wrapper
  survives. It enables the already-qualified BF scalar/vector leaves only for the admitted local
  record allocation. Existing size/alignment composition is reused; the default global/resource
  callers do not receive this permission. Local preflight compares it against CUDA layout, and
  allocation uses the same proved alignment. This avoids applying LLVM register-vector layout to
  component storage. No record-specific hardcoded offsets are added to production.
- Storage selection for local allocations/helper pointees and reachable struct declarations
  survives. The positive unit checks physical field handles and allocation alignment. StructType
  is an IR parent declaration, so the module audit must retain the newly reachable declaration as
  well as its field types. This does not admit whole-record instructions or by-value signatures.
- The CUDA-exported reference rejection survives and has a dedicated negative case. Internal
  provider helper layout does not establish external CUDA interoperability. Existing readonly,
  nested, by-value and resource boundaries remain enforced.

No new equivalence relation, AST/IR reconstruction, operand-graph search, default-value fallback or
provider operation was introduced. The two representations are existing provider uses of one
canonical semantic vector. Field address, load and store validation still prove exact pointee/value
identity before emission. All retained branches own the local storage boundary demonstrated here.

The first new unit attempt used the fake builder's global struct-field list instead of its existing
scalarStructFieldTypes list. A new readonly negative fixture also used brace initialization, which
introduced an earlier by-value constructor rejection. Correcting the new test assertion and using
fieldwise out initialization reaches the intended borrow diagnostic; production and old fixtures
were unchanged. Failed attempts remain in evidence. A PTX audit initially searched for the C++
IR opcode name; the dump spells it `get_field_addr`. Correcting that audit token required no new
runtime run or source change.

The independent full-buffer oracle reconstructs672 fixture words per mode (2016 total). Exhaustive
controls cover scalar and widths2/3/4, both lowering orders and all three modes:24 launches,
100664064 words/402656256 bytes. Each control runs every65536 encoding under both permutation flags.
Every old/replaced/copied lane and packed guard covers the full encoding set. Untouched input,
header, gaps and sentinels are also compared exactly; NVRTC agreement supplements this independent
oracle. No arithmetic or numeric conversion is involved.

PTX retains local frames, non-inlined initialization/replacement/read helpers and real loads/stores.
BF3/4 replacement retains individual component accesses. Optimized read helpers can accept values
already loaded by the caller. Declaration order verifies value-first versus record-storage-first
lowering in both direct modes. Frame sizes and optimized helper signatures are not external ABI
measurements. The separate LLVM14 DataLayout query verifies guarded scalar/2/3/4 records at6/2,
12/4,10/2,12/2 with offsets0/2/4,0/4/8,0/2/8,0/2/10. Mixed unsigned8/BF16/unsigned32/unsigned64/BF3/
unsigned16 fields are24/8 at0/2/4/8/16/22. Structural provider-type checks and real source GPU output
connect that physical-layout qualification to emitted code.

Full frozen1356 retains1347 correct and9 unresolved. Discovery354 preserves all351 old outcomes
and adds3 correct, reaching324 correct and30 unresolved. Combined1710/1671correct/39unresolved
preserves every1707 old five-field outcome and18 resolved histories, with no missing/duplicate cells.
All1062 prior unit IDs and1129 semantic IDs/outcomes remain exact; the new unit brings totals to
1050pass/13skip. Semantics1052pass/77skip, runtime4/focused30, toolkit18/contracts6 and all6 material
compile/assembly cells pass. Material PTX and cubins remain byte-identical to256. No material runtime
or performance claim is made.

The tested base is a11fda54adc9ba3681eb5529a3d25673bad352d7 plus the259 diff. Final identity covers
136 source/generated/test/manifest snapshots,12 artifacts and566 runtime inputs. All565 prior inputs
and18 submodule pins are unchanged. Compiler SHA256 is 638db4d1cbbfb6c9a3ccca2f6058a89c45071e94ab1ba51a710fee5b8a3bf5f9;
provider remains5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab on ABI41.
The source delta consists of the3 production files, unit file and one-row manifest addition, plus
new fixture and2 newly captured producer files. The compiler, unit library and builtin cache are the
only changed tested artifacts. An initial identity-audit assertion omitted the intentional manifest
change; the corrected audit verifies its exact one-row addition. See runtime-validation.slice-259.json
for complete commands, hashes, outcomes and preserved histories.
Raw roots are `build/nvvm-loop/slice-259-before` and `slice-259-after`.

Evidence closure verifies3885 raw artifacts (818746917 bytes),136 final source snapshots and358
final compact references. The final read-only closure includes its own two audit references without
rewriting the accepted result. No GPU loss, system/driver change, reboot or push occurred.
