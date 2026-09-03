# Refresh the proposed compute corpus v2 at the MVP checkpoint

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Refresh the exact corpus-v2 candidate proposed by Slice 157 against Slice 189 capability without
freezing a new denominator. The result must say whether the original 50 additions remain healthy,
deduplicated, and useful; calculate current O0/O3/both correctness over the unchanged 502-row
candidate; and name the three representative workload release gates that productionization should
carry forward.

## Progress

- [x] (2026-09-03) Located Slice 157's existing exact candidate instead of creating a competing
  corpus-v2 composition.
- [x] (2026-09-03) Joined all 50 proposed additions against Slice 189 discovery evidence and
  validated identity, native-reference health, and direct O0/O3 classification.
- [x] (2026-09-03) Recalculated the complete candidate denominator and separate all-row staging.
- [x] (2026-09-03) Recorded the refreshed proposal, representative release gates, and explicit
  approval boundary.
- [x] (2026-09-03) Validated artifacts, updated durable design evidence, and completed self-review.

## Surprises and Discoveries

- The corpus-v2 proposal is not new work: Slice 157 already selected 50 discovery additions using
  23 baseline tag representatives, 14 newly unlocked invariants, and 13 then-remaining healthy
  failures.
- Every one of those 50 additions now has a healthy native CUDA reference and is correct in direct
  O0 and O3. The candidate therefore improves from 421/425/421 to 469/469/469 over its unchanged
  477 healthy-MVP denominator.
- The candidate still exposes all eight healthy frozen-v1 failures. Its 98.3% score is not produced
  by filtering the hard rows.

## Decision Log

- Decision: retain Slice 157's exact addition manifest by reference and SHA-256 rather than copy or
  reselection. Rationale: it is already the reviewed semantic-deduplication boundary, and changing
  it now would mix capability progress with composition changes. Date/author: 2026-09-03, Codex.
- Decision: keep the refreshed candidate `proposed-only`. Rationale: the user approved proceeding
  with the checkpoint slice, but the prior contract requires an explicit freeze decision after the
  exact composition and score are presented. Date/author: 2026-09-03, Codex.
- Decision: formalize the existing measurement defaults as the proposed release gates. Rationale:
  together they cover resource/aggregate/helper transport, parameter-block layout, and shared
  memory/control flow/barriers, and all three are currently correct in both modes. Date/author:
  2026-09-03, Codex.

## Outcomes and Retrospective

The unchanged Slice 157 candidate is ready for explicit review at 469/469/469 over 477 healthy MVP
references. All 50 additions are healthy and correct in both modes, while all eight healthy
frozen-v1 gaps remain visible. The three representative gates are current and correct. No active
corpus, runner, implementation, test, provider, or ABI changed.

## Context and Current Pipeline

Frozen corpus v1 remains the immutable 452-row historical contract with 427 healthy MVP
references. Discovery remains the separate 82-row rolling generalization set with 72 healthy
references. Slice 157 proposed adding 50 selected discovery identities to all 452 v1 identities,
producing a 502-row candidate with a 477-row healthy denominator, but deliberately did not freeze
it.

At Slice 157 capability, that candidate was 421/425/421 correct. Slices 158 through 189 expanded
canonical ABI, resource, aggregate, numeric, and legalization coverage. Slice 189 now reports
419/419/419 over frozen v1 and 72/72/72 over discovery. This slice measures the old proposal at the
new capability boundary without changing either input corpus.

## Scope and Non-Goals

In scope are read-only joins of the Slice 157 proposal with Slice 189 corpus evidence, a refreshed
machine-readable summary, the proposed release-gate contract, durable design/report updates, and
an explicit approval boundary.

Out of scope are editing compiler/provider/test code, changing the 50 selected additions, freezing
corpus v2, modifying runner defaults, changing corpus v1 or discovery, repairing unhealthy native
references, and implementing any remaining feature.

## Architecture and Invariants

- The exact Slice 157 additions manifest is the sole composition source and remains unchanged.
- Every addition must resolve to exactly one Slice 189 discovery row, have a healthy native
  reference, and remain source-disjoint from frozen v1.
- Candidate scores are the exact union of all frozen-v1 identities and those 50 additions.
- Frozen-v1 and discovery scores remain separately reported and unchanged.
- All healthy failures remain visible; no passing-only denominator is created.
- A future freeze requires explicit approval and a separate slice that creates an immutable runner
  manifest without rewriting historical artifacts.

## Interfaces and Dependencies

The refresh consumes `corpus-v2-proposed-additions.slice-157.tsv`, `census.slice-189.tsv`, and
`discovery-census.slice-189.tsv`. The existing `measure-compute-mvp.py` default workload list
supplies the release-gate identities. No executable or provider interface changes.

## Milestones

1. Verify that the earlier proposal is the current canonical candidate and hash its exact manifest.
2. Validate all addition identities, inclusion classes, health, source disjointness, and current
   classifications.
3. Calculate candidate and all-row classifications without changing either active corpus.
4. Verify the three representative release gates against current frozen evidence.
5. Publish the refresh, durable rationale, five-part report, and approval boundary.

## Validation and Acceptance

Acceptance requires:

- the exact additions manifest hash remains
  `8126F402CE9F45D706C24A51E189783528DB5B5797AA45035AA7FB2ACD08CE9E`;
- all 50 additions resolve, remain healthy, and are correct at both O0 and O3;
- the candidate remains 502 rows/498 sources with a 477-row healthy denominator;
- current candidate correctness is exactly 469/469/469, or 98.3%, with eight healthy failures;
- frozen v1 remains 419/419/419 over 427 and discovery remains 72/72/72 over 72;
- all three release gates have healthy native references and correct direct O0/O3 results;
- no source, test, provider, ABI, runner, or active corpus artifact changes; and
- JSON integrity and `git diff --check` pass without staging `external/slang-binaries/`.

## Failure and Recovery

If an addition no longer resolves, loses a healthy reference, or regresses, report that exact row
instead of editing the proposal. This refresh is additive documentation and can be discarded
without affecting either corpus or backend execution.

## Artifacts and Hand-Off

Commit this completed plan with a machine-readable refresh JSON, five-part report, and durable
design/ledger update. A future slice may freeze the exact candidate only after explicit approval.
