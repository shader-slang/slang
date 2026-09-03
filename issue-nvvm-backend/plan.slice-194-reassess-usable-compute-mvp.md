# Reassess the bounded usable-compute MVP

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Turn Slice 193's four remaining healthy frozen-v1 failures into an explicit, auditable scope
decision. Preserve every frozen and discovery identity and denominator, distinguish historical
corpus scoring from the bounded backend feature promise, refresh the proposed corpus-v2 score,
and identify the next highest-value productionization work without adding speculative feature
support.

## Progress

- [x] (2026-09-03) Confirmed the Slice 193 repository hand-off and exact corpus metrics.
- [x] (2026-09-03) Audited the source and canonical first failure for all four healthy frozen-v1
  gaps.
- [x] (2026-09-03) Recorded the gap dispositions and refreshed corpus-v2 evidence in a
  machine-readable artifact.
- [x] (2026-09-03) Distilled the bounded MVP conclusion and next productionization priorities into durable design
  documentation and the five-part report.
- [x] (2026-09-03) Mechanically validated artifact identities, counts, hashes, and documentation
  consistency and prepared the Slice 194 commit.

## Surprises and Discoveries

- `compute/dynamic-dispatch-substandard-float` is not evidence of a general dynamic-dispatch or
  aggregate-helper gap. Its two concrete implementations carry `FloatE4M3`, `FloatE5M2`, and
  `BFloat16`; the first selected helper result is the user struct `A` containing FP8 values.
- `cuda/require-prelude` uses `__requirePrelude` to define a user macro that following
  `__intrinsic_asm` expressions evaluate. The marker therefore has live source-language semantics
  and cannot be deleted as metadata.
- Frozen corpus v1 intentionally classified these rows before the bounded MVP exclusions were
  settled. Preserving the 427-row denominator means the historical score remains 423/427 even when
  the four gaps are not promised initial-MVP features.

## Decision Log

- Decision: do not support or strip `RequirePrelude` in the direct backend. Arbitrary target text
  and a dependent source expression cannot be represented by the typed provider contract without
  parsing CUDA C++, and removing it changes program semantics. Date/author: 2026-09-03, Codex.
- Decision: defer the three substandard-numeric rows. FP8 is explicitly outside the initial MVP;
  BFloat16 and aggregates containing these formats are not ordinary scalar/vector/matrix coverage,
  and no representative release gate requires them. Date/author: 2026-09-03, Codex.
- Decision: preserve all frozen-v1 classifications and its 427-row denominator. Report the four
  historical failures honestly rather than changing scope metadata to make the score 100%.
  Date/author: 2026-09-03, Codex.
- Decision: keep corpus v2 proposed-only. Refresh its evidence to 473/477 but do not cross the
  explicit approval boundary by installing a new baseline. Date/author: 2026-09-03, Codex.

## Outcomes and Retrospective

Frozen corpus v1 remains exactly 452 identities/427 healthy references at 423/423/423; discovery
remains exactly 82/72 at 72/72/72. The unchanged proposed-v2 union is now 473/473/473 over 477 but
remains behind its explicit freeze-approval boundary.

All four healthy frozen gaps now have an exact source-level disposition. Three depend on FP8 or
BFloat16 types outside the ordinary numeric MVP, and one requires arbitrary target prelude text
consumed by user GenericAsm. No corpus metadata or accepted IR shape changed. The implementation
loop can therefore pivot to enforcing provider discovery/deployment/caching as the next universal
production contract instead of chasing the historical denominator.

## Context and Current Pipeline

Slice 193 left four native-healthy rows outside direct O0/O3 correctness. Three stop while
`_validateNVVMHelperTarget` validates post-specialization helper signatures: result `A`, result
`BFloat16`, or parameter `FloatE4M3`. The fourth contains canonical `IRRequirePrelude`, produced
from the intrinsic operation declared in `core.meta.slang`; the C-like emitter collects its string
and emits it ahead of target text that references the newly defined macro.

The direct path instead legalizes known standard-module target operations to typed semantic IR,
preflights exact canonical shapes, and invokes an isolated LLVM 14 provider. It deliberately has no
CUDA C++ parser, macro expander, or generic target-text fallback.

## Scope and Non-Goals

This is a bounded census/reassessment slice. It may add evidence artifacts and documentation only.
It does not change compiler/provider code, tests, directives, corpus manifests, row classifications,
runner defaults, ABI revision 35, or accepted IR shapes. It does not freeze corpus v2, add FP8 or
BFloat16 support, interpret `RequirePrelude`, or begin packaging implementation.

## Architecture and Invariants

- Frozen corpus v1 remains exactly 452 identities with exactly 427 healthy references.
- Discovery remains exactly 82 identities with exactly 72 healthy references.
- Scores for frozen v1, discovery, and the proposed corpus-v2 union remain separate.
- Unsupported-feature disposition never changes a historical corpus row or hides its diagnostic.
- Typed direct-NVVM support begins from canonical semantic IR; arbitrary CUDA prelude/source text is
  not a representation accepted by the backend.
- A deferred feature is reconsidered only when a selected representative workload establishes its
  importance and a principled typed representation is identified.

## Interfaces and Dependencies

No code interface changes. The evidence artifact references Slice 193's immutable TSV snapshots and
Slice 157's proposed-additions manifest by path and SHA-256. JSON is schema-versioned and uses exact
workload identities so later automation can compare the decision without parsing prose.

## Milestones

1. Audit all healthy native rows that fail either direct mode in `census.slice-193.tsv` and trace
   their sources.
2. Write `mvp-gap-reassessment.slice-194.json` with separate corpus metrics, exact gap shapes,
   producers, diagnostics, dispositions, and the refreshed proposal score.
3. Add a five-part Slice 194 report and a durable design-document section explaining why the
   implementation loop now pivots from denominator chasing to productionization.
4. Validate JSON parsing, referenced hashes, exact identity/count joins, Markdown hygiene, and a
   clean diff; then commit the completed plan with the slice.

## Validation and Acceptance

Acceptance requires a read-only validation script to prove:

- Slice 193 frozen evidence has 452 unique identities and the expected 423/423/423 over 427.
- Slice 193 discovery evidence has 82 unique identities and 72/72/72 over 72.
- the four recorded gap IDs are exactly the native-correct MVP-tier rows failing direct O0/O3;
- the Slice 157 proposal still has the recorded SHA-256 and 50 additions, with no frozen overlap;
- the proposed union is 502 identities, 477 healthy references, and 473/473/473;
- the JSON parses and `git diff --check` passes.

No build or runtime test is required because production code, build inputs, test inputs, runner
logic, and manifests do not change. The established Slice 193 build/runtime/census evidence is the
input to this slice rather than being regenerated under an identical tree.

## Failure and Recovery

If a count, identity, diagnostic, or hash differs, stop and correct the artifact from the committed
TSV/manifests; do not edit the historical evidence. All Slice 194 changes are additive text files
and can be removed without affecting compiler behavior.

## Artifacts and Hand-Off

Commit this completed plan, the schema-versioned reassessment JSON, the five-part report, and the
durable design update. The next slice should use the recorded productionization priority rather
than interpreting a deferred corpus row as an automatic implementation requirement.
