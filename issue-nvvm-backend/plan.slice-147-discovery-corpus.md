# Establish a separate direct-NVVM discovery corpus

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the repository has a separate, reproducible discovery corpus of 50--100
repository-local compute workloads that are absent from the frozen 452-row corpus v1. The
discovery runner preserves each selected test's source, inputs, and expected-output contract while
creating disposable native CUDA/NVRTC, direct NVVM O0, and direct NVVM O3 lanes. It records a
healthy native-reference denominator, all requested result classifications, the first canonical
IR shape, producer, diagnostic, and a Pareto comparison with corpus v1.

This is a measurement slice. It does not change the compiler, provider ABI, checked-in shader
directives, or corpus-v1 artifacts, and it does not implement support for failures found by the
census.

## Progress

- [x] (2026-08-30) Read the planning contract and audited the existing corpus-v1 runner,
  classifier, summarizer, current 452-row TSV, and Slice 146 Pareto.
- [x] (2026-08-30) Selected and documented 82 non-v1 workloads that emphasize underrepresented
  semantic combinations without observing their direct result.
- [x] (2026-08-30) Added a separate manifest, runner, and summarizer with mechanical corpus-v1
  exclusion and frozen-metric checks.
- [x] (2026-08-30) Ran native NVRTC O3 and direct NVVM O0/O3, audited classifications and
  canonical producers, and published a discovery-only TSV, JSON Pareto, and report.
- [x] (2026-08-30) Verified frozen-v1 artifacts and metrics are unchanged, validated tooling and
  the 424-test selected regression prefix, and completed self-review. Commit is the remaining
  mechanical hand-off step.

## Surprises and Discoveries

- The repository contains 1,272 shader sources with an active compare-compute directive that are
  not among the 448 sources represented by corpus v1. Many belong to deliberately deferred
  families, while the ordinary compute, language-feature, bugs, bindings, IR, and optimization
  directories alone provide hundreds of local discovery candidates.
- A discovery test does not need a permanent CUDA directive. Its existing compare-compute
  directive already owns deterministic inputs and expected output; a disposable mirror can adapt
  only the execution target, exactly as corpus v1 derives direct lanes from native CUDA lanes.
- Seven selected sources are unavailable to the CUDA target before code generation, one
  multisampled-surface source fails in NVRTC 12.9, and two execute with target-specific output
  mismatches. Requiring native correctness leaves 72 healthy references rather than hiding those
  ten rows or treating empty output after E36107 as a runtime result.
- Direct O0 and O3 have identical discovery identities: 45 healthy both-mode successes, 26 healthy
  preflight failures, and one healthy provider failure. The leading exact healthy family is 13
  aggregate/pointer/layout rows; helper ABI accounts for seven.
- A single hard-linked mirror preserves complete relative test imports and reduces the census from
  three physical test-tree copies to one. Each mode overwrites only its generated sibling sources
  after the previous mode has completed.
- The repository formatting script cannot run to completion on this machine because its WSL
  environment lacks `gersemi`, `clang-format`, `prettier`, and `shfmt`. This slice changes no C++,
  CMake, JavaScript, or shell source handled by those tools; Python syntax, generated JSON, Markdown,
  and staged whitespace were checked independently.

## Decision Log

- Decision: keep corpus v1 mechanically immutable and place all discovery manifests, raw outputs,
  summaries, and reports under separately named paths.
  Rationale: an expanding discovery set must not change the historical 452-workload or 427
  healthy-MVP denominators.
  Date/author: 2026-08-30, Codex.
- Decision: use an explicit rolling discovery manifest rather than selecting tests by pass/fail or
  adding directives to checked-in shaders.
  Rationale: source plus existing test ordinal preserves a reviewable semantic contract, while
  thematic tags make the intended coverage distribution auditable and independent of direct-NVVM
  results.
  Date/author: 2026-08-30, Codex.
- Decision: adapt only target-selection arguments in a disposable copy and retain the original
  compare command, source, TEST_INPUT metadata, and indexed expected-output sidecar.
  Rationale: this tests the same workload through native CUDA and direct NVVM without reconstructing
  source syntax or inventing expected results.
  Date/author: 2026-08-30, Codex.
