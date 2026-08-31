# Slice 157: Proposed compute corpus v2

## 1. Motivation

Frozen corpus v1 now reports 384/427 workloads correct at both direct-NVVM optimization modes, or
89.9%. The discovery instructions named approximately 90% as the point to consider a new frozen
denominator. Simply appending all 82 discovery rows would mix useful semantic combinations,
duplicate successes, and ten workloads without healthy native CUDA references. Selecting only
currently passing rows would make the score look better while hiding the exact failures discovery
was created to expose.

The decision therefore needs an explicit, reviewable composition before any runner or baseline
changes.

## 2. Proposed solution

Define a candidate v2 as all 452 exact frozen-v1 identities plus 50 discovery additions selected by
three rules:

1. Keep one original healthy success for each exact Slice 147 selection-tag combination.
2. Keep every workload newly unlocked between Slice 147 and Slice 156.
3. Keep every remaining Slice 156 failure with a healthy native reference.

Deduplicate workload IDs, require zero source overlap with v1, and exclude discovery rows without a
healthy native reference. Publish the exact additions and calculated candidate score, but leave
both active corpora and every runner default unchanged until explicit approval.

## 3. Change summary

- `corpus-v2-proposed-additions.slice-157.tsv` lists the 50 exact additions, inclusion class, and
  semantic rationale.
- `corpus-v2-proposal.slice-157.json` records composition, exclusions, current score, identity
  checks, and the approval boundary.
- The design and capability ledger document how to interpret the harder candidate denominator.
- This completed plan and report preserve the selection process and validation evidence.
- No compiler, provider, test, corpus runner, active manifest, or ABI file changes.

## 4. Concepts and vocabulary

- **Selection-tag set**: the exact comma-separated discovery categories attached to a workload,
  such as `atomic-wave,mixed-resources` or `parameter-layout,helper-generic`.
- **Original success**: a healthy workload already correct in both modes at the Slice 147 discovery
  baseline, before discovery-driven feature implementation.
- **Newly unlocked invariant**: a workload that failed at Slice 147 and is correct in both modes at
  Slice 156, with permanent direct lanes protecting the supporting representation change.
- **Proposed-only composition**: an exact candidate that has not changed an active manifest,
  denominator, runner default, or historical score.

## 5. Process report

The first partition compares the exact 82 discovery identities at Slice 147 and Slice 156. Native
reference health remains 72. Forty-five healthy rows were correct in both modes at the discovery
baseline, 14 additional healthy rows became correct through Slices 148–156, and 13 healthy rows
still fail. The remaining ten rows have infrastructure or native runtime-reference problems and
cannot strengthen a differential correctness denominator.

The 45 original successes are the only category deduplicated by tags. Their exact tag strings form
23 distinct sets. One composition-rich representative is retained from each set: for example,
`byte-address-buffer-interlocked-add-f32` represents `atomic-wave,mixed-resources`,
`addr-scope-fix` represents `shared-barrier,aggregate-pointer`, and `loop-inversion` represents
`control-flow,large`. This keeps coverage of every discovery selection dimension while removing
22 rows whose tag set is already represented. Tags are used only for proposal selection; no
compiler behavior or fixture-name check depends on them.

All 14 newly unlocked rows remain even where their tags overlap. They are historical evidence for
distinct invariants: recursive parameter-group fields and loaded values, composable local
addresses, resource helper results, specialized-function identity, CUDA descriptor handles,
finite group-shared values, and UInt64 word reconstruction. Each has permanent O0/O3 directives,
so retaining it binds the candidate to behavior deliberately promoted during the program.

All 13 remaining healthy failures also remain. Their current first blockers cover entry-point and
helper aggregate ABI, double-indirect helper pointers, device-to-generic UserPointer conversion,
aggregate field/sequential pointers and storage layout, fixed-array construction, and resource
arrays. Removing duplicate cluster labels would lose distinct producer/type combinations—for
example, the two UserPointer rows originate in device-array and group-shared storage paths—so this
small failure set is retained intact for future Pareto work.

Validation joins every proposed ID against `discovery-census.slice-156.tsv`. The additions are 50
unique rows with 50 healthy native references, zero missing IDs, zero frozen-v1 ID overlap, and
zero frozen-v1 source overlap. The 23 baseline representatives have 23 unique exact tag sets and
were all correct at Slice 147. The 14 unlocks were all non-correct at Slice 147 and correct in both
modes at Slice 156. The 13 failure rows are all still non-correct in at least one mode.

The exact union contains 502 rows from 498 sources. Its healthy MVP denominator is 477. At Slice
156 capability, 421 are correct at O0, 425 at O3, and 421 in both modes, yielding
88.3%/89.1%/88.3%. There are 56 healthy both-mode failures. Across all candidate rows, native CUDA
is correct for 499; the three inherited v1 infrastructure rows remain outside the denominator.

The candidate's 88.3% both-mode score is lower than v1's 89.9% because it exposes harder combined
workloads and all current healthy discovery failures. It must start as a newly named baseline if
approved; it must not rewrite the historical v1 series. Discovery also remains useful as a rolling
generalization set and is not deleted by this proposal.

The self-review inventory contains only data selection and documentation. No new helper, fallback,
special case, AST/IR representation, test directive, source reconstruction, or downstream patch is
introduced. The proposal JSON says `proposed-only` and `approval_required_before_freeze: true`.
A future freeze must be a separate reviewed slice that creates an immutable manifest and updates
runner contracts only after explicit approval.
