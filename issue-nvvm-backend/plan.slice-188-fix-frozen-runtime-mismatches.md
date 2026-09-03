# Slice 188: Resolve frozen-corpus direct NVVM runtime mismatches

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds.

## Purpose and Observable Result

Diagnose the three frozen-corpus workloads that compile through direct NVVM but disagree at
runtime, fix each principled producer/representation defect that belongs to the initial compute
MVP, and promote them to correct O0/O3 results without regressing either corpus.

## Progress

- [x] 2026-09-03: Reproduced each mismatch independently and captured native/direct outputs and PTX.
- [x] 2026-09-03: Traced each mismatch to its first divergent canonical representation or setup rule.
- [x] 2026-09-03: Implemented only producer-side/canonical-contract and evidence-classification fixes.
- [x] 2026-09-03: Added focused regression coverage and ran O0/O3 differential validation.
- [x] 2026-09-03: Updated corpus metrics, design evidence, and the five-part report.

## Surprises and Discoveries

- Only one of the three rows reaches runtime. The SM90 atomic row fails reference/profile setup,
  and AnyValue aborts during common bit-cast lowering before direct preflight.
- The bounds option exists only as a generated CUDA-prelude macro. Direct O3 PTX is byte-identical
  with and without the define, while native PTX changes.
- The complete O0 run briefly lost the CUDA renderer. Five focused retries restored four correct
  rows and one known preflight row; classifier ordering now prevents ignored tests from counting
  as correct.

## Decision Log

- 2026-09-03, Codex: Treat silent runtime divergence as higher priority than admitting new
  operations. Do not combine unrelated fixes unless each has independent reproduction and
  ownership evidence.
- 2026-09-03, Codex: Do not implement bounds behavior in emission because final linked IR contains
  no bound-fix operation. Keep the one genuine mismatch open for a producer-side representation.
- 2026-09-03, Codex: Preserve exact two/four-word descriptor transport through common bit-cast
  lowering so direct preflight, the owner of physical representation, produces the diagnostic.

## Outcomes and Retrospective

The audit reduced the truthful runtime-mismatch count from three to one per mode without changing
the frozen denominator or supported correctness. AnyValue now receives deterministic E52017
preflight instead of an internal compiler abort, and capability-profile setup is infrastructure.
Frozen correctness remains 418/418/418 over 427 and discovery remains 72/72/72 over 72 with zero
old-correct regression. The remaining bounds gap requires a future producer-side option/IR design.

## Context and Current Pipeline

Slice 184 records three direct-NVVM runtime mismatch clusters: bounds checking at zero index,
SM90 mixed-width byte-address atomic behavior, and one remaining aggregate/layout result. The
census harness already supplies stable native CUDA/NVRTC references and direct O0/O3 execution.
This slice begins from those exact rows and traces upstream from generated PTX through NVVM-ready
IR rather than patching emitted text.

## Scope and Non-Goals

Do not widen unsupported IR, add fixture checks, weaken diagnostics, alter the frozen v1
denominator, or hide infrastructure failures. A workload whose native reference is not healthy is
diagnosed and reported but cannot be counted as newly correct until the reference is stable.

## Architecture and Invariants

Native and direct executions must consume equivalent deterministic inputs and layouts. Every fix
must identify the canonical producer and preserve one representation across O0/O3. Emitter-side
special cases are rejected when an upstream legalization or ABI producer owns the defect.

## Interfaces and Dependencies

Use existing census scripts, CUDA/NVRTC reference path, direct provider ABI revision 34, and
repository-local tests. Add no provider callback unless an exact canonical operation is proven
inexpressible through current generic operations.

## Milestones

1. Reproduce and minimize the bounds-zero-index mismatch.
2. Reproduce and audit the SM90 mixed-width atomic/reference setup.
3. Identify the third current mismatch from the all-row census and trace its layout contract.
4. Implement independently justified fixes and validate all modes/corpora.

## Validation and Acceptance

Run all builds/tests outside the sandbox. Each changed workload must match its healthy native
reference at O0 and O3, focused negative tests must preserve deterministic diagnostics, old-correct
regressions must remain zero, and selected/category gates must pass. Report infrastructure rows
separately from the healthy denominator.

## Failure and Recovery

Keep each root-cause fix independently reviewable. If a row lacks a healthy reference, retain its
classification and stop short of claiming correctness. If traces show the issue is outside the MVP
or requires a provider ABI change, document that evidence rather than adding an emitter workaround.

## Artifacts and Hand-Off

Retain direct/native outputs needed for diagnosis locally; commit only stable tests, Slice 188
census/cluster artifacts, design/ledger updates, and the five-part report. Keep this active plan
uncommitted.
