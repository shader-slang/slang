# Establish an explicit NVVM-ready IR contract

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, direct NVVM has a named legalization stage between linked optimized Slang IR and
preflight. That stage consumes canonical target markers, compile-time layout queries, and no-op
instructions that should never reach LLVM emission. Preflight verifies an explicit NVVM-ready IR
contract instead of teaching the emitter to ignore each residual operation.

The motivating frozen candidates are `cuda/cuda-texture.slang` (`RequireComputeDerivative`),
`cuda/require-prelude.slang` (`RequirePrelude`), and `hlsl-intrinsic/matrix-double.slang`
(`unmodified`). They are gains only if producer tracing proves each operation has already served its
semantic purpose and its removal exposes no unrelated unsupported shape.

## Progress

- [x] (2026-09-01) Converted the architecture review's NVVM-ready-IR phase into this ExecPlan.
- [x] (2026-09-01) Traced every selected residual instruction to its producer and semantic consumer.
- [x] (2026-09-01) Added the named legalization entry point and focused postcondition verifier.
- [x] (2026-09-01) Moved layout/no-op/marker normalization out of emission and removed the old handoff.
- [x] (2026-09-01) Validated both corpora and documented the exposed cascades.

## Surprises and Discoveries

`RequirePrelude` still carries the macro text used by the following CUDA `GenericAsm` in
`cuda/require-prelude.slang`. It therefore remains unsupported and is not erased. Removing
`RequireComputeDerivative` from `cuda-texture.slang` exposes its honest next blocker: the ordinary
texture-sample GenericAsm. Removing `unmodified` unlocks `matrix-double` completely.

## Decision Log

- Decision: introduce one target-specific legalization entry point rather than more conditionals in
  `linkAndOptimizeIR` or the final emitter.
  Rationale: the direct path already diverges at `emitNVVMForEntryPoints`; one explicit boundary
  makes its input contract inspectable and restartable.
  Date/author: 2026-09-01, Codex.
- Decision: a legalizer must reduce representations and delete downstream cases.
  Rationale: moving the same switch to a pass would not lower future feature cost.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

`legalizeIRForNVVM` now owns CUDA layout-query folding, exact removal of the read-none void
`unmodified` marker, exact discharge of CUDA's compile-time `RequireComputeDerivative`, DCE, and a
postcondition that rejects residual instances before provider discovery. The old public fold
handoff and 261 lines of emitter-owned legalization were removed.

Frozen corpus v1 remains 452/427 and advances to 418/418/418 O0/O3/both solely through
`hlsl-intrinsic/matrix-double.slang`, with no old-correct regression. All-row direct correctness is
432 in each mode. Discovery remains 82/72 at 72/72/72. The selected prefix passes 437/437 and the
expanded permanent NVVM category passes 92/92. Provider ABI 34 is unchanged.

## Context and Current Pipeline

`CodeGenContext::emitNVVMForEntryPoints` in `source/slang/slang-emit.cpp` runs
`linkAndOptimizeIR`, a handoff `lowerBitCast`, `foldNVVMCompileTimeLayoutQueries`,
`validateNVVMSupportedIR`, provider discovery, and `emitNVVMIRFromLinkedIR`. Target-specific choices
are therefore divided between the common pipeline, handoff helpers, preflight, and emission.

The frozen census shows residual target markers and `unmodified` reaching
`_validateNVVMFunction`, where they are rejected by the default instruction switch. These are not
new LLVM operations. They are evidence that a producer/consumer phase contract is incomplete.

## Scope and Non-Goals

In scope are a new `source/slang/slang-ir-nvvm-legalize.h/.cpp` boundary (final naming may follow
nearby repository conventions), its invocation in `emitNVVMForEntryPoints`, migration of compile-
time layout folding, exact marker/no-op producer audits, focused positive and negative tests, and
deletion of corresponding emitter/preflight special cases.

Out of scope are arbitrary `GenericAsm` semantics, helper ABI rewriting, new numeric types,
provider ABI changes, compatibility fallbacks, treating unknown target requirements as no-ops, and
accepting a marker based on fixture identity.

## Architecture and Invariants

- Common linking and optimization still own target-independent specialization and canonicalization.
- `legalizeIRForNVVM` owns representation changes required only by direct NVVM.
- A requirement marker is removed only after its named semantic checker has consumed or discharged
  it for the selected CUDA target.
