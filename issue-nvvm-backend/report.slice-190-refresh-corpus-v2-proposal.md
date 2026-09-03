# Slice 190: Refresh the proposed compute corpus v2

## 1. Motivation

Slice 189 brings frozen corpus v1 to 419/427 correct in both modes and discovery to 72/72. That is
past the milestone at which the project agreed to propose a new long-term baseline. A new selection
would be counterproductive, however: Slice 157 already proposed an exact, deduplicated 50-workload
addition set and left it behind an explicit approval boundary.

The useful question is therefore whether that unchanged proposal still represents the right
coverage and what its score is after 32 more implementation and cleanup slices.

## 2. Proposed solution

Keep the exact Slice 157 composition and join it against current Slice 189 evidence. Publish a
machine-readable refresh containing its immutable manifest hash, current frozen/discovery scores,
candidate score, all-row failure staging, and three representative release gates. Leave corpus v1,
discovery, runner defaults, and the proposed-only approval boundary unchanged.

The candidate remains:

```text
all 452 frozen-v1 identities
+ 23 original discovery tag representatives
+ 14 invariants newly unlocked by Slice 157
+ 13 healthy discovery failures that remained at Slice 157
= 502 workload identities, 498 sources, 477 healthy MVP references
```

## 3. Change summary

- `corpus-v2-proposal-refresh.slice-190.json` records current evidence for the unchanged proposal.
- The refresh names resource/aggregate/helper transport, parameter-block layout, and
  shared-control/barrier workloads as release gates.
- The design and capability ledger record that the candidate is now 469/469/469 over 477.
- This completed plan and report preserve the selection and validation rationale.
- No compiler, provider, test, runner, active corpus, or ABI file changes.

## 4. Concepts and vocabulary

- **Composition source**: Slice 157's exact proposed-additions TSV, identified by path and SHA-256.
- **Candidate score**: current results over the proposed union; it does not replace either existing
  corpus headline.
- **Release gate**: a stable, deterministic workload that combines several semantic families and
  must remain correct in both direct modes.
- **Proposed-only**: evidence ready for review that has not changed an active manifest or baseline.

## 5. Process report

The input-shape audit here concerns evidence identities rather than compiler IR. Slice 157 selected
one representative for each of 23 original discovery tag combinations, then retained all 14
workloads it had newly unlocked and all 13 healthy failures still open at that time. Those rules
produced 50 identities with no source overlap with corpus v1. Reselecting after all discovery rows
became correct would erase the historical reason the difficult rows were included, so the old
manifest remains the semantic source of truth.

Its SHA-256 is
`8126F402CE9F45D706C24A51E189783528DB5B5797AA45035AA7FB2ACD08CE9E`. Joining all 50 IDs against
`discovery-census.slice-189.tsv` finds 50 unique rows, no missing identity, 50 healthy native
references, 50 O0 successes, and 50 O3 successes. Rejoining their sources against all frozen-v1
sources finds zero overlap. The complete union remains 502 identities from 498 sources.

The frozen part contributes 427 healthy rows and 419 successes; the additions contribute 50
healthy rows and 50 successes. The candidate is therefore 469/477 at O0, O3, and both modes, or
98.3%, with eight healthy failures. This is an honest denominator: those eight are exactly the
remaining healthy frozen-v1 preflight gaps, not rows filtered out to improve the score.

Across all candidate rows, native CUDA has 499 correct and three infrastructure results. Each
direct mode has 483 correct, 18 preflight failures, one infrastructure failure, and no runtime
mismatch. Frozen v1 remains separately reported as 419/419/419 over 427; discovery remains
72/72/72 over 72. No denominator or historical artifact changed.

The three release gates already form the default list in `measure-compute-mvp.py`:

- `dynamic-dispatch-bindless-texture` combines resources, aggregates, and helper dispatch;
- `parameter-block` covers conventional parameter-group layout; and
- `groupshared-multi-barrier-functional` combines shared storage, control flow, and barriers.

All three have healthy native references and correct direct O0/O3 results in Slice 189. Naming them
in the proposal turns an existing measurement default into an explicit release expectation without
adding test directives or changing the runner.

The self-review inventory contains one JSON evidence snapshot and documentation only. It introduces
no helper, fallback, shape matcher, syntax reconstruction, compatibility path, compiler special
case, or provider operation. The exact composition remains proposed-only because proceeding with
this checkpoint is not the same as explicitly authorizing a baseline freeze. A freeze should be a
separate slice that installs an immutable manifest and runner contract while retaining v1 forever.
