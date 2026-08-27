# Slice 32: Add exact scalar float32 device loads

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to ship with its implementation, overriding the repository's
default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs this exact raw CUDA copy kernel:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
```

The final linked IR contains two canonical AS1 float pointers, one float32 load, and one float32
store. Slang owns the exact source/type/access/availability policy. The existing generic provider
load/store callbacks own LLVM construction, so this slice grows neither V2 nor V3. Verified LLVM
and NVVM-2.0 text, differential PTX, matching-root `ptxas`, and CUDA runtime agreement establish
the result.

## Progress

- [x] (2026-08-27) Selected the direct float32 load boundary deliberately left by Slice 31.
- [x] (2026-08-27) Recorded the post-Slice-31 baseline: 201 registered NVVM tests, SHA-256
  `73434ac732eccaf42c9fad54ad2956b13aa5e2371e9e2e72d5fbbc2aaaf6e2e2`, Release 201/201, and
  Debug preservation 10/10.
- [x] (2026-08-27) Probed the final linked IR and reused the existing provider load/store contracts
  without ABI growth.
- [x] (2026-08-27) Admitted and emitted only canonical float32 loads, made fake load results
  indexed and type-aware, and retained adjacent memory/type boundaries.
- [x] (2026-08-27) Added real-builder, direct-topology, capability, PTX, `ptxas`, and runtime
  evidence; focused Slice 32 and preservation coverage passes 8/8.
- [x] (2026-08-27) Formatted, built the standalone provider plus Release/Debug tests, passed the
  full Release 207/207 and Debug 10/10 lanes, updated durable docs/plan, and completed self-review.

## Surprises and Discoveries

- Observation: Slice 31 already lowered float pointers and stores, and the provider's original
  `emitLoad` callback derives its result type from the typed pointer.
  Evidence: `_emitLoad` uses `CreateAlignedLoad(pointerType->getPointerElementType(), ...)`; the
  host currently stops the exact source only at first-pass E52017 `load result type`.
  Consequence: make this a host supported-subset expansion with no provider table callback or size
  growth.

- Observation: the fake represents every load with one untyped handle and historically classifies
  it as signed i32.
  Evidence: `_isFakeNVVMBuilderIntegerValue` accepts `FakeNVVMBuilderValueKind::Load`
  unconditionally, while `_isFakeNVVMBuilderFloatValue` does not.
  Consequence: record the pointee scalar kind when the generic load callback runs. Do not add a
  float-load callback, storage family, or source-name special case.

- Observation: the generic fake load handle also assumed there could be only one load in a module.
  Evidence: `_getFakeNVVMBuilderLoad()` returned one storage address and every load value reference
  used index zero.
  Consequence: index the existing generic load storage and result-kind records together. Existing
  one-load tests keep index zero, while future same-module loads no longer alias in fake evidence.

## Decision Log

- Decision: reuse Slice 31's `SCALAR_FLOAT32_ADD` provider feature together with established
  `SCALAR_MEMORY`; add no new feature bit.
  Rationale: that advertised prefix supplies the float type constructor, while the already
  negotiated generic load/store callbacks are type-polymorphic and require no new provider
  behavior. A Slice 31 provider therefore already satisfies the provider side of this graph.
  Date/author: 2026-08-27, Codex.
  Revisit when: a new float memory operation needs a callback or contract not represented by the
  existing scalar-memory API.

- Decision: support only a direct AS1 pointer load whose canonical result exactly equals the
  pointer pointee and is immediately available to established consumers.
  Rationale: pointer offsets, arrays, resources, local/shared storage, volatile/atomic operations,
  helper values, phis, and other types have separate producer or ABI contracts.
  Date/author: 2026-08-27, Codex.
  Revisit when: a later bounded slice selects one of those producers.

- Decision: make fake load result classification derive from the pointer record.
  Rationale: this mirrors the typed provider contract and scales to integer and float loads without
  parallel callbacks or semantic reconstruction from the source fixture.
  Date/author: 2026-08-27, Codex.
  Revisit when: a new pointer producer can yield a supported scalar kind that the current exact
  parameter/pointer-offset/array/resource classification cannot determine.

## Outcomes and Retrospective

The exact source produces two canonical AS1 `Ptr<Float>` parameters, one `kIROp_Load : Float`, and
one store of that load through the destination. Preflight now admits float alongside signed i32 in
the existing load arm, requires the Slice 31 float feature plus scalar memory, and reuses the exact
pointee/availability/dominance validator. Emission changes only the diagnostic operation name;
both scalar types call the same aligned generic load callback.

The fake records indexed generic load handles plus result scalar kind. Direct topology observes one
float construction, one shared float-pointer construction for read/read-write source types,
`[FloatPointer, FloatPointer]`, source parameter 1 feeding float load 0, and that load feeding the
store through destination parameter 0 at alignment 4. The existing integer copy test still passes,
and integer pointer-offset/array/resource producers retain integer load classification.

The real builder verifies both LLVM and audited NVVM-2.0 text with exactly one `load float`, one
`store float`, two `align 4` occurrences, kernel metadata, and no `fadd`. Direct NVVM and NVRTC agree
on `[64, 64]`, one global 32-bit load/store, and no float add. Matching-root CUDA 12.9 `ptxas`
accepts both. On the RTX 5090, both routes copy `3.75`, `-7.5`, `0`, and `1024` exactly.

Six registered tests raise the prefix from 201 to 207 names. Its exact sorted LF-terminated
SHA-256 is `5e9c007c59d45c4db5bf9724e6b76c039455d342330f06b8aa68cd2e5eb2316b`. Focused Slice 32 plus
integer/negative preservation passes 8/8; the full Release prefix passes 207/207; Debug preservation
passes 10/10; and the Release/Debug test builds plus standalone Release provider build succeed.

## Context and Current Pipeline

`validateIRForDirectNVVM` walks the selected entry and reachable helpers twice. Its first pass
currently admits only signed-i32 `kIROp_Load`, even though float parameters/pointers and stores are
legal. Its second pass already calls `_validatePointerValue` with the load's exact result type, so
the canonical pointee/result relationship, access, availability, and dominance checks are shared.

`emitNVVMIRFromLinkedIR` maps the load pointer, calls the generic `NVVMIRBuilder::emitLoad` with
four-byte alignment, and maps the result for the following store. The real provider validates an
available module-owned typed pointer at an unterminated insertion point and constructs a load of
its pointee type. The audited NVVM-2.0 text writer has no float-load rewrite.

## Scope and Non-Goals

In scope are exact canonical float32 AS1 read/read-write pointers, one direct load, one established
float32 store, four-byte alignment, type-aware generic fake load evidence, real LLVM/NVVM text,
differential PTX, matching-root `ptxas`, runtime comparison, and capability-gating evidence.

Out of scope are loads through pointer offsets, arrays, raw resources, locals, globals, or shared
memory; volatile or atomic loads; float constants, helpers, phis, loops, casts, arithmetic beyond
Slice 31 addition, half/double, vectors/matrices/aggregates, new address spaces, new provider API,
text rewrites, optimization/performance claims, and changing read-only store legality.

## Architecture and Invariants

First-pass preflight accepts a load result only when it is canonical signed i32 or float32. Float32
loads require both the existing float32 and scalar-memory feature bits. Second-pass preflight keeps
one source of truth: `_validatePointerValue` proves the pointer is an exact supported device scalar
pointer, its canonical pointee equals the load result, and the pointer is available/dominating.

Emission remains generic. The operation name used for diagnostics reflects the result type, but
both integer and float loads call the same facade and provider callback at alignment 4. The provider
must not gain a float-specific branch beyond its existing typed-pointer construction.

The fake infers `Integer` versus `Float` from the pointer value's recorded parameter type (and keeps
established integer addressing producers classified as integer). Its load handle remains a generic
load value; integer and float value validators consult the recorded result kind. Store topology
must prove the load result, not a reconstructed parameter or scalar operation, is consumed.

## Interfaces and Dependencies

Expected production change is confined to `source/slang/slang-emit-nvvm.cpp`. Test/support changes
are expected in `tools/slang-unit-test/unit-test-nvvm-support.h`, builder/emitter/integration test
files, and durable NVVM design/ledger documents. No public API, provider table, feature count,
provider implementation, third-party dependency, target, or packaging rule changes.

## Milestones

1. Confirm the 201-name baseline and exact E52017 float-load stop before provider discovery.
2. Make the generic fake load value type-aware without changing established integer identities.
3. Admit canonical float32 loads in both validation passes and use a type-specific diagnostic name
   while retaining the generic emission callback.
4. Add real-builder text evidence for exact `load float`/`store float`, and direct topology proving
   two shared float-pointer types, one float load, and one result-consuming store.
5. Prove missing float capability stops before module/type/load/libNVVM work; retain half/double,
   pointer/type mismatch, non-device, aggregate, and other memory boundaries.
6. Compare NVVM/NVRTC `[64, 64]`, global 32-bit load/store with no float add; assemble both and run
   exact finite values including negative and zero.
7. Format, build, run the complete Release prefix and Debug preservation, update design/ledger and
   outcomes, perform the input-shape audit, and commit plan plus implementation as `slice 32`.

## Validation and Acceptance

Build the standalone Release provider plus Release/Debug `slang-unit-test` and `slang-test` targets
outside the sandbox. Run focused real-builder, direct, missing-feature, differential PTX, `ptxas`,
runtime, and adjacent-negative tests; then the complete `slang-unit-test-tool/nvvm` prefix and the
established Debug 10-test preservation set. Enumerate and hash the exact sorted registered prefix.

Acceptance requires verified LLVM and NVVM-2.0 text with one aligned `load float` and `store float`,
no `fadd`; matching `[64, 64]` PTX parameter widths; global 32-bit load/store in both routes;
matching-root `ptxas`; CUDA runtime agreement; no provider ABI/export/feature growth; unchanged
integer load tests; formatted code; completed self-review; and `git diff --check` success.

## Self-Review and Input-Shape Audit

The production diff adds no helper, fallback, custom equivalence, or representation repair. The
only new branch is the result-type dispatch in the existing load validator and its diagnostic label.
The exact shape is canonical final linked IR produced by dereferencing a raw device `Ptr<float>`;
the source type checker and IR builder already preserve the identical float pointee/result type.
This is intentionally valid input, not an accidental spelling. The backend preflight owns its
supported subset, and the existing pointer validator remains the source of truth for canonical
pointee identity, access, availability, and dominance.

The test helper inventory contains `_getFakeNVVMBuilderLoadIndex`, `_populateFloat32CopyKernel`, and
`_runFloat32CopyKernel`. The first replaces the fake's singleton alias with one generic indexed
identity and is used by all fake value classification; it does not walk or reconstruct source IR.
The latter two own repeated provider-module and CUDA launch mechanics for the new exact fixture.
They do not alter production behavior or introduce per-emitter API wrappers. Removing the
production type dispatch restores the measured E52017 failure and makes the direct, PTX, and
runtime tests fail at the intended owner.

## Failure and Recovery

If the source produces a cast, helper, offset, or other unexpected producer, narrow the fixture or
split it into a later slice. If libNVVM rejects text, inspect provider LLVM and audited NVVM output
before adding any rewrite. The host preflight check is the rollback boundary; removing float from
the load-result classifier restores Slice 31 behavior without touching provider compatibility.

Do not delete or stage `external/slang-binaries/`. Builds/tests write only ignored build or temporary
artifacts and are safe to rerun.

## Artifacts and Hand-Off

The retained evidence is the exact linked/fake load-store graph, LLVM/NVVM assertions, `[64, 64]`
PTX ABI and load/store facts, runtime values `3.75`, `-7.5`, `0`, and `1024`, missing-feature and
adjacent-negative evidence, 207-name hash
`5e9c007c59d45c4db5bf9724e6b76c039455d342330f06b8aa68cd2e5eb2316b`, and the self-review above.
Release 207/207 and Debug 10/10 are complete. Commit the plan with Slice 32 and continue to the next
bounded capability unless blocked.
