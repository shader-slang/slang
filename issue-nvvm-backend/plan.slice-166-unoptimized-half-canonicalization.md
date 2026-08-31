# Make unoptimized Half IR acceptable to libNVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM O0 either compiles and runs the canonical native-Half value operations
already supported at O3, or records a precise external-contract blocker if CUDA 12.9 libNVVM cannot
accept any principled bounded canonicalization. Success is observable when the four healthy frozen-
v1 Half workloads currently correct only at O3 also compare correctly at O0 without changing their
O3 representation or weakening unsupported-shape diagnostics.

## Progress

- [x] (2026-09-01) Completed and committed Slice 165 as `3041b1d17`; frozen v1 remains
  396/400/396 and discovery advances to 66/66/66 O0/O3/both.
- [x] (2026-09-01) Re-ranked both exact corpora and selected the five-row O0 provider Half cluster,
  which contains four healthy frozen-v1 workloads and one unhealthy-reference workload.
- [x] (2026-09-01) Captured byte-identical O0/O3 NVVM assembly and reduced the provider failure to
  runtime `insertelement <N x half>` for two through four lanes; neighboring Half operations pass.
- [x] (2026-09-01) Proved with direct libNVVM probes that exact i16 lane construction plus one
  vector bitcast is sufficient at O0 and O3 without running any optimization pass.
- [x] (2026-09-01) Centralized provider vector construction, routed generic, broadcast, and surface
  producers through it, added exact serialization checks, and passed ten promoted real lanes.
- [x] (2026-09-01) Advanced frozen v1 to 400/400/400 with exactly four healthy gains and zero loss,
  preserved discovery at 66/66/66, assembled 30 measurement gates, and completed durable records.
- [x] (2026-09-01) Rebuilt the formatted provider and unit-test module, reran the selected prefix at
  433/433 and promoted fixtures at 10/10, and passed `git diff --check`. The repository formatter
  was attempted outside the sandbox but its Bash environment lacks gersemi, clang-format,
  prettier, and shfmt; Windows clang-format 21.1.8 formatted only the changed C++ ranges.

## Surprises and Discoveries

- CUDA 12.9 libNVVM accepts the selected native Half/Half2 modules at O3 but reports only
  `Error: unsupported operation` at O0. Scalar Half at O0 has previously succeeded, so the cluster
  is not simply a blanket ban on LLVM `half`.
- The cluster has remained stable since the initial Half slices: texture load/store, Half vector
  comparison, bit reinterpretation, and the focused value-algebra fixture all share the provider
  failure. `half-vector-calc` has the same failure but lacks a healthy NVRTC reference on this
  machine because CUDA 12.9's `__half4` source type has no `.xyz` member.
- O3 success proves the compiler's selected type and operation contracts are already sufficient;
  this slice must identify the transformation that makes the module consumable rather than widen
  Slang preflight or add another Half API.
- The `-dump-intermediates` artifacts for O0 and O3 were byte-for-byte identical. The mode
  distinction exists only inside `nvvmCompileProgram`; Slang and the provider did not emit
  different graphs.
- Direct CUDA 12.9 libNVVM probes accepted Half2 arithmetic, comparison, fptrunc/fpext, fptosi,
  constant and dynamic extraction, scalar Half loads, and a Half2 helper call at `-opt=0`. Only a
  vector built by inserting a runtime Half lane failed with `Error: unsupported operation`.
- Rebuilding the same two-, three-, and four-lane values as `<N x i16>` from scalar bitcasts and
  bitcasting the completed vector to `<N x half>` passed verification and compilation at O0 and
  O3. This is a representation recipe, not an optimization pipeline.
- The first provider change fixed ordinary Half constructors, comparisons, helpers, and bit casts,
  but the surface workload still failed. Its generated surface-load helper independently assembled
  a `<4 x half>` with native insertion. Routing every provider-owned vector construction through
  one helper fixed that second producer and removed the cluster completely.

## Decision Log

- Decision: prioritize unoptimized Half canonicalization over the remaining one-row discovery
  blockers.
  Rationale: it is the largest coherent healthy cluster, closes O0/O3 mode agreement for ordinary
  16-bit compute operations, and exercises one external representation invariant across arithmetic,
  comparison, resources, and bit transport.
  Date/author: 2026-09-01, Codex.
- Decision: treat provider O3 as a differential oracle, not as permission to run the full O3
  pipeline for an O0 request.
  Rationale: O0 remains a distinct correctness/debugging configuration. Any retained transform
  must be the smallest semantics-preserving canonicalization required by libNVVM and must have a
  nameable unsupported input shape plus focused evidence.
  Date/author: 2026-09-01, Codex.
- Decision: represent every provider-owned runtime Half vector construction as exact i16 lane
  transport followed by one vector bitcast.
  Rationale: the reduced probes identify native Half lane insertion as the only rejected operation,
  and i16 construction preserves every payload bit and lane position while compiling at both modes.
  Applying the rule in one helper covers generic construction, scalar broadcast, and surface-load
  reconstruction without compiler-side shape checks or textual rewriting.
  Date/author: 2026-09-01, Codex.
- Decision: keep the compiler catalog and provider ABI revision 31 unchanged.
  Rationale: all semantic operations and types were already correctly classified and exposed by
  the generic interface. This is an LLVM/libNVVM representation constraint wholly owned by the
  isolated provider.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Direct NVVM O0 now accepts canonical runtime Half2/Half3/Half4 construction across ordinary value
algebra, helper transport, bit reinterpretation, and typed surface loads. The provider has one
source of truth for vector construction: non-Half values use native insertion, while Half values
insert exact i16 lane bits and bitcast the completed vector. No LLVM pass pipeline or O3 remapping
is involved.

