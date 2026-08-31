# Propose a deduplicated compute corpus v2

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, the repository has an evidence-backed proposal for a bounded corpus v2 without
changing either current denominator. The proposal starts with every exact frozen-v1 identity and
adds a deduplicated set of healthy discovery workloads that preserve representative combinations,
all newly proven invariants, and all remaining healthy first blockers.

The observable artifact is a proposed-additions TSV plus a five-part report that states the exact
candidate size, healthy denominator, current O0/O3/both score, inclusion rules, exclusions, and
approval needed before freezing it.

## Progress

- [x] (2026-08-31) Reached the previously defined approximately-90% checkpoint: frozen v1 is
  384/388/384 over 427 healthy MVP references, or 89.9% correct in both modes.
- [x] (2026-08-31) Partitioned discovery into 45 original healthy successes, 14 workloads unlocked
  since its baseline, 13 remaining healthy failures, and 10 unhealthy reference rows.
- [x] (2026-08-31) Selected one original-success representative per exact discovery tag
  combination, then retained every newly unlocked workload and every remaining healthy failure.
- [x] (2026-08-31) Validated identity, health, deduplication, classification, and exact candidate
  metrics: 502 rows/477 healthy, with 421 O0, 425 O3, and 421 correct in both modes.
- [x] (2026-08-31) Documented the proposal, explicitly preserved v1/discovery, and completed
  artifact and self-review checks for Slice 157.

## Surprises and Discoveries

- Selecting only the thirteen measurement gates would underrepresent combinations already correct
  at discovery baseline, including atomics with mixed resources, shared memory with barriers,
  parameter layouts, and aggregate pointer transport.
- Selecting one row per exact discovery tag combination retains 23 of the 45 original healthy
  successes. Adding all 14 later unlocks and all 13 remaining healthy failures yields a round,
  bounded 50-row candidate addition while dropping 22 tag-duplicate successes and all 10 rows
  without a healthy native reference.
- The repository formatting script still lacks `gersemi`, `clang-format`, `prettier`, and `shfmt`
  on this machine. This slice changes only Markdown, JSON, and TSV; they were manually reviewed,
  the JSON parses, and `git diff --check` is clean.

## Decision Log

- Decision: propose `frozen v1 + 50 discovery additions`, not the 82-row discovery union.
  Rationale: the proposal preserves the historical 452 identities and adds breadth without
  freezing redundant tag combinations or unstable native references.
  Date/author: 2026-08-31, Codex.
- Decision: retain every newly unlocked workload even when its selection tags overlap another row.
  Rationale: each has permanent direct lanes proving a distinct invariant added after the discovery
  baseline; removing it would discard historical generalization evidence.
  Date/author: 2026-08-31, Codex.
- Decision: retain every remaining healthy failure.
  Rationale: the discovery corpus currently has only 13 such rows, and each protects a live
  canonical producer/type/operation shape needed for future Pareto selection.
  Date/author: 2026-08-31, Codex.
- Decision: do not modify runner manifests, corpus artifacts, or denominators in this slice.
  Rationale: the user explicitly requires a proposal and rationale before changing the long-term
  baseline.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

The proposal keeps every exact frozen-v1 row and adds 50 unique healthy discovery rows with zero
v1 source overlap. The additions comprise 23 representatives covering 23 distinct original
selection-tag sets, all 14 invariants unlocked since Slice 147, and all 13 current healthy
failures. It excludes 22 original-success rows that duplicate one of those tag sets and 10 rows
without a healthy native CUDA reference.

The candidate would contain 502 workloads and 477 healthy MVP references. At Slice 156 capability,
421 are correct at O0, 425 at O3, and 421 in both modes (88.3%/89.1%/88.3%). Native CUDA is correct
for 499 of all 502 rows; the three inherited v1 infrastructure rows remain outside the healthy
denominator. The 56 current both-mode failures remain visible rather than being filtered out.

This result demonstrates why the new score must begin as a separately named baseline: it is lower
than frozen v1's 89.9% because it deliberately adds difficult ABI, aggregate, pointer, resource,
and provider shapes. No runner manifest, corpus artifact, test directive, source, provider, or ABI
changes in this slice. Explicit approval is still required to freeze the candidate.