- Compile-time layout queries become constants before preflight; emission never interprets them.
- `unmodified` becomes its canonical operand only if repository semantics prove it is an identity
  operation at this stage.
- The postcondition verifier deterministically reports the first residual forbidden operation and
  its producer before provider discovery.
- NVRTC remains unchanged.

## Interfaces and Dependencies

Primary files are `source/slang/slang-emit.cpp`, the new NVVM legalization files,
`source/slang/slang-emit-nvvm.h/.cpp`, relevant marker producers and existing requirement-checking
passes, and focused tests in `tools/slang-unit-test/unit-test-nvvm-emitter.cpp` plus the three named
shader fixtures.

This slice depends on Slice 180's current generic facade but not on any new provider callback. If
Slice 180 retains a convenience method, update this plan's context without broadening legalization.
All builds and tests use Windows-native tools outside the sandbox.

## Milestones

1. Dump final linked IR for each candidate and trace `RequirePrelude`,
   `RequireComputeDerivative`, `unmodified`, and layout queries from construction through all
   existing consumers. Record whether each final shape is canonical or an accidental residue.
2. Add `legalizeIRForNVVM` with no behavior change and invoke it after common linking/late handoff
   normalization but before preflight. Add a focused order/postcondition test.
3. Move `foldNVVMCompileTimeLayoutQueries` into the legalizer and make emission require folded
   values. Remove the old public handoff function.
4. Consume only the proven residual marker/no-op set at its canonical owner. Add adjacent negative
   tests showing an unconsumed or unknown requirement remains a deterministic preflight failure.
5. Remove downstream cases made unreachable, replay both corpora, document every newly exposed
   blocker, update durable design/report artifacts, and commit with subject `slice 181`.

## Validation and Acceptance

Build the Release provider and host outside the sandbox, set `SLANG_NVVM_BUILDER_PATH`, and run
focused unit tests for legalization ordering, exact residual rejection, layout folding, and the
three candidate fixtures at O0/O3. Then run:

```powershell
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build --config Release
cmake.exe --build build --config Release --target slang-unit-test slangc slang-test
$env:SLANG_NVVM_BUILDER_PATH = 'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Release/bin/slang-test.exe -category nvvm
python.exe issue-nvvm-backend/run-compute-census.py --output build/nvvm-census/slice181 --workload-ids-from issue-nvvm-backend/census.slice-179.tsv --jobs 8
python.exe issue-nvvm-backend/summarize-compute-census.py --input build/nvvm-census/slice181/results.tsv --table issue-nvvm-backend/census.slice-181.tsv --clusters issue-nvvm-backend/census.slice-181-clusters.json
python.exe issue-nvvm-backend/run-compute-discovery.py --frozen-v1 issue-nvvm-backend/census.slice-146.tsv --output build/nvvm-discovery/slice181 --jobs 8
python.exe issue-nvvm-backend/summarize-compute-discovery.py --input build/nvvm-discovery/slice181/results.tsv --selection build/nvvm-discovery/slice181/selected-workloads.tsv --frozen-v1-clusters issue-nvvm-backend/census.slice-181-clusters.json --table issue-nvvm-backend/discovery-census.slice-181.tsv --clusters issue-nvvm-backend/discovery-census.slice-181-clusters.json
```

Acceptance requires unchanged denominators, no old-correct regression, a recorded first blocker
for every candidate that does not become correct, no provider ABI revision, no marker ignored by
the emitter, changed-line clang-format 17, and `git diff --check`. The implementation must include
a revert drill proving that removing one legalizer rewrite restores the focused preflight failure.

## Failure and Recovery

If producer tracing shows a marker still carries live target semantics, leave it unsupported and
fix or extend its named semantic consumer in a separately justified milestone; do not erase it in
the handoff. If moving layout folding changes optimized IR, retain both implementations only long
enough to compare output, then select one owner and delete the other before completion.

The new legalizer is direct-NVVM-only and can be disabled by removing one call without affecting
NVRTC. Corpus and IR dumps remain below ignored build directories and are safe to regenerate.

## Artifacts and Hand-Off

Keep pre/post-legalization IR and candidate diagnostics below `build/nvvm-census/slice181-*`.
Commit the completed plan, implementation, focused tests, frozen/discovery artifacts, durable
design/capability updates, and five-part report. The hand-off to Slice 182 must enumerate the exact
NVVM-ready IR contract and list every remaining accepted `IRGenericAsm` family.
