# Support field access through local BF16 record references

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires completed NVVM plans/reports in
local commits. The development loop is authorized. Fresh delegation remains unavailable at the
agent-thread limit; one local writer uses separate source, byte-oracle and mechanical acceptance.

## Purpose and Observable Result

Allow a nonrecursive local record containing integer guards and canonical BF16 scalar/vector fields
to be allocated and accessed through internal inout/out references. Preserve each field's raw bits
and qualified CUDA layout. BF2 fields use native two-lane storage; BF3/BF4 fields use component arrays.
A focused source test must fail before the change, then pass exact GPU output under NVRTC O3 and
NVVM O0/O3. No whole-record by-value, device/resource, nested-record/array or external ABI support
is implied. Original research255 Ptr<H> and frozen FP8 A result diagnostics remain unchanged.

## Progress

- [x] 2026-09-25: Read current WORKFLOW/STATUS, reports255/256/258 and relevant role/layout emitters.
- [x] Base a11fda54adc9ba3681eb5529a3d25673bad352d7 is clean;258 restored all accepted256 identities.
- [x] Declare the bounded scope before implementation.
- [x] Before133/12/565 identities exact256 and runtime4 pass. New672-word fixture passes NVRTC;
      NVVM O0/O3 both reject OutParam<Record2>. Original fixture/oracle retained unchanged.
- [x] Local record role implemented; unchanged672-word fixture passes all3 modes.
- [x] Focused units pass after correcting new test assertions/initialization. The field-producer
      route revert causes E52017 load result vector<BFloat16,3> at both O0/O3; source restored.
      IRBuilder::emitFieldAddress intentionally produces the explicit pointer spelling.
- [x] Both cache orders pass24 exhaustive GPU controls; independent oracle checks100664064 words.
      PTX proves real field memory paths and declaration order. LLVM14 layouts qualify scalar/2/3/4
      guarded records and the mixed24/8 record. Runtime4/focused30 and6 original boundaries pass.
- [x] Full checkpoint and separate acceptance audit pass:1710 cells/1671 correct/39 unchanged
      unresolved, all1707 old outcomes exact and18 resolved histories retained. Units1063 IDs and
      semantics1129 IDs preserve every old result; material6 PTX/cubins remain exact256.
- [x] Close3885 raw artifacts/136 final snapshots/358 compact references; complete report/design/
      STATUS and prepare the accepted slice for its authorized local commit.

## Surprises and Discoveries

Research258's getter experiment failed its performance thresholds and was fully discarded. It
provides the required material-driven review; do not force another speculative optimization.
Research255 source controls first stop at device Ptr<H>, while frozen dynamic-dispatch-substandard-
float first stops at the unrelated FP8 record result A. Neither establishes a local W record test.
The frozen B implementation uses a BF2 field, but it is behind A and has separate by-value/reference
needs. Keep those original failure histories intact and make no claim of fixing that frozen workload.

## Decision Log

- 2026-09-25, parent: Select direct field access through local BF16 records as the next bounded
  capability. It composes255's qualified field layouts with256's exact vector load/store conversion.
  Whole-record transport and recursive storage admission would require broader representation work.
- Use an explicit finite, nonrecursive local-record predicate: integer scalar fields plus canonical
  BF16 scalar or vector2/3/4 fields, with at least one BF16 field. Existing integer field layout is
  reused; no record names, field positions, reconstructed semantic type or generic numeric widening.
  More field families remain unqualified in this newly admitted role.
- Preserve one canonical IR struct. NVVMTypeUse::Storage owns physical fields; ordinary value and
  global/resource classifications do not acquire the new permission. The role must be checked
  before type cache lookup. No provider ABI increment is planned; existing typed operations suffice.

## Outcomes and Retrospective

Prototype source fixture passes. Initial new units exposed two test issues: the fake separates
scalarStructFieldTypes from global structFieldTypes, and a brace-initialized readonly fixture
introduced a by-value constructor before the intended borrow. Correct the new unit inputs/assertion
target without changing production or existing tests; retain those attempts.

Full acceptance passes on the final source:1710 cells/1671 correct/39 unresolved/18 resolved histories.
The new fixture adds3 correct cells; every1707 old outcome remains exact. Full259/targeted233/cadence0
and rolling254 CUDA layout correctness,256 local BF vector capability,259 local BF record capability
are current after acceptance. Research258's discarded material experiment justifies the cadence
exception; reconsider material work before selecting the next bounded slice. Evidence closure and
local commit complete the handoff.

## Context and Current Pipeline

Consider `struct W { uint16_t prefix; vector<BFloat16,4> value; uint16_t suffix; };` and
`void replace(inout W record, vector<BFloat16,4> value) { record.value = value; }`. Checking/lowering
produces canonical IRStructType fields, BorrowInOutParam<W>, local Ptr<W> and FieldAddress operations.
These are valid semantic shapes. CUDA layout after254 gives W size12/alignment2, offsets0/2/10.
Research255 independently qualifies LLVM `{i16,[4 x i16],i16}` for this stored record. For BF2 the
native field instead requires alignment4 and W size12 with offsets0/4/8; BF3 W is10/2 at0/2/8.

The current local-helper pointer classifier admits bare BF vectors but not W. Field-address
selection expects an admitted helper/copyable/resource/physical-storage root. Generic helper-value
classification deliberately excludes recursive BF leaves. Preserve that exclusion: only the new
local storage root/field path gets permission. Bare vector field loads/stores should reuse256's
array/register conversion, not invent a record-valued representation or whole-record converter.

