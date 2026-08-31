# Generalize canonical UserPointer address-space transport

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM correctly transports canonical `UserPointer<T>` values from their
producer-proven physical address space into the generic executable representation required by
pointer-bearing helper values. The two healthy discovery workloads currently blocked by the same
provider operation must compare correctly at O0 and O3, while identity, mismatched-pointee, and
unsupported-provenance casts retain deterministic rejection.

## Progress

- [x] (2026-08-31) Completed and committed Slice 164 as `f27bc4a95`; frozen v1 reached
  396/400/396 and discovery remained 64/64/64 O0/O3/both.
- [x] (2026-08-31) Ranked remaining exact failures and selected the two-row healthy discovery
  `UserPointer` provider cluster over a one-healthy-row aggregate field-address cluster.
- [x] (2026-08-31) Dumped both final linked IR shapes and instrumented the provider long enough to
  prove that each reported global-to-generic conversion was actually an invalid AS0-to-AS0 cast.
- [x] (2026-08-31) Corrected recursive conventional-global type lowering so Storage outranks a
  pointer-bearing struct's helper-value classification; provider ABI revision 31 is unchanged.
- [x] (2026-08-31) Promoted both workloads after O0/O3 differential probes and passed the selected
  433-test prefix plus focused positive and adjacent-negative provider coverage.
- [x] (2026-09-01) Preserved frozen v1 at 396/400/396, advanced discovery to 66/66/66 with exactly
  two gains and no losses, assembled all 28 measurement gates, and completed durable records and
  self-review.
- [x] (2026-09-01) Attempted the repository formatter outside the sandbox; the WSL environment
  lacks gersemi, clang-format, prettier, and shfmt. Applied the available Windows clang-format to
  the changed C++ range and passed `git diff --check`.

## Surprises and Discoveries

- `force-inline-array` loads `UserPointer<Int>` from the synthesized conventional global, stores it
  into the pointer-bearing local `BufferSink`, and then carries that generic helper value into
  `BufferSink.store`.
- `groupshared-ptr-of-device` stores a device pointer in a groupshared array of `UserPointer<Int>`.
  Slice 155 proved the shared aggregate storage and addressing; the remaining failure is the same
  reported provider conversion as the conventional-global helper path.
- The provider already exposes a generic pointer address-space cast and has focused AS1-to-AS0
  coverage. The real rows therefore indicate a representation/precondition discrepancy that must
  be identified before changing the ABI or adding another operation.
- Temporary provider instrumentation showed that both failing values and requested result types
  were LLVM AS0 `i32*`, with identical pointees and valid insertion points. The provider's identity-
  cast rejection was therefore correct; the compiler had lost AS1 before requesting the cast.
- `_lowerStructType` allowed a pointer-bearing helper-struct classification to override an explicit
  `NVVMTypeUse::Storage`. The synthesized conventional `GlobalParams` struct consequently lowered
  its `UserPointer<Int>` field as a generic AS0 helper value even though the global itself is
  producer-proven constant-address-space storage whose pointer payload denotes device AS1.
- The fake-emitter struct model intentionally cannot represent this cross-layer shape: it admits a
  pointer-bearing `GlobalParams` only when a resource leaf is present, while copyable fake structs
  exclude pointer leaves. A synthetic fixture was removed rather than broadening unrelated test
  machinery; the two real-provider differential lanes own the integration proof.

## Decision Log

- Decision: make Slice 165 an audit of canonical UserPointer physical-to-executable transport.
  Rationale: it blocks two healthy discovery workloads with different storage combinations and
  reaches a shared provider operation after all compiler preflight checks. Resolving one exact
  representation invariant should unlock more real pointer behavior than a fixture-specific
  downstream patch.
  Date/author: 2026-08-31, Codex.
- Decision: preserve `NVVMTypeUse::Storage` recursively before considering helper-struct shape.
  Rationale: storage use is explicit producer provenance, while helper-struct classification only
  describes a possible value representation. This restores the documented physical AS1 field and
  leaves the existing executable-boundary conversion responsible for the single AS1-to-AS0 cast.
  Date/author: 2026-08-31, Codex.
- Decision: retain the provider's identity-cast rejection and the compiler's global-pointer
  provenance map.
  Rationale: accepting AS0-to-AS0 would mask a compiler representation error, while removing
  provenance would lose the physical role needed by entry parameters and ordinary global access.
  The existing revision-31 generic cast operation already expresses the required canonical work.
  Date/author: 2026-08-31, Codex.
- Decision: use the two real workloads as permanent regression proof instead of widening the fake
  aggregate model solely for this slice.
  Rationale: each workload crosses the actual Slang/type-lowering/provider boundary and compares
  against a stable native reference. The attempted fake shape was not otherwise canonical in that
  harness and would have created unrelated test-only policy.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Direct NVVM now preserves conventional-global `UserPointer<T>` fields as physical LLVM AS1 values
