# Generalize canonical Float64 vector algebra

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM accepts the bounded family of ordinary Float64 vector operations
that canonical numeric legalization leaves in the two healthy frozen-v1 double vector/matrix
workloads. The provider must express those operations through its existing generic typed value
interface, both workloads must compare correctly at O0 and O3 before promotion, and unsupported
adjacent widths or lane counts must retain deterministic preflight diagnostics.

## Progress

- [x] (2026-08-31) Completed and committed Slice 163 as `831d9f59e`; frozen v1 reached
  394/398/394 and discovery remained 64/64/64 O0/O3/both.
- [x] (2026-08-31) Ranked exact remaining failures and selected the two-row canonical
  `vector<double,N>` algebra cluster for an audit-first reusable numeric widening.
- [x] (2026-08-31) Dumped and inventoried final linked IR for both workloads, tracing the retained
  fixed Float64 vectors to numeric and matrix legalization.
- [x] (2026-08-31) Widened the existing generic selected-float invariant to Float64 vectors of two
  through four lanes and added real-provider, fake-emitter, and adjacent-negative coverage.
- [x] (2026-08-31) Probed and promoted both workloads at O0/O3; the exact frozen corpus reached
  396/400/396 with two gains and zero old-correct loss, while discovery remained 64/64/64.
- [x] (2026-08-31) Refreshed 26 representative measurement gates and durable documentation,
  completed self-review, and prepared the exact Slice 164 commit.

## Surprises and Discoveries

- `NVVMSemantics::isSelectedFloatValue` intentionally accepts Float16/Float32 vectors of one to
  four lanes but restricts Float64 to scalar. This is the first rejection boundary for the two
  selected healthy workloads; the generic LLVM provider already represents fixed numeric vectors.
- The frozen census groups these rows under the broad `preflight-other` label. Their exact first
  shapes are `add: vector<double,2> -> vector<double,2>` and
  `add: vector<double,3> -> vector<double,3>` from canonical linked IR.
- The retained matrix representation is `Array<Vec(Double, 2), 2>`. Matrix legalization leaves
  ordinary row-vector add, subtract, and multiply operations, while scalar-only `abs`, `min`,
  `max`, `sign`, and reciprocal work has already been scalarized into helper loops.
- A first synthetic compiler test used ordinary scalar entry parameters and therefore reached the
  unrelated unsupported launch-parameter ABI before numeric emission. Recasting it as a
  conventional dispatch-ID entry with `RWStructuredBuffer<double>` isolates the intended typed
  operation family.
- The fake typed-operation validator treated every non-half floating vector as Float32. Float64
  vector coverage exposed that test-only classification gap; the validator now mirrors the exact
  16/32/64-bit descriptor kinds used by the compiler and provider.
- The discovery runner's `--frozen-v1` option checks the original Slice 146 historical contract,
  not an arbitrary recent frozen snapshot. The discovery run therefore used its default immutable
  frozen identity check and retained the separate Slice 164 frozen result.

## Decision Log

- Decision: make Slice 164 an inventory-driven Float64 vector-family slice rather than adding only
  vector addition.
  Rationale: both real workloads contain longer vector/matrix algebra pipelines, and the reusable
  invariant is whether a conventional component-wise Float64 vector operation is representable,
  not whether one fixture happens to begin with `add`.
  Date/author: 2026-08-31, Codex.
- Decision: widen `isSelectedFloatValue` instead of adding operation-specific Float64-vector rows.
  Rationale: every admitted operation already requires exact result/operand kinds, widths, lane
  relations, and operation-family rules. The selected-type predicate was the sole artificial
  restriction, and LLVM's existing generic fixed-vector operations already express the inventory.
  Date/author: 2026-08-31, Codex.
- Decision: keep vector `min`/`max` and other scalar libdevice contracts rejected.
  Rationale: widening the selected value representation must not invent vector libdevice ABIs;
  canonical legalization has already scalarized those operations in the selected workloads.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Slice 164 generalizes ordinary typed numeric operations to fixed Float64 vectors of two through
four lanes without changing the provider interface or ABI revision 31. The two selected healthy
frozen-v1 workloads now compare correctly through direct NVVM at O0 and O3 and have permanent
regression lanes. Frozen corpus v1 remains exactly 452 workloads/427 healthy references and moves
from 394/398/394 to 396/400/396 O0/O3/both, with exactly two gains and no old-correct loss.
Discovery remains exactly 82/72 and 64/64/64, also without loss. The selected NVVM unit prefix
passes 433/433.

All 26 representative gates produced PTX and assembled cubins with CUDA 12.9 for SM70, SM80, and
SM90, yielding 130 measurement rows. At SM70, the vector workload measured 278.0 ms and 3,538-byte
PTX through direct O3 versus 429.6 ms and 11,410 bytes through NVRTC O3; the matrix-row workload
measured 304.5 ms and 4,893-byte PTX versus 461.5 ms and 12,828 bytes. These single-repetition
numbers remain exploratory rather than a controlled performance claim.