## Scope and Non-Goals

Production files: source/slang/slang-emit-nvvm-type-lowering.{h,cpp} and slang-emit-nvvm.cpp. Tests:
focused emitter unit coverage and tests/cuda/nvvm-bf16-local-records.slang, with one new discovery
source if accepted. Reuse existing helpers/maps, type layout and field-key lookup. No public API,
provider ABI41, checked AST/IR producer, global numeric/copyable/helper algebra, FP8, nested records,
record arrays, matrices, resource/device pointers, readonly references or exported CUDA helper ABI
changes. No whole-record make/extract/load/store/phi or by-value helper admission. Do not weaken a
fixture, frozen ID list, oracle, original diagnostic or unrelated gate. No push/system/driver changes.

## Architecture and Invariants

Local storage pointers must retain the exact one-operand Generic Ptr/OutParam/BorrowInOutParam
shapes already qualified for local helpers. Storage lowering recursively applies only to the
admitted direct fields. Prove provider field offsets/size/alignment against canonical CUDA layout
using the existing aggregate-layout walk; do not bypass it with guessed alloca alignment. Field
selection remains keyed by canonical IRStructKey. Reads/writes preserve root mutability and exact
pointee type, with BF3/BF4 whole-field vector/array transport handled at the existing leaf boundary.
A stored field's provider type must be independent of whether its register type was cached first.
The ordinary aggregate/helper/device/resource gates remain closed to W and its arrays/pointers.

## Interfaces and Dependencies

Native Ubuntu24.04, RelWithDebInfo, CUDA12.9.2/NVRTC12.9.86, LLVM14/provider41, L4SM89/driver580.126.09,
SM80. Follow the local slang-build skill and source build/nvvm-loop/slice-203-env.sh. Max4 CPU workers
across all owned work; GPU suites sequential and bounded30m. Baseline133 source/12 artifact/565
input hashes and18 pins are recorded in256 and restored by258. Compiler
1fc2311e9e0c332f2bff54d65dedb1a225218f740042a2c23c5210e76245c85f; provider
5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab.

## Milestones

1. Capture baseline before editing. Add an independent exact-bit source fixture for local guarded
   records at widths2/3/4, internal out initialization and inout replacement/readback. Start from
   the256 boundary encodings and include scalar BF16/integer-width neighbors if admitted. Retain
   original before outputs: NVRTC passes, NVVM O0/O3 reject the new local-record role.
2. Add the explicit local record classification and carry its Storage role through pointer lowering,
   local allocation/layout proof and field-address emission. Reuse the existing leaf conversions.
   Inventory every new helper/branch; explain the canonical producer and consumer ownership.
3. Add unit checks for actual stored field types/allocation alignment, separate LLVM field-offset
   qualification, and denied by-value, readonly,
   recursive/device/resource roles. Require source-level exact GPU results, not fake-builder success
   alone. Each new branch needs a failing test or a declared invariant; perform a small revert drill.
4. Build with CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4
   --target slangc slang-test render-test test-server. Keep full logs and exit codes. Run focused
   tests, then all-encoding source controls with both storage-first/value-first helper declaration
   orders:scalar plus3 vector widths ×2 orders ×3 modes (24 controls). Verify all65536 encodings under both permutation flags and
   every returned guard/observation/sentinel byte; prove PTX retains actual local field accesses/calls.
5. Replay all6 unchanged255 direct device Ptr<H> rejections. Run full units and semantic neighbors,
   runtime4, toolkit18/contracts6, full frozen452/1356 using census.slice-195.tsv, full discovery117/
   351 plus the new source3 cells, and material6 compile/assembly checks. Inspect exact identities
   and execution counts, every old outcome/diagnostic and all failure histories. New cells separate.
6. Independently audit byte oracles, cache-order representation and source/binary/input identity.
   Update compact result maps, completed plan, report, durable design and STATUS; close raw index
   and references before local acceptance/commit. Do not claim material runtime/performance.

## Validation and Acceptance

New focused local-record cells must pass independently expected complete outputs in all3 modes.
Every1707 old corpus outcome must remain exact unless an explicitly investigated improvement is
accepted with original failure history preserved. Require all1062 unit identities (1049 pass/13 skip)
and1129 semantic identities (1052 pass/77 skip), with new tests separate. Full checkpoint is required
because the backend's shared type/layout/address machinery changes. Preserve original255 Ptr<H>
stops, FP8 A result, texture and column-major gaps. Validate no missing/duplicate/ignored selected
cells and no stale executable/configuration. Reassess every material/backend/optimization cell.

## Failure and Recovery

Record failed/interrupted attempts. Trace any bad field/provider shape to its producer or canonical
role selection; do not add null/default guards or a second semantic representation. If ordinary
valid local field code necessarily requires whole-record transport, explicitly reassess this scope
before broadening; prefer stopping at a documented unsupported boundary to accidental admission.
Fix or revert regressions before accepting. Keep raw evidence and return to accepted source/artifacts
if the approach cannot be made principled. Stop GPU dispatches on device loss; never change drivers.

## Artifacts and Hand-Off

Raw roots build/nvvm-loop/slice-259-before and slice-259-after. Commit accepted production/tests,
completed plan/report, compact validation/census/discovery, design and STATUS. Raw binaries, complete
logs, generated controls, datasets, outputs and immutable snapshots remain ignored under build/.