- Decision: require a successful native NVRTC lane before a row enters the healthy discovery
  denominator, but retain native failures as explicitly classified discovery evidence.
  Rationale: direct differential correctness has meaning only with a stable reference, while native
  incompatibilities still identify whether a selected repository workload is portable to CUDA.
  Date/author: 2026-08-30, Codex.
- Decision: classify E36107 target-requirement rejection before expected/actual buffer handling.
  Rationale: the entry point never compiled or executed, so an empty result buffer is infrastructure
  evidence rather than a direct or native runtime mismatch.
  Date/author: 2026-08-30, Codex.
- Decision: retain complete diagnostic type/operation suffixes in the discovery summary and fail
  summary generation if a preflight/provider shape lacks an audited producer mapping.
  Rationale: normalized families are convenient counts but do not satisfy the required canonical
  producer/type/operation deduplication or reveal whether two helper ABI failures share a type role.
  Date/author: 2026-08-30, Codex.
- Decision: use one hard-linked mirror for all modes.
  Rationale: generated tests are the only files changed in the mirror; original test files and
  imports are read-only, so hard links preserve semantics while avoiding repeated multi-gigabyte
  copies on this Windows checkout.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

The separate corpus selects 82 unique repository workloads with zero source overlap against frozen
corpus v1. Native NVRTC O3 supplies 72 healthy references. Direct O0, O3, and both-mode correctness
are each 45/72. Over all 82 rows each direct mode records 45 correct, 28 preflight, one provider,
seven infrastructure, and one runtime-mismatch result; over healthy references, the remaining 27
are 26 preflight and one provider failure.

Every non-correct row has an exact shape, diagnostic, and named producer/owner in the committed
TSV/JSON. Discovery independently finds 13 healthy aggregate/pointer/layout blockers and seven
helper-ABI blockers, agreeing with corpus v1's leading reusable families without combining their
denominators. Three larger correct workloads compile faster and emit much smaller O3 PTX through
direct NVVM in exploratory samples; CUDA 12.9 assembles each direct O3 module for SM70, SM80, and
SM90.

The slice changes no production compiler/provider code, ABI, or shader directive and therefore
unlocks zero workloads; its 45 both-mode successes establish the discovery baseline. Corpus v1
remains 371/375/371 over 427 healthy MVP rows and is below the proposed corpus-v2 milestone. The
next implementation work should establish a reusable nested aggregate/pointer invariant, followed
by generalized helper ABI transport, and measure gains independently in both corpora.

## Context and Current Pipeline

Corpus v1 derives one native CUDA reference and two direct lanes from every active eligible CUDA
compare-compute directive. It now contains 452 fixed workloads from 448 sources, including a frozen
427-workload healthy-MVP denominator. Slice 146 reports 371/427 O0, 375/427 O3, and 371/427
both-mode correctness, plus a separate 424/424 selected regression prefix.

The repository has many other compute-runtime tests with native Vulkan, DirectX, CPU, or implicit
target directives. Their source files already contain the entry point, resource bindings,
`TEST_INPUT` values, FileCheck buffer, and expected output. The discovery runner will select one
existing active compare directive by its source test ordinal, remove only target-specific execution
arguments, add `-cuda`, then derive native NVRTC O3 and direct NVVM O0/O3 lanes using the existing
mode construction and result classifier. Generated mirrors and raw logs remain under ignored
`build/nvvm-discovery/`.

E52017 identifies compiler-owned direct-NVVM preflight rejection and includes the first canonical
linked-IR shape. E52018/E52019, LLVM verification, and libNVVM diagnostics identify provider
failure. Successful compilation with differing expected and actual buffers is a runtime mismatch.
Native CUDA setup, compilation, discovery, or unsupported-target failures are infrastructure/
toolchain results and never enter the healthy-reference denominator.

## Scope and Non-Goals

In scope are an explicit 50--100-workload manifest; thematic selection evidence; target-only
adaptation in disposable mirrors; three-mode execution; exact result classification; first-shape,
producer, and diagnostic ownership; a discovery Pareto; comparison with corpus v1; separate
artifacts; and limited exploratory timing/PTX observations where the selected contracts permit.