The self-review found no new production helper, fallback, callback, or shape-specific branch. The
one production edit broadens the established selected-float predicate. Exact operation-family
checks still reject mixed types, scalar-only libdevice operations, and vectors wider than four
lanes. The fake Double classification survives because it corrects the test harness to match the
typed descriptor's existing semantic source of truth and is directly exercised by the new fake
compiler test.

## Context and Current Pipeline

Numeric and matrix legalization produces fixed-width vector values in final linked IR. The direct
emitter maps ordinary arithmetic through `_getNVVMValueOperation`, describes result and operand
types with `SlangNVVMValueTypeDesc`, and asks `NVVMSemantics::resolveValueOperationFamily` for the
exact operation family before provider mutation. The LLVM 14 provider consumes the same generic
descriptor in `_emitCatalogOperation` and emits fixed-vector LLVM operations.

The selected frozen-v1 rows are:

- `hlsl-intrinsic/matrix-double-reduced-intrinsic.slang#cuda-1`;
- `hlsl-intrinsic/vector-double-reduced-intrinsic.slang#cuda-1`.

Their native CUDA/NVRTC references are healthy. Both direct modes currently stop in Slang NVVM
preflight on the first Float64 vector addition, before the provider is created.

## Scope and Non-Goals

In scope are fixed Float64 vectors of two to four lanes; ordinary component-wise operations proved
by final linked IR; existing scalar-broadcast rules when the exact types match; focused provider
serialization and compiler-descriptor tests; permanent O0/O3 lanes after differential correctness;
both separate corpus snapshots; representative measurement gates; and durable documentation.

Out of scope are arbitrary vector widths, Float64 wave intrinsics, matrix types that survive
numeric legalization, aggregate ABI changes, libdevice vector ABI invention, operation-name or
fixture-name checks, downstream patches for malformed IR, provider callbacks, provider ABI
revision changes, corpus-v2 activation, and unrelated remaining failures exposed after the selected
family is complete.

## Architecture and Invariants

- Admission is by a generic typed family over exact kind, bit width, and lane count, not source
  spelling or fixture identity.
- Float64 vectors use the same one-to-four-lane bound already established for Float16/Float32 and
  integer vectors; scalar behavior remains unchanged.
- Operations whose LLVM or libdevice contract is scalar-only are not admitted merely because
  their operand happens to be a supported vector type.
- Component-wise result and operand lane relationships continue to use the shared broadcast and
  exact-element-type checks.
- Unsupported lane counts, mixed widths, and non-component-wise shapes fail during preflight.
- Provider ABI revision 31 and both corpus identities remain unchanged unless the audit proves a
  concrete operation that the generic callback cannot express.

## Interfaces and Dependencies

Expected production changes are in `source/compiler-core/slang-nvvm-semantic-catalog.h`, with
provider changes only if an already-generic operation mishandles a proved vector descriptor.
Focused real-provider tests live in `tools/slang-unit-test/unit-test-nvvm-builder.cpp`; exact fake
compiler descriptors use the existing NVVM emitter test infrastructure. CUDA 12.9/libNVVM and the
isolated LLVM 14 provider supply end-to-end evidence.

## Milestones

1. Dump final linked IR for both targets and enumerate the complete Float64 vector operation set,
   including any scalar-broadcast, comparison, select, construct, or extraction shapes.
2. Widen the smallest existing generic type/operation families that exactly cover that inventory;
   add focused positive serialization and negative boundary coverage.
3. Build and probe both real targets at O0/O3. Retain only semantics that compile, assemble, and
   match the stable native reference.
4. Promote useful representatives, run the selected prefix and exact corpora, refresh measurement
   evidence, and complete the producer/input-shape self-review.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
focused fake and real-provider coverage; differential correctness for every promoted workload at
O0 and O3; zero old-correct regression; the selected NVVM unit prefix; frozen identity 452/427;
discovery identity 82/72; separate corpus metrics and Pareto artifacts; direct-O3 PTX assembly for
all representative gates at SM70, SM80, and SM90; formatting attempt; `git diff --check`; artifact
integrity; and an exact staged-file audit excluding `external/slang-binaries/`.

## Failure and Recovery

If widening the first family exposes a distinct unsupported operation, record its exact canonical
shape and decide whether it belongs to the same conventional component-wise invariant. Stop before
libdevice-vector emulation, ABI changes, or unrelated aggregate/resource work. Raw IR/PTX and probe
logs remain under ignored `build/nvvm-census` paths so repeated probes do not affect repository
state.

## Artifacts and Hand-Off

Retain the completed plan with implementation under the user's established experimental-workflow
exception. Keep refreshed frozen/discovery TSV and Pareto JSON, a measurement manifest, five-part
report, promoted lanes, and design/ledger updates. Keep raw dumps, generated PTX, cubins, and logs
under `build/`.
