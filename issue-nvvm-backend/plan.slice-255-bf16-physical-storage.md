# Qualify physical BF16 storage at pointer and record-array boundaries

This ExecPlan follows `.agent/PLANS.md` and the NVVM workflow. The maintainer requires completed
plans/reports in local slice commits; raw controls and outputs remain ignored under `build/`.

## Purpose and Observable Result

Determine a physical LLVM storage representation for BF2/BF3/BF4 that preserves the now-correct
CUDA record layout and every16-bit encoding through actual global record-array and local-reference
loads/stores. Qualify native `<2 x i16>` storage for BF2 and component arrays for BF3/BF4 against
source NVRTC and an independently constructed byte oracle. A passing raw candidate is evidence
for a later bounded implementation, not production storage support.

## Progress

- [x] 2026-09-25: Accept254 as `76f9a000e0c8c5666f29f58e126c42186eb32500`; clean checkout.
- [x] Read STATUS and research240/253/254 contracts; compare candidate scope before experiments.
- [x] Declare this research slice, all-bit byte oracle, preservation obligations and stop conditions.
- [x] Capture exact254 source/artifact/input identity and dependencies; run small runtime gate.
- [x] Query current LLVM DataLayout for vector/array candidates and wrapped/holder layouts.
- [x] Generate source NVRTC and raw LLVM pointer/record-array/local-reference controls.
- [x] Compile, assemble and execute all3 widths in source NVRTC O3 and raw NVVM O0/O3.
- [x] Audit exact input/output bytes, local/global paths and unchanged production rejection shapes.
- [x] Complete separate oracle/mechanical acceptance, design, five-part report and STATUS; include
      the completed bounded evidence in the local research commit.

## Surprises and Discoveries

All9 controls match the independent oracle across603980928 returned bytes (150995232 words).
Each source and destination lane/record covers every65536 encoding under both flags. NVRTC
scalarizes loadLocal/readGuards helper parameters, but caller local loads remain; raw helpers use
generic record pointers and volatile loads/stores. Source BF2's local frame is40 bytes while its
record ABI is12 bytes; BF3/BF4 source frames are10/12, and raw frames are12/10/12. Frame extents are
compiler observations, not a record-layout or performance claim. Raw register-vector helper ABIs
also differ from CUDA component-struct helper ABIs; no external helper interoperability is claimed.

The independent audit's first attempt indexed254's CUDA metadata by width instead of its zero-based
position after BF1, and stopped before any dataset audit. Retain that script/log; the corrected
index is(width-1)*14. Dataset, expected bytes and all9 completed GPU results are unchanged.

Before generating any dataset or running a control, the oracle audit identified that deriving the
permutation flag from seed parity can restrict the values reaching a particular destination lane.
Use131072 packets per width (every16-bit seed at each of the2 flag values) so every source and
written lane covers every encoding under both branch outcomes. This changes no production code or
existing fixture, and no earlier dataset/GPU result is being reinterpreted.

Fresh delegation remains unavailable: the253 spawn reached the agent thread limit and no tool can
close those contexts. Follow WORKFLOW's local fallback with one parent writer and separate oracle/
mechanical audits. Do not claim fresh independent-agent review.

## Decision Log

- 2026-09-25, parent: Choose storage qualification now because254 removed the proven BF4 metadata
  defect and240 already measured the representation hazard. This has a concrete path to runnable
  pointer/record/array support. FP8 aggregate/conversion/overflow and another material profile are
  separate candidates with independent semantics; none is bundled here. Latest material-driven
  implementation252 remains in rolling250/252/254, so no cadence exception is required for research.
- 2026-09-25, parent: Use the existing value vector for native BF2 storage, which naturally preserves
  its4-byte type alignment. Use scalar component arrays for BF3/BF4. Increasing an alloca's alignment
  does not repair a scalar array's alignment when embedded as a field; qualify field offsets and
  record-array stride explicitly, and retain the wrong candidates' DataLayout observations.
- 2026-09-25, parent: Cover all65536 BF16 bit encodings at both runtime flag values without arithmetic or conversion. Source
  records, destination records and observations are separate regions. Assign vector fields only;
  C++ whole-struct padding-copy behavior is outside this contract. Check every buffer byte,
  including untouched prefix/suffix/tail/padding/sentinel bytes.

## Outcomes and Retrospective

The three selected physical field types match the corrected CUDA record/holder ABI. All9 source
NVRTC/raw NVVM controls preserve603980928 bytes (150995232 words) over131072 packets per width,
with complete per-lane/record/flag encoding coverage and all guards/tails/padding/sentinels checked.
Actual global/local operations and helper calls survive. All6 production source cases remain
explicit Ptr<H> helper-parameter preflight stops; no production support is inferred.

Baseline132 source/12 artifact/564 input hashes remain exact254. Runtime4 is fresh; full254's
1704/1665/39 outcomes and18 resolved histories are inherited, with targeted233/cadence0 unchanged.
A later implementation should bound one local/reference role, preserving qualified native BF2 and
component-array BF3/BF4 storage and explicit value conversions. Device/resource admission, FP8 and
matrix policy remain separate. Final mechanical acceptance passes; retain the closed artifact index and closure verification in
the local research evidence.

## Context and Current Pipeline

BF2's CUDA type is native4/4; BF3/BF4 are component structs6/2 and8/2. LLVM i16 vectors have
allocation/alignment4/4,8/8,8/8, while scalar arrays have4/2,6/2,8/2. Research240 qualified register
transport and local component-array roundtrips, but explicit BF2 alloca alignment did not qualify
its embedded array type. Research253 proved the BF4 AST/IR metadata defect;254 fixed that producer
without admitting runtime storage. Register/helper values remain physical i16 vectors, provider
ABI41, with strict role checks before type-handle cache lookup.

