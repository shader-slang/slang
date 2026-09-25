# Preserve the CUDA masked FP32 min/max algorithm

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. The agent
thread limit prevents a fresh worker; as in 217, the parent owns local work under WORKFLOW's fallback.

## Purpose and Observable Result

FP32 masked scalar/vector/matrix minimum and maximum match the concrete CUDA helper's selected raw
input word for finite, infinity, zero and NaN values. Accepted 217's 288-execution oracle matrix must
become exact in all modes. FP64 min/max remains unsupported.

## Progress

- [x] Read accepted 217 research, source algorithm and current typed emission loop.
- [x] Select one bounded recipe correction on base `5316e1b2516b9f9761a6386ed59296e5f758fa53`.
- [x] Establish final corpus fixture failure on unchanged accepted 216 artifacts.
- [x] Implement typed caller-seeded comparison/select and source butterfly/scan order.
- [x] Format/build, focused/oracle/smoke/unit/toolkit/corpus/complex gates and exact comparison.
- [x] Complete representation audit, report/evidence/STATUS, review and local commit.

## Surprises and Discoveries

Research 217 proves all-NaN inputs become injected infinities, predating 216. Mixed-NaN and zero
results additionally depend on source order. The existing two-phi loop can represent both algorithms:
its unsigned state is either the remaining scan mask or the descending butterfly offset.

## Decision Log

Correctness takes priority over FP64 admission. Reuse the typed scalar recipe for aggregate leaves,
existing indexed shuffle for XOR partners, and existing operation closure/preflight. No provider ABI,
source helper or front-end changes. Preserve source comparison/select behavior, not global numeric
min/max behavior. Avoid a final all-NaN patch or a seed-only fix that leaves the algorithm wrong.

## Outcomes and Retrospective

Completed: the final fixture and all 288 research cases pass in all three modes. All 618 old fresh
cells retain all five stable fields, three new cells pass, and 1,035 frozen cells inherit full 214.
Cumulative 1,656 cells contain 1,603 correct and 53 unchanged failures. Required focused, smoke,
unit, toolkit and material compile/assembly gates pass. Full 214 remains latest, cadence becomes two.
The source algorithm corrects the recipe without provider/ABI changes or a special all-NaN fallback.

## Context and Current Pipeline

Canonical GenericAsm helpers from `hlsl.meta.slang` resolve through `_initializeNVVMMaskedWaveScalarOperation`.
`_emitNVVMMaskedWaveScalarValue` currently loops over named lanes with an injected numeric identity.
For FP32 min/max reductions only, select caller value as initial accumulator. Low-bit contiguous
power-of-two masks use XOR partners at population/2 down to 1 and shuffle the prior accumulator;
other masks scan original inputs. Combine with ordered less/greater comparison and typed selection.
Extract the existing mask classification from FP64 sum seed logic for reuse, preserving its behavior.

## Scope and Non-Goals

Only FP32 reduction recipe, relevant structural coverage, one discovery fixture and documentation.
No FP64 admission, prefix/arithmetic changes, provider numeric operation changes, batching or material
runtime claims. Existing singleton behavior remains correct through the source algorithm; remove
redundant FP32-only final selection if the corrected algorithm makes it unnecessary.

## Architecture and Invariants

Keep one shared mask classifier and one typed scalar loop for homogeneous leaves. For the butterfly,
every participating lane reads its partner's prior-stage accumulator; the scan always reads original
inputs. Integer state and floating selections must use validated descriptors included in preflight.
Canonical source shape is valid; recipe owns the mistranslation. No fallback, malformed-shape guard,
source-text parser or alternate semantic representation. Mask zero remains outside participation.

## Interfaces and Dependencies

Matching RelWithDebInfo binaries, native Ubuntu, L4 SM89 target SM80, driver 580.126.09, CUDA 12.9.2,
NVRTC 12.9.86, LLVM 14, provider ABI 36. Source `build/nvvm-loop/slice-203-env.sh`, follow local build
skill. Before compiler is `fa55d1fdc41988e27e672d4a2ad92b93060293d101f4a7a076be3e68dd08298f`;
provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Four CPU build/corpus workers, two unit servers, sequential suites and bounded commands.

## Milestones and Validation

1. Add one dynamically loaded raw-bit source fixture: scalar/vector/matrix qNaN/sNaN/mixed NaN and
   signed-zero families across full, low/high16, low15/high17, even/odd and singleton partitions.
   Its oracle uses closed-form source selection for all-unordered/tied inputs, not a wave intrinsic.
   Preserve final before fixture hash; expect NVRTC pass and both NVVM modes fail.
2. Implement responsible typed recipe changes and structural invariants. Format only changed paths,
   remove only unrelated formatter hunks and build slangc/slang-test/render-test/test-server.
3. Smoke 4, focused old singleton/FP64 edges/new fixture and structural/strict-negative tests; replay
   all 288 research 217 cases into a new raw directory without overwriting accepted data. Relevant
   units, toolkit 18, all six material compile/assembly cells.
4. Targeted frozen selection `selection.slice-208-frozen.tsv`: 107 identities/321 cells. Full discovery
   grows from 99 to 100 identities/300 cells, within its declared capacity. Expected 618 old fresh
   cells exact against 216 plus three new correct; 1,035 frozen cells inherit 214 explicitly.
   Cumulative expected 1,656 cells, 1,603 correct and 53 unchanged failures, four resolved histories.

## Preservation and Checkpoint Triggers

Compare classification, return code, complete execution counts, diagnostic and canonical shape.
Preserve all original failures and first-known evidence; research 217 resolution is separate from
53 registered failures. No old source/oracle/selection change. Full 214 remains latest checkpoint;
targeted acceptance would advance cadence one to two. Provider/library/ABI/shared lowering changes,
uncertain impact or unexplained regression require a full checkpoint. Correctness loss blocks acceptance.
No material runtime correctness or performance claim; bindings/oracle remain absent.

## Failure and Recovery

Record apparatus errors separately and preserve raw logs. Stop on GPU loss without driver changes.
Trace unexpected shapes to producers; stop new independent blocker investigation after a minimal
handoff. No commits until final review, no push. Escalated commands are required for sandbox bwrap.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-218-{before,after}`. Durable plan, five-part report, runtime manifest,
census tables, source fixture/discovery registration, design and STATUS. Parent handles local review
and acceptance because fresh-context delegation is unavailable.

2026-09-24: Final formatted fixture passes NVRTC and fails NVVM O0/O3 on unchanged 216 artifacts.
The source loop and mask extraction build successfully. Final validation script freezes source and
artifact hashes before smoke/focused/research gates; broad suites follow only after those pass.

2026-09-25 acceptance review: audited each recipe branch and helper extraction against the CUDA
producer/consumer trace; verified unchanged research source/inputs/expected arrays and all 288 exact
outputs. The only reporting apparatus correction was the old census filename. Preserve raw failure
logs and exact source/binary identity. No independent worker review was available due to thread limit.

Final local acceptance audit, 2026-09-25: verified 114 evidence references, 18 tested source hashes,
12 artifacts and 548 runtime input hashes. Independently compared all 321 selected frozen and
297 old discovery cells across five stable fields; three additions pass. All 53 first-known failure
records and four resolved histories remain intact. Accepted targeted, full 214 retained, cadence two.
