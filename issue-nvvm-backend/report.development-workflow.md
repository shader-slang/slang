# Establish a restartable NVVM development loop

## Motivation

The maintainer wants a new session to continue from measured corpus failures without rebuilding
context from chat history. The complex material now provides an application-sized compilation
probe, while runnable tests must establish the correctness of each feature it motivates. The
existing slice-201 hardware interruption also needed an unambiguous accepted or blocked outcome.

## Proposed solution

Complete three ordered checkpoints: preserve the complex corpus, reconcile slice 201 acceptance,
and document the repeatable slice loop. Use one stable workflow and one current handoff, backed
by existing ExecPlans, reports, manifests, and reusable test runners. Preparing the loop does not
start another feature slice.

## Change summary

- `WORKFLOW.md` defines startup, balanced selection, executable companion coverage, principled
  implementation, acceptance and regression policy, commit boundaries, repetition, and stopping.
  It supplies actual native Linux runner commands and an explicit environment fallback.
- `STATUS.md` identifies accepted evidence, inherited versus fresh results, missing runtime
  contracts, next candidates, host setup, and the exact next action for a new session.
- Root `AGENTS.md` and the backend design document link the workflow/handoff so future sessions
  discover them without a remembered chat prompt.
- This completed plan/report records the scope and validation of the workflow setup itself.
  Earlier commits separately established the complex corpus (`63c118b8d`) and completed slice 201
  acceptance (`187d87148`). No compiler code changes belong to this documentation checkpoint.

## Concepts and vocabulary

An **accepted slice** has its final-source acceptance evidence and committed records. A
**checkpoint** can retain incomplete work without promoting it. **Inherited evidence** names an
older run and remains distinct from freshly executed results. A **complex-driven slice** addresses
a measured application workload blocker and uses a corresponding executable test for semantics.

## Process report

The complex corpus commit preserved both material entries, the agreed CUDA define, runner, initial
assessment, and plan/report. The slice-201 replay then matched the old 61-identity manifest hash and
completed every mode cell on the replacement L4 host. All 165 old-correct cells were preserved and
prefix count gained both direct modes. Its separate commit retains the interrupted A6000 attempt,
fresh per-identity results, focused gates, and the limits of the bounded replay.

The workflow makes those distinctions mandatory. Exact workload/mode comparisons prevent equal
aggregate totals from hiding regressions. Frozen v1 remains stable; runnable tests related to a
complex blocker can enter discovery where its selection contract allows. Matching feature names
is insufficient: the focused test must reach the same representation boundary and execute with
real inputs and expected output. A new diagnostic in the material is progress evidence, not proof
that the whole shader runs correctly.

Selection reserves effort for complex-driven features while prioritizing correctness and reusable
coverage. Newly introduced correctness losses must be fixed or reverted; pre-existing failures
stay recorded with reproduction and provenance. Full corpus acceptance is the default for feature
slices. A justified bounded replay must explicitly identify inherited rows and never claim a fresh
full run. Before slice 202, the new session must establish the full L4 baseline because this task
only refreshed the agreed wave domain.

Restart instructions select existing tooling rather than introducing another orchestration system.
The handoff records the ignored environment helper but provides a fallback and build references,
so Git-tracked instructions do not silently depend on a previous machine's `build/`. The loop starts
only on a future start/resume request, then continues through routine decisions and local commits
without repeated permission questions. Hardware/access problems and consequential unresolved
semantic or scope decisions produce a concrete checkpoint requiring human intervention.

Validation checks command options against the existing runner CLIs, all relative documentation
links, accepted manifest/snapshot identities and hashes, formatting, and staged whitespace. The
compiler/runtime gates were completed for the separate reconciliation commit and are not repeated
for documentation. Final self-review finds no compiler helper, fallback, or special case to audit;
all additions are workflow documentation and links. No slice 202 implementation has started.