Consider the complete shape:

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

For BF2/BF3/BF4 respectively, W is12/4,10/2,12/2 with value offsets4/2/2 and suffix offsets8/8/10.
H is40/4,32/2,38/2 with tails36/30/36. Reuse254's actual CUDA ABI/reflection evidence unchanged.
Trace future integration through `NVVMTypeInfo::supports`, `NVVMTypeLoweringContext::lowerType`,
aggregate storage layout validation, compact vector load/store and existing storage conversions.
Do not change those production paths in this research slice.

## Scope and Non-Goals

Only standalone source/raw-LLVM controls, data-layout proof, independent host bytes and retained
research evidence. No compiler/provider/API/ABI, test manifest, existing fixture, input or oracle
change. No FP8 support, BF arithmetic/conversion, external CUDA helper ABI, StructuredBuffer or
parameter-group admission, matrix layout, emitter patch or material runtime/performance claim.
Pointer and local-reference controls qualify the selected physical representation, not every
potential storage role. Retain production preflight stops as stops.

## Architecture and Invariants

Use two512-byte packets per seed0..65535 (one for each runtime flag), with separately guarded source/destination holders and
uint32 observations. Each of the3 source records and each vector lane contains a bijective additive
16-bit transform of the seed, so every stored source lane sees every encoding. Read every source
record; permute its register lanes by a runtime flag, write a different destination record's vector
field, reload it, and roundtrip it through a noinline local inout-record helper. Observe original,
global-reloaded and local-reloaded values plus local prefix/suffix guards. No floating arithmetic
may quiet signaling NaNs or collapse signed zero.

The host oracle packs records using independently specified CUDA size/alignment/offset contracts,
then updates only destination vector bytes and designated observation words. Input source records,
destination guards, both holder tails and all padding/sentinel bytes must remain exact. Never infer
expected bytes from compiler reflection or a GPU result. Keep complete input, expected and output
buffers, source/IR/PTX, compile/assembly logs and exact commands.

Raw LLVM uses `<N x i16>` register values. BF2's stored field is `<2 x i16>`; BF3/BF4 use `[N x i16]`
and explicit component extraction/insertion at value/storage boundaries. Noinline pointer helpers
and a local record allocation must survive into the measured paths. Do not replace these with
constant-return controls. Verify source IR, raw IR and PTX retain the intended helper and memory
boundaries; report optimizer differences rather than inventing absent operations.

## Interfaces and Dependencies

Native Ubuntu24.04, L4SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86, targetSM80, LLVM14, ABI41.
Baseline compiler `a595092cb50be989df9015d38852946afd599def83b5485ba5188b2a5e4e3f7a`, provider
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`. Source the inspected203 env and
use matching RelWithDebInfo tools. At most4 CPU workers; GPU suites sequential and bounded30min.
Retain exact dependencies from240's raw libNVVM compiler helper, LLVM DataLayout proof and the
qualified CUDA-driver helper; adapt only inside the new raw root. No full compiler build is planned.

## Milestones and Validation

1. Capture254's132 source/12 artifact/564 runtime-input hashes plus any additional research
   dependencies. Record clean branch/base/submodule/toolchain identity. Pass the small runtime gate.
2. Fresh DataLayout proof for all6 vector/array candidates, including value layout, W offsets/stride
   and H tail/size/alignment. Compare correct candidates against254's actual CUDA device evidence;
   record array2 and vector3/4 mismatches without labeling them supported.
3. Declare and generate exact full-bit datasets and9 runnable cells (3 widths x source NVRTC O3 /
   raw NVVM O0/O3), compile with fixedSM80 and assemble every successful PTX. Retain source direct
   O0/O3 diagnostics for all3 widths to confirm no accidental admission. Document these6 rejection
   cells separately from GPU correctness. If a source control needs a harness correction, preserve
   the failed attempt and regenerate a fresh, explicitly identified set before acceptance.
4. Execute9 controls and compare full buffers byte for byte against the independent oracle. Require
   all65536 input encodings for every source lane/record, exact global/local vector outputs and
   unchanged guard/tail/padding/sentinel bytes. Inspect live source/IR/PTX paths and current type-role
   producers/consumers only far enough to define a bounded implementation handoff.
5. Recheck unchanged production/source/artifact/input hashes. Inherit full254's1704/1665/39 outcomes,
   units/semantics/toolkit/contracts and material6 with original identities; do not relabel them fresh.
   Research does not advance implementation cadence. No full-corpus replay without changed code.
6. Complete report/design/STATUS and an exact local acceptance audit, close raw artifact index and
   commit only the bounded research documents/evidence. Continue the authorized loop from findings.

## Failure and Recovery

Retain all unsuccessful controls with source and commands. An LLVM layout mismatch is a rejected
candidate, not a reason to weaken CUDA expectations. A wrong GPU output requires tracing actual
memory/value conversion before retaining a representation. Keep source/IR diagnosis distinct from
backend admission; do not change production merely to advance a diagnostic. Stop on GPU loss or an
unresolved representation/correctness decision requiring human input. No driver/system change,
reboot, push or publishing. Bound compile/runtime waits and do not retry indefinitely.

## Artifacts and Handoff

Raw root: `build/nvvm-loop/slice-255-bf16-physical-storage`. Completed plan, five-part report,
semantic evidence, durable design and STATUS are the only intended committed changes. Preserve
actual before identities, dataset/oracle definitions, all-byte outputs, layout facts, source/IR/PTX
path evidence, failures, commands and artifact index. A later implementation needs its own ExecPlan,
focused failing source regression, strict role-specific integration and full shared-contract checkpoint.