until they cross into a pointer-bearing executable helper value, where the existing typed provider
operation performs exactly one AS1-to-AS0 conversion. The same invariant lets a device pointer be
stored in and loaded from canonical groupshared pointer-bearing storage.

Both discovery workloads become correct at O0 and O3. Frozen corpus v1 remains exactly 452/427 at
396/400/396 O0/O3/both with zero old-correct loss. Discovery remains exactly 82/72 and advances
from 64/64/64 to 66/66/66 with exactly the two intended gains and no loss. The selected prefix
passes 433/433. All 28 representative gates produce five measurement rows and assembled cubins,
for 140/140 total, across NVRTC O3, direct O0 SM70, and direct O3 SM70/SM80/SM90.

The resulting change is smaller than the initial provider-facing symptom suggested: one recursive
type-use precedence correction, four permanent real-workload lanes, and evidence artifacts. No
provider callback, ABI revision, fixture-name check, syntax reconstruction, compatibility fallback,
or downstream repair survives the slice.

## Context and Current Pipeline

`UserPointer<T>` is canonical final IR `Ptr<T, ReadWrite, UserPointer, DefaultLayout>`. Kernel and
conventional-global producers use an LLVM global-memory representation for ordinary access, while
pointer-bearing helper values require LLVM generic pointers because local addresses are also valid.
`_getLoweredNVVMHelperValue` consults producer-proven provenance and calls
`_emitNVVMExecutableUserPointer` at that boundary. The latter lowers the canonical value type and
uses the revision-31 generic `emitPointerAddressSpaceCast` provider callback.

The selected discovery rows are:

- `bugs/force-inline-array.slang#discovery-1`;
- `language-feature/pointer/groupshared-ptr-of-device.slang#discovery-1`.

Both native CUDA/NVRTC references are healthy. Direct O0 and O3 currently fail with E52018:
`LLVM 14 NVVM IR builder operation 'global-to-generic UserPointer conversion' failed with result
-2147024809`.

## Scope and Non-Goals

In scope are the exact canonical physical and executable representations of selected copyable
`UserPointer<T>` values; conventional-global, local-helper, and shared-storage boundaries proved by
the two targets; existing generic pointer operations where sufficient; focused provider and fake
compiler tests; permanent O0/O3 lanes after differential correctness; both separate corpus
snapshots; measurement gates; and durable documentation.

Out of scope are arbitrary pointer/address-space casts, opaque or mismatched pointees, unsupported
pointer qualifiers/layouts, pointer-to-interface or double-indirect helper ABIs, resource-field
pointers, provider callbacks without a demonstrated generic-interface gap, corpus-v2 activation,
fixture-name checks, syntax reconstruction, and downstream repair of malformed IR.

## Architecture and Invariants

- Canonical Slang pointer type and producer provenance jointly determine the physical provider
  address space; the same source spelling alone cannot prove one.
- A pointer-bearing helper value has one generic executable representation, independent of whether
  its valid source is global, shared, or local.
- Address-space transport preserves the exact lowered pointee representation and pointer bits.
- Identity casts, mismatched pointees, foreign handles, unsupported address spaces, and values not
  available at the insertion point remain rejected before provider mutation.
- Compiler classification uses exact producers and existing value/provenance maps; it does not
  walk arbitrary operand graphs or infer from fixture names.
- Provider ABI revision 31 remains unchanged unless the audit proves the current generic callback
  cannot express a canonical required cast.

## Interfaces and Dependencies

Expected audit points are `source/slang/slang-emit-nvvm.cpp`,
`source/slang/slang-emit-nvvm-type-lowering.cpp`, and the revision-31
`emitPointerAddressSpaceCast` implementation in `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`.
Focused real-provider coverage lives in `tools/slang-unit-test/unit-test-nvvm-builder.cpp`; exact
compiler behavior uses the existing fake emitter infrastructure. CUDA 12.9/libNVVM and the isolated
LLVM 14 provider supply end-to-end evidence.

## Milestones

1. Dump the final linked IR for both targets, trace each pointer from canonical producer through
   physical storage to the helper boundary, and identify the exact failed provider predicate.
2. Correct the smallest source-of-truth representation or generic cast contract and add focused
   positive plus adjacent-negative coverage.
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

If correcting the first failed precondition exposes a distinct canonical blocker, inventory it and
include it only when the same UserPointer transport invariant owns it. Stop before widening
unrelated pointer ABIs, inventing an opaque pointer representation, or weakening provider ownership
checks. Raw IR/PTX and probe logs remain under ignored `build/nvvm-census` paths.

## Artifacts and Hand-Off

Retain the completed plan with implementation under the user's established experimental-workflow
exception. Keep refreshed frozen/discovery TSV and Pareto JSON, a measurement manifest, five-part
report, promoted lanes, and design/ledger updates. Keep raw dumps, generated PTX, cubins, and logs
under `build/`.
