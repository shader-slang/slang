# Research255: physical BF16 storage at record boundaries

Status: accepted bounded research. Production
admission is unchanged. Delegation remains unavailable at the agent limit, so one parent owns this
bounded research with separate oracle/mechanical audits; no fresh independent-agent review is claimed.

## Motivation

Consider this complete record and holder:

```slang
struct W
{
    uint16_t prefix;
    vector<BFloat16, 4> value;
    uint16_t suffix;
};
struct H
{
    W records[3];
    uint16_t tail;
};
```

After254, CUDA metadata correctly describes W as12/2, with value/suffix offsets2/10, and H as38/2
with tail36. Reusing the LLVM register vector as its stored field would still produce24/8 and80/8.
Conversely, using a scalar array for every width would break native BF2's4-byte field alignment.
Research240's explicitly aligned local arrays did not qualify their type alignment inside records.
This slice determines which physical types preserve the actual memory contract before changing
production support.

## Proposed solution

Use `<2 x i16>` as the native BF2 stored field and `[3 x i16]`/`[4 x i16]` as the BF3/BF4 component
fields. Keep logical/register values as `<N x i16>` and explicitly load/store components at the
array/vector boundary. Qualify all three through guarded source/destination record arrays and
noinline local-reference calls. The same canonical BF16 type remains the semantic source of truth;
physical storage is a role-specific representation, not a second AST/IR type.

| Width | Stored field | W size/alignment | value/suffix offsets | H size/alignment | tail |
| ----- | ------------ | ---------------- | -------------------- | ---------------- | ---- |
| 2     | `<2 x i16>`  | 12/4             | 4/8                  | 40/4             | 36   |
| 3     | `[3 x i16]`  | 10/2             | 2/8                  | 32/2             | 30   |
| 4     | `[4 x i16]`  | 12/2             | 2/10                 | 38/2             | 36   |

Fresh LLVM14.0.6 DataLayout queries agree with254's actual CUDA-device metadata for these three
candidates. The other three remain rejected layout candidates: array2 produces an8/2 record and
26/2 holder; vector3/vector4 produce24/8 records and80/8 holders. Merely aligning an allocation
cannot correct those field and array-stride differences.

## Change summary

Only this plan/report, compact semantic evidence, durable design and STATUS are intended for the
research commit. No compiler/provider/API/ABI, corpus manifest, existing fixture/input/oracle or
runtime admission changes. Generated source controls, raw LLVM, host datasets, binaries and full
outputs remain under `build/nvvm-loop/slice-255-bf16-physical-storage`.

- The small runtime gate passes4 cells on accepted254 compiler/provider identities.
- All12 source compiles are accounted for:3 CUDA source and3 NVRTC PTX controls succeed;6 production
  NVVM controls retain explicit E52017 rejection of `helper function parameter: Ptr<H, ...>`.
- All6 raw LLVM candidates verify/compile, and all9 executable PTX controls assemble forSM80.
- All9 GPU controls match603980928 returned bytes (150995232 words), including every source byte,
  destination guard/tail/padding byte, observation and sentinel.
- Each width uses131072 packets:65536 encodings at each flag value. Every source and destination
  lane of every record has complete encoding coverage under each flag; signed zero, subnormals,
  infinities and every NaN payload remain exact because no arithmetic/conversion is performed.
- Source IR, raw IR and PTX retain actual global reads/writes, local reads/writes and helper calls.
  The independent byte oracle and separate path audit pass.

Evidence closure verifies259 indexed artifacts,132 baseline source snapshots and59 final compact
references; the last2 references name the closure script/result.

All132 baseline source,12 artifact and564 runtime-input hashes remain exact254. Full254's1704 cells/
1665 correct/39 unresolved,18 resolved histories, unit/semantic gates and material6 are inherited
with their original identities. They are not fresh255 runs. Latest targeted233 and cadence0 remain
unchanged. This research proves neither production storage support nor material runtime/performance.