Out of scope are compiler/provider changes, ABI revision, new permanent test directives, fixture
promotion, external or third-party shader imports, source reconstruction, diagnostic weakening,
fallbacks, admitting new IR shapes, modifying any corpus-v1 TSV/JSON row, declaring corpus v2, and
fixing any feature gap exposed by discovery.

## Architecture and Invariants

- `census.slice-146.tsv` is the frozen corpus-v1 identity set for this slice. Discovery validation
  rejects any selected source already represented there and asserts its 452 rows, 448 sources, and
  427 healthy-MVP rows.
- The rolling manifest selects workloads before their direct result is known. Selection uses
  semantic-combination tags and existing repository test contracts, never fixture pass/fail.
- Target adaptation has one source of truth: remove a bounded list of harness target/profile and
  target-emission options, retain all semantic/compiler/input arguments, and append native CUDA.
- A selected source must have an active `COMPARE_COMPUTE` or `COMPARE_COMPUTE_EX` directive at the
  recorded source-test ordinal. The runner stops if this contract changes instead of silently
  choosing another directive.
- Native correctness is the reference gate. O0, O3, and both-mode numerators are reported over the
  healthy native denominator; all raw classifications are also reported over the selected set.
- Pareto keys are canonical operation/type/role plus producer/owner. Fixture names remain examples,
  not semantic classification keys.
- Discovery artifacts and denominators never update, overwrite, or aggregate with corpus-v1
  artifacts and percentages.

## Interfaces and Dependencies

Add a rolling manifest and discovery-only development scripts below `issue-nvvm-backend/`.
Generated sources, logs, raw TSV/JSON, and scratch evidence live below ignored
`build/nvvm-discovery/`. Durable Slice 147 evidence lives in separately named committed TSV, JSON,
and report files. The scripts use the Release `slang-test.exe`, the isolated LLVM 14 provider, and
the existing corpus-v1 helper functions without changing builder ABI revision 30.

## Milestones

1. Select a bounded, balanced set from ordinary repository test families and record source,
   directive ordinal, semantic-combination tags, and rationale. Verify no source overlaps v1 and no
   result knowledge affects selection.
2. Add the separate runner. Validate its target-argument adaptation and exact directive lookup on a
   small passing probe, then run native NVRTC O3 plus direct NVVM O0/O3 over the complete manifest.
3. Add the separate summarizer, inspect every non-correct signature, trace the named canonical
   producer/consumer boundary, and publish per-workload and Pareto evidence.
4. Compare discovery clusters with Slice 146 corpus-v1 clusters; record healthy denominator,
   O0/O3/both correctness, classification totals, no-overlap proof, and exploratory performance
   data without changing implementation.
5. Validate deterministic regeneration, script syntax, frozen-v1 metrics, selected NVVM regression,
   formatting, and diff hygiene. Complete the plan/report and commit Slice 147.

## Validation and Acceptance

Acceptance requires 50--100 unique selected workloads; zero source overlap with frozen v1; one raw
classification for every selected mode; native-health and direct correctness totals that reconcile
exactly; no unclassified direct result; an exact diagnostic and canonical producer/type/operation
shape for every failure cluster; a separate discovery Pareto compared with v1; unchanged v1 files,
rows, and healthy-MVP denominator; no backend/provider/fixture implementation change; script syntax
checks; deterministic summary regeneration; the selected direct-NVVM regression prefix; staged
`git diff --check`; and no staged content from `external/slang-binaries/`.

## Failure and Recovery

If target adaptation changes a semantic compiler argument, compare the generated directive with the
recorded original and narrow the bounded adapter; do not edit the shader. If a source cannot produce
a stable native CUDA reference, retain it as infrastructure/toolchain evidence and exclude it from
the healthy denominator. If a direct diagnostic lacks the first linked-IR shape, use an ignored
single-workload final-IR probe to identify the named producer and record that evidence in the
summary; do not widen the backend. Generated output is disposable and rerunning the script replaces
only the exact `build/nvvm-discovery/mirror` directory.

## Artifacts and Hand-Off

Commit the rolling manifest, reusable discovery runner and summarizer, discovery-only per-workload
TSV, Pareto JSON, report, and this completed plan. Retain raw generated mirrors, logs, and probes
under ignored `build/nvvm-discovery/`. The report must name the leading cross-corpus canonical root
causes and recommend the next reusable implementation slice without implementing it here.
