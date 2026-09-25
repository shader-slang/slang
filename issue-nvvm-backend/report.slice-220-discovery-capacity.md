# Slice 220: expand explicit discovery capacity

## Motivation

The discovery manifest has reached 100 entries. A new FP64 min/max fixture would be rejected before
source validation, even though research 219 established the expected CUDA behavior. Capacity must
be expanded explicitly without dropping old obligations or bypassing the loader's bound.

There is also a test-fixture issue at the old maximum: appending a duplicate row to the live
100-entry manifest produces 101 rows, so the size check fires before the intended duplicate check.
The existing four-test suite exposes that single failure before this change.

## Proposed solution

Raise the explicit maximum from 100 to 128, retaining minimum 50. Shared constants drive the size
check and its diagnostic. Preserve all selection, source identity, frozen-overlap, directive, oracle
normalization, semantic-tag and filtering behavior. Keep the duplicate fixture at a valid length
by replacing its last row with the first instead of appending.

This is a separate infrastructure slice. The real manifest still contains exactly the same 100
entries, and the runner still executes exactly the same 300 selected cells. A full checkpoint is
required because the loader contract changes and this is the third implementation since full 214.

## Change summary

- `run-compute-discovery.py` defines minimum/maximum constants and uses them in validation and error text.
- `test-run-compute-discovery.py` exercises actual synthetic source contracts at 50, 100, 101 and 128
  entries; rejects 49 and 129; and keeps the duplicate fixture independent of current manifest size.
- WORKFLOW and the design note record the explicit bound and separate capacity from workload additions.
- The completed plan, full result manifest/census, this report and STATUS retain validation and provenance.

## Concepts and vocabulary

A _manifest identity_ selects one active compare-compute source contract by its ordinal. _Capacity_
limits how many explicit entries may be supplied before validation/filtering; it does not find or
add sources. _Selection metadata_ includes the source identity, original/normalized arguments,
filecheck oracle, tags and selected directive. A _full checkpoint_ runs every existing frozen and
discovery identity in NVRTC O3 and NVVM O0/O3, retaining known failures as failures.

## Process report

Fresh-context delegation remains unavailable at the app's agent-thread limit. The parent executes
and reviews this small infrastructure change locally under WORKFLOW's fallback; no independent
worker review is implied.

`_load_discovery_workloads` first reads the manifest and checks its cardinality. Its subsequent
loop rejects duplicate identities, overlap with frozen v1, missing source files and invalid active
directive ordinals. Target normalization keeps oracle arguments, and required semantic tags are
checked before returning selected workloads. Filtering in `main` remains after this loader. The
only production change is the maximum count and its diagnostic, sourced from the same constants.
No alternative loading path or validation bypass was introduced.

Before editing the loader, the new positive tests rejected 101 and 128 entries as expected; the
negative-range diagnostic assertions still displayed the old maximum. After the change, all six
contract tests pass. The synthetic helper writes real unique source files and manifest rows, then
calls the complete loader. Positive assertions compare the complete ordered ID list and tag counts,
not just the absence of an exception. Lower/upper invalid counts still reject. Existing target
normalization and frozen-overlap tests remain unchanged.

The duplicate test now replaces the final row with the first, retaining a valid manifest length.
It tests the same duplicate-source rejection even when the live manifest reaches a capacity limit.
This repairs an apparatus assumption rather than altering production duplicate detection. The original
four-test failure and final six-test before/after logs are retained under the slice's raw directories.

The helper/special-case inventory contains only `_write_synthetic_manifest` in the test class, shared
by the positive/negative boundary tests. It survives because it exercises real loader behavior at
otherwise-unrepresented sizes. Production adds two constants and no helper, fallback, special-case
source selection or compiler representation change. A compiler input-shape fix is not applicable.

Deterministically serialized before/after selection metadata for all 100 real entries is byte-identical,
and the checked-in manifest hash is unchanged. All source/oracle contracts therefore remain explicit
preservation obligations. No workload is added, removed, replaced or made expected-error.

The optimized compiler, provider, libraries and runtime input files match accepted 218 exactly.
No build is needed. Compiler units 478 plus one existing Windows-only skip and toolkit 18 results
inherit 218 explicitly, while discovery, routing/reporter and protocol contracts run freshly.
The full runtime checkpoint and six material compile/assembly cells are fresh; no material runtime
correctness or performance claim follows from assembly.

The first census command accidentally omitted `--workload-ids-from` and selected 525 currently
discoverable contracts. The parent caught the count during execution, stopped only the owned gate
script and process group, and retained the partial run under `excluded-unfrozen-partial`. None of
those results counts toward acceptance. The corrected command explicitly selects immutable
`census.slice-195.tsv`, reruns the small GPU gate after process termination, and runs all 452 frozen
identities freshly. Successful contract/routing/protocol gates remain valid on identical source.
Both command snapshots and the selection correction are retained; no baseline or source contract changed.
WORKFLOW now names the explicit frozen selector so future checkpoints do not rely on the runner's
unfiltered discovery default.

The corrected full checkpoint passes preservation review: all 1,656 cells are fresh, with 1,603
correct and the same 53 registered failures. Frozen contributes 1,333 correct, five infrastructure
failures and 18 preflight stops across 1,356 cells. Discovery contributes 270 correct, 22 infrastructure
failures, four output mismatches and four preflight stops across 300 cells. All five stable fields
match accepted outcomes exactly; there are no additions, omissions, duplicates or inherited runtime
cells. The 53 first-known failure records and four resolved histories remain intact.

Fresh gates pass: discovery contracts 6/6, routing/reporter 32/32, protocol 15/15, GPU smoke 4/4 and
material compile/assembly 6/6. Full corpus exit codes are two because registered failures remain.
Compiler units 478 plus the existing Windows-only skip and toolkit 18 explicitly inherit 218.
Local parent acceptance verifies 117 evidence references, 22 tested source hashes, 12 artifact hashes
and 548 runtime input hashes. The real manifest and deterministic selection metadata are unchanged.

Accepted locally on 2026-09-25 under the recorded delegation limitation. Slice 220 becomes the latest
full checkpoint and resets implementation cadence to zero. The next bounded slice may register and
admit FP64 masked min/max using the source behavior established in research 219.