## Concepts and vocabulary

**Stored field type** determines field offset, record stride and enclosing alignment in LLVM
DataLayout. An alloca alignment only constrains that allocation's address. **Register vector** is
the internal by-value representation; its ABI need not match CUDA component structs. **Holder**
contains three wrapped records plus a guarded tail so record stride is observable. **Role-specific
lowering** chooses physical value/storage types through existing NVVMTypeUse caches and strict
support predicates. **Full-bit transport** copies encodings without numeric interpretation.

## Process report

The source controls construct canonical `Vec(BFloat16Type,N)` fields, typed device `Ptr<H>` values
and `BorrowInOutParam(W)` local helper parameters. Those are valid checked shapes. Source NVRTC
loads all three source records, optionally reverses each register vector, writes a different
record's vector field, reloads it, and roundtrips the value through a local inout record. Prefix,
suffix and holder-tail fields surround both arrays. This preserves the original semantic types;
there is no reconstruction of checked values as syntax or special equivalence relation.

The raw controls use the selected physical field types in `%W = { i16, storage, i16 }` and
`%H = { [3 x %W], i16 }`. BF2 uses native vector loads/stores. BF3/BF4 explicitly load scalar array
components into register vectors and extract them on stores. Generic record-pointer helpers and
volatile memory operations preserve both global and local paths. The host oracle uses independent
CUDA contracts, creates separate source/destination holders, and changes only vector-field bytes
and observation words. A second audit reconstructs writes from immutable input field bytes,
checks every input padding byte, and proves complete per-lane/record/flag encoding coverage. It
then compares the independently reconstructed whole buffers with all saved expected and GPU bytes.

Before any dataset or GPU run, the oracle review found that deriving a permutation flag from seed
parity can restrict the encodings reaching a destination lane. The declared dataset was therefore
expanded to both flags for every seed. No earlier input or output was reinterpreted. The first
independent audit used the width as an index into254's CUDA metadata, forgetting the preceding BF1
row, and stopped before dataset checking. That script/log is retained; fixing the index changed
neither datasets nor the nine already-completed GPU outputs. No GPU correctness control failed.

PTX inspection distinguishes actual behavior from source-level intent. NVRTC scalarizes the
loadLocal/readGuards helper parameters, but their callers still issue local field loads after the
pointer-based replacement call. Source BF2's local frame is40 bytes, while BF3/BF4 frames are10/12;
raw frames are12/10/12. A frame can contain compiler temporaries, so its extent is not an ABI proof
or a performance result. Raw BF3/BF4 register-helper return ABIs also differ from CUDA component
helper ABIs; the tests call internally consistent helpers and do not qualify external interoperability.

Production still stops all six new direct source controls at the Ptr<H> helper parameter. The
existing `NVVMTypeInfo::supports` BF vector branch permits only value and internal by-value helper
roles before cache lookup. `NVVMTypeLoweringContext::lowerType` already has separate aggregate and
structured-buffer representation maps; compact vector and storage conversion machinery provides
patterns to reuse. A future change must preserve format identity and select the qualified field
representation only in explicitly admitted roles. Ordinary structured-buffer vector3 conversion
cannot simply be copied to BF4 without recognizing that its stored field is also an array.
Do not widen the generic numeric/copyable classifier, bypass `_getNVVMAggregateStorageLayout`, or
apply Half's two-lane chunks and padding to BF3/BF4.

Helper/fallback inventory: no new production helper, fallback or special case. Research helpers
only expose the valid memory/value boundary and independent byte contract. The existing producer
metadata is correct after254; the remaining issue is explicit backend support, not malformed
source IR. Stop this investigation at that diagnostic and handoff. A later implementation should
start with one bounded local/reference role and its unchanged failing source test, then validate
its representation/conversion path and full preservation checkpoint before broader pointer/resource
admission. FP8 aggregates, matrices and parameter groups remain separate slices.