## Context and Current Pipeline

Corpus v1 freezes 452 workload identities and a 427-row healthy-MVP denominator. It is the
historical regression contract. Discovery separately contains 82 zero-overlap workloads and 72
healthy native references. Slice 156 reaches 384/427 both-mode correctness on v1 and 59/72 on
discovery.

The earlier discovery instructions name approximately 90% both-mode correctness on frozen v1 as a
reasonable point to propose corpus v2. The proposal must not make established progress appear to
regress through an unexplained denominator change. It therefore needs an explicit composition,
current score, and rationale rather than silently repointing the existing scripts.

Discovery selection tags describe semantic combinations such as `helper-generic,large`,
`atomic-wave,mixed-resources`, and `shared-barrier,aggregate-pointer`. They are suitable for
deduplicating original successes because those rows were already correct before discovery-driven
implementation began. Newly unlocked rows instead name exact proven invariants, and remaining
healthy failures name exact future work; both categories remain intact.

## Scope and Non-Goals

In scope are read-only analysis of Slice 147 and Slice 156 artifacts, one proposed-additions
manifest, exact metric calculation, inclusion/exclusion rationale, durable documentation, and an
explicit approval boundary.

Out of scope are changing corpus v1, changing discovery, editing test directives, compiler/provider
implementation, running speculative feature probes, importing external sources, selecting rows
without a healthy native reference, declaring corpus v2, and updating runner defaults.

## Architecture and Invariants

- All 452 exact corpus-v1 identities remain in the candidate.
- Every proposed addition is one current discovery identity with a healthy native CUDA reference
  and zero source overlap with v1.
- Original discovery successes contribute exactly one representative per exact selection-tag set.
- All workloads newly unlocked since Slice 147 and all remaining healthy failures are retained.
- A workload appears once even if it satisfies multiple inclusion rules; the strongest rule names
  its inclusion class.
- Current v2 candidate scores are calculated, not substituted for either existing headline.
- No long-term baseline changes without explicit user approval.

## Interfaces and Dependencies

The proposal consumes `census.slice-156.tsv`, `discovery-census.slice-147.tsv`,
`discovery-census.slice-156.tsv`, and the Slice 156 cluster artifacts. It adds no executable
interface. A future approved freeze would need a new immutable manifest and runner contract in a
separate slice.

## Milestones

1. Select 23 original healthy-success rows, one per exact discovery tag combination, preferring
   larger or more composition-rich representatives.
2. Add the 14 distinct workloads newly correct since Slice 147 and the 13 current healthy failures;
   deduplicate IDs and attach a concise semantic rationale.
3. Validate all 50 additions against Slice 156 discovery: healthy native reference, exact identity,
   zero frozen-v1 source overlap, current classification, and inclusion-class counts.
4. Compute candidate totals and current O0/O3/both correctness from the exact union while keeping
   v1/discovery results separate in the report.
5. Update durable design documentation and write the five-part proposal report. Run artifact and
   diff checks, then commit without changing any active corpus.

## Validation and Acceptance

Acceptance requires:

- proposed additions are exactly 50 unique healthy discovery rows: 23 baseline representatives,
  14 newly unlocked invariants, and 13 remaining failures;
- all additions have zero ID and source overlap with frozen v1;
- the candidate is exactly 502 total rows and 477 healthy MVP references;
- calculated candidate correctness is 421 O0, 425 O3, and 421 in both modes;
- current frozen v1 remains 452/427 and discovery remains 82/72 in every checked-in artifact;
- no source, test, runner, provider, ABI, or active manifest changes occur; and
- JSON/TSV integrity and `git diff --check` pass without staging `external/slang-binaries/`.

## Failure and Recovery

If selection validation finds a duplicate tag set, unhealthy row, identity mismatch, or v1 source
overlap, fix the proposed manifest rather than changing either source corpus. The proposal is
additive and can be discarded without affecting test execution or backend behavior.

## Artifacts and Hand-Off

Commit this completed plan with the proposed-additions TSV, proposal summary JSON, five-part report,
and a short design update. A future slice may freeze the proposal only after explicit approval; it
must preserve this proposal's exact composition or document every reviewed change.