Frozen corpus v1 remains exactly 452/427 and advances from 396/400/396 to 400/400/400 O0/O3/both,
with exactly four healthy gains and no old-correct loss. The fifth raw provider failure also becomes
correct at O0 but stays outside the healthy denominator because its CUDA 12.9 NVRTC reference is an
infrastructure failure. Discovery remains exactly 82/72 at 66/66/66 with no gain or loss. The
selected prefix passes 433/433 and the five fixture pairs pass 10/10.

All 30 representative gates produced five measurement rows and assembled cubins, for 150/150
total. The result closes the historical O0 Half provider cluster with a mandatory representation
invariant while preserving O0 as a distinct compilation mode.

## Context and Current Pipeline

Slices 94 through 101 established native LLVM `half` scalars and fixed vectors, typed arithmetic,
comparison, conversions, reinterpretation, helper transport, resource access, and surface-specific
recipes. `NVVMSemantics::resolveValueOperationFamily` supplies exact typed descriptors, and the
LLVM 14 provider emits ordinary native LLVM operations. The compatibility writer serializes LLVM
7-compatible textual NVVM IR, and `nvvmCompileProgram` applies the requested libNVVM option.

At `-opt=3`, libNVVM consumes these modules and produces correct PTX. At `-opt=0`, the same selected
workloads reach provider compilation and fail after verification with a generic unsupported-
operation diagnostic. The first task is therefore to preserve the exact emitted module and compare
its pre/post-O3 instruction forms, then isolate which form libNVVM requires.

The healthy frozen rows are:

- `compute/half-rw-texture-simple.slang#cuda-1`;
- `compute/half-vector-compare.slang#cuda-1`;
- `cuda/nvvm-half-values.slang#cuda-1`;
- `hlsl-intrinsic/bit-cast/bit-cast-16-bit.slang#cuda-1`.

`compute/half-vector-calc.slang#cuda-1` is useful diagnostic evidence but remains outside the
healthy denominator because its native reference does not compile on this toolkit.

## Scope and Non-Goals

In scope are native Float16 scalar/vector instructions already admitted by exact compiler
semantics, provider serialization and module canonicalization immediately before libNVVM, the O0
option contract, focused libNVVM probes, the five exact cluster rows, adjacent scalar-Half and O3
regressions, both fixed corpora, and representative measurement evidence.

Out of scope are BFloat16, FP8, new Half operations, source-level Half ABI changes, blanket O3 for
O0 requests, textual regex rewriting, fixture-name checks, compatibility fallbacks, accepting
malformed LLVM IR, changing the frozen/discovery denominators, and unrelated remaining Half
GenericAsm or raw-buffer operations.

## Architecture and Invariants

- Slang preflight and the typed semantic catalog remain the source of truth for which Half
  operations exist; provider canonicalization cannot admit a new semantic operation.
- O0 and O3 must preserve identical observable kernel results. O0 may run only a bounded mandatory
  legalization whose exact input and output forms are required by libNVVM, not an optimization
  pipeline chosen merely because it happens to remove the failure.
- Any transform acts on the LLVM graph before compatibility serialization. No textual pattern
  reconstruction or post-serialization patch is permitted.
- Scalar Half behavior and every adjacent rejected type/operation remain unchanged.
- Provider ABI revision 31 remains unchanged unless a concrete canonical operation cannot be
  represented inside the existing provider implementation.

## Interfaces and Dependencies

Likely audit points are `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`, the LLVM compatibility writer,
and existing optimization setup around `nvvmCompileProgram`. Focused real-provider coverage lives
in `tools/slang-unit-test/unit-test-nvvm-builder.cpp`. CUDA 12.9/libNVVM supplies the authoritative
consumer behavior; the isolated LLVM 14 provider supplies graph construction and any bounded
canonicalization pass.

## Milestones

1. Preserve O0 and O3 provider input for representative arithmetic/comparison, resource, and
   reinterpretation workloads; enumerate instruction differences without inferring from fixtures.
2. Build minimal provider/libNVVM probes for each candidate unsupported operation and test existing
   LLVM canonicalization components one at a time. Promote only a transform that is both necessary
   and sufficient across the cluster.
3. Implement the transform at its owning graph boundary, add focused positive and negative unit
   coverage, rebuild outside the sandbox, and differentially validate the real workloads.
4. Promote stable O0 lanes where needed, rerun the selected prefix and exact corpora, refresh
   measurements, and complete the producer/input-shape self-review.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
focused real-provider proof of the exact pre/post canonical form; correct differential O0 and O3
results for every healthy promoted workload; unchanged adjacent scalar-Half and unsupported-shape
behavior; zero old-correct regression; the selected NVVM unit prefix; frozen identity 452/427;
discovery identity 82/72; separate census/Pareto artifacts; PTX assembly for representative gates
at SM70, SM80, and SM90; formatting attempt; `git diff --check`; artifact integrity; and an exact
staged-file audit excluding `external/slang-binaries/`.

## Failure and Recovery

Every probe is additive or temporary and must be removed before completion. If no bounded LLVM
graph transform makes O0 acceptable, record the smallest rejected IR and toolkit version, keep O0
classified as a provider limitation, and stop the loop for design discussion rather than silently
mapping O0 to O3. If one candidate fixes only one fixture, reject it unless the exact operation
defines a reusable family with adjacent coverage.

## Artifacts and Hand-Off

Keep raw LLVM/NVVM IR, PTX, and probe logs under ignored `build/nvvm-census` paths. Retain a
completed plan only if the slice yields a committed result under the user's workflow exception.
Distill the accepted external contract into `docs/design/nvvm-backend.md`, exact coverage into the
capability ledger and census artifacts, and the full input-shape/rejected-alternative analysis into
the five-part report.
