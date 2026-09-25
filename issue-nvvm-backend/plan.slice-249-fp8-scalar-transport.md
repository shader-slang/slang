# Admit canonical FP8 scalar transport

This ExecPlan follows `.agent/PLANS.md`; the maintainer requires this completed plan and report
committed with the NVVM slice. Worker is sole checkout writer; parent accepts and commits.

## Purpose and Observable Result

Compile and execute the frozen `hlsl-intrinsic/substandard-fp-folding.slang#cuda-1` in direct NVVM
O0/O3 with its unchanged CHECK prefixes58,61,1628 and exact captured output58,61,16288. Also transport every byte of both FP8 formats through
internal helper parameters/results, selection and branch/phi, proving bit preservation on GPU.

## Progress

- [x] 2026-09-25: Read workflow/status, research243, producer244 and native build skill.
- [x] 2026-09-25: Establish clean branch nvvm-backend at 7263a71a8761f61ee04eede22d69c75dab4d77d8.
- [x] 2026-09-25: Smoke4 passes; frozen1036-word fixture passes NVRTC and rejects FP8 helper result in both direct modes.
- [x] 2026-09-25: Implemented ABI41 distinct descriptors/roles/finite bits; corrected signed builder argument representation. Focused6 and transport replay3 pass, as do provider/preflight units.
- [x] 2026-09-25: Final runtime4, focused6, 512 main units plus literal supplement1 (one inherited skip), toolkit18, contracts6, full frozen/discovery and complex6 all pass their preservation gates.
- [x] 2026-09-25: Helper/input-shape audit, report, exact histories, snapshots and output audit complete; ready for independent parent acceptance and local commit.

- [x] 2026-09-25: Independent parent audit passed: 1,654 old passes retained, two resolutions, three additions; exact 10,866 final words and all evidence identities verified. Accepted for local commit.

## Surprises and Discoveries

Research243 qualifies physical i8 and all256 encoding transport; producer244 repairs finite/subnormal
literal roundtrips. Overflow NaN/infinity policy still differs from CUDA SATFINITE and is excluded.
The first focused GPU run rejected negative finite constant bytes because getIntegerConstant
requires signed values in the destination width. Pass bitCast<int8_t>(bits), just as ordinary UInt8
materialization preserves its high-bit encodings through that API; no literal policy changes.
Both new contract units (including22 preflight negatives) pass on the first candidate.

## Decision Log

2026-09-25: Select narrow transport rather than runtime conversion or texture metadata ABI. The frozen
folding cell needs only exact1.25 constants plus UInt8 bitcasts and internal helper parameters.
Generic equal-width bitcast also admits E4M3/E5M2 reinterpretation; qualify all256 bytes in each direction with supplemental whole-buffer GPU replay. Use role-specific admission, analogous to BF16 vector internal roles, leaving recursive storage
predicates unchanged. Scalar same-width signed/unsigned8 bitcasts share the existing generic family.

## Outcomes and Retrospective

Implementation, validation and independent parent acceptance are complete. All1,695 old
cells are compared exactly: all1,654 old passes survive, two old folding cells resolve, and the new
fixture adds three passes. Final1,698cells/1,659correct/39unresolved; all41 old failure histories and16
resolved histories are retained (18 resolved histories now). Supplemental phi and signed-byte helper
controls qualify the actual SSA join boundary and all256 encodings. Full buffer audit checks10,866
final words. Original literal-overflow policy remains visible and excluded. The first failed constant
argument attempt is preserved with118 exact source snapshots. A separate unchanged BF16 negative
constant API rejection is reproduced on accepted244 and candidate249 as the next bounded handoff.
Parent audit verifies exact preservation, independently derived outputs, gate identities, unit IDs,
snapshots and artifact hashes. The accepted slice is ready for its local commit.

## Context and Current Pipeline

`FloatE4M3(1.25)` and `FloatE5M2(1.25)` become canonical IRFloatLit through SCCP and
IRBuilder::getFloatValue. `pack8<T>` takes that scalar and produces scalar BitCast to UInt8.
Current `_validateNVVMHelperSignature` rejects the FP8 parameter. Type lowering and typed semantic
catalog own legitimate register admission; no source reconstruction or runtime numeric conversion
is needed. Research243 raw LLVM proves the internal representation separately from production.

## Scope and Non-Goals

Allow both distinct formats as scalar register/internal by-value parameters/results, finite canonical
literals, same-width8 integer and cross-format bitcasts, select and phi. Exclude runtime numeric casts, arithmetic,
vectors, local/storage/pointers/resources/aggregates, external helper ABI, dynamic objects and BF
vector storage. Do not harmonize overflow or saturate literals. Nonfinite literal support requires
separate policy and remains diagnosed. Stop at the next independent blocker with a minimal trace.

## Architecture and Invariants

Canonical FP8 semantic identity remains distinct even though physical representation is i8. Do not
admit FP8 to IEEE numeric classifiers or recursive helper/copyable predicates. `_getNVVMSemanticType`,
NVVMTypeInfo role admission, canonical constant materialization and provider `_getSemanticLLVMType`
are the owning boundaries. Existing BitReinterpret and Select recipes suffice; phi is generic.

## Interfaces and Dependencies

Provider ABI40 ->41 for added semantic type kinds. Native Linux RelWithDebInfo, LLVM14, CUDA12.9,
SM80 target on L4SM89; source inspected slice-203-env.sh (overrides setup Debug). Maximum4 CPU workers,
sequential GPU suites,30-minute bounds. No driver/system/reboot/push changes.

## Milestones

1. Capture accepted246 source/artifact/input identities before modifications; preserve old research.
2. Add focused runnable fixture and capture NVRTC pass/direct failures before production edits.
3. Change API/catalog, emitter/type-lowering, provider and focused unit contracts at owning layers.
4. Build with `cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server` and provider target as configured.
5. Full final checkpoint: runtime4, focused fixture, NVVM/routing/reporter units, toolkit18, contracts6,
   frozen452x3 explicit census195, discovery113x3 plus one new source, complex6 compile/assembly.
6. Compare all1695 old five-field outcomes against246, preserving1654 passes and all41 histories;
   permit only two demonstrated frozen resolutions, explicitly report new3 discovery cells.

## Validation and Acceptance

Use workflow commands with output `build/nvvm-loop/slice-249-after`. Before evidence lives beside it
in `slice-249-before`. Full checkpoint is mandatory for shared type/provider/API/corpus changes.
Freeze all256 expected bytes and both branch choices independently in source/oracle, run NVRTC O3
and NVVM O0/O3. Capture exact source, artifacts, runtime inputs and index raw outputs. Negative units
cover FP8 arithmetic/casts/vector/storage/local pointer/aggregate/external ABI and descriptor mismatch.
No missing/ignored runs count as passes. Acceptance records latest full249, targeted233, cadence0.

## Failure and Recovery

Preserve failed attempts; use fresh output directories. Stop GPU work on device loss. If canonical
literal API cannot faithfully recover finite checked values, report the precise producer break to
parent before promotion. Fix or revert regressions; never reset baselines. Do not investigate a second
independent feature. Worker does not commit.

## Artifacts and Hand-Off

Completed plan, five-part report, runtime-validation.slice-249.json, census/discovery-census249,
explicit discovery addition, design contract and STATUS draft. Raw scripts/identity/index/snapshots
stay ignored. Return <=500 words with changed behavior, validation counts/deltas and remaining limits.
