# Consolidate NVVM results and continuation tooling

## Motivation

A fresh results session previously depended on scripts in ignored slice257/260 directories and a
long STATUS history. Correctness preservation and material timing should be reproducible from tracked
commands, while keeping the accepted compiler and original evidence unchanged.

## Proposed solution

`nvvm-results.py` provides compare, checkpoint, material, quality and report commands. It reuses the
existing census/discovery/complex compiler contracts. Exact inventory and five-field comparisons
reject missing, duplicated, changed and false-passing cells. Only explicitly reviewed `accepted-full`
baselines are admitted, preventing a rejected result from silently becoming the next baseline.

## Change summary

- `nvvm-results.py`, its contract tests and the fixed quality manifest maintain the reusable pipeline.
- `RESULTS.md` records current commands, complete baseline gates, promotion schema, warmed-cache
  timing interpretation and refresh steps. `requirements-results.txt` isolates optional plotting.
- STATUS/WORKFLOW/AGENTS use compact future reporting; HISTORY and HANDOFF preserve navigation and
  fresh-session authority. Historical plans, reports, workload inputs and compiler source are intact.
- Matplotlib reports produce structured summaries, Markdown and SVG/PNG charts. Material charts show
  repeated wall median/IQR; quality charts compare named-entry O3 registers, never single-run speed.

## Concepts and vocabulary

A cell is a workload/mode pair. The five-field outcome includes classification, process return code,
executed/passed/ignored counts, diagnostic and canonical shape. A material sample is a fresh process,
with filesystem/toolkit caches intentionally warmed; it is not a cold-cache measurement. Quality
observations describe assembled code/resources, not measured kernel speed. Accepted-full denotes
reviewed full gates and preservation, not merely a runner's zero exit status.

## Process report

Consider a new result missing one NVVM O0 cell. A runner exit or matching total alone could conceal
that loss. `index_outcomes` rejects duplicate/false passing rows; `compare_rows` compares exact keys,
requires all three modes, and retains each changed field. `accepted_baseline` refuses review-required
outputs, so rerunning the changed data cannot erase the original obligation. Histories remain in the
compact ledger; intentional upstream changes require explicit review rather than a permissive mode.

For material timing, `expected_inventory` defines the established two opposite-order rounds with two
warmups and nine samples per cell, followed by separate assembly samples. `validate_measurements`
checks against the authoritative manifest, not a potentially truncated observed cell list. Every
sample, failure, timer and PTX/cubin remains on disk; mutated outputs, missing phases and invalid
numbers fail reporting. `configure` identifies selected libraries, compiler/cache, source and input
bytes; `verify_identity` rejects changes during a run. `run` kills a timed-out process group and keeps
its log. These are runner representation contracts, not compiler AST/IR workarounds.

Quality uses12 fixed previously correct sources/36 modes. Row-major fixture options are explicit.
It requires matching accepted runtime IDs, source hashes and compiler/provider identity. Named ptxas
function blocks prevent helper resource rows from being confused with kernel entry metrics. SASS and
executable text sizes have whole-module scope; absent metrics remain unavailable. Compile quality
latencies are raw single observations and are not the headline figure. Material runtime contracts
remain unavailable; no new capability or GPU runtime result is claimed.

[Maintenance validation](maintenance-validation.slice-261.json) retains the compact evidence.
Validation is CPU-only:11 new acceptance-contract tests pass; existing discovery6 pass and complex14
pass/1skip. The final accepted260 replay preserves all1713 cells (1674 correct/39 unresolved) and
39 unresolved/18 resolved history records without reading historical raw paths from the baseline.
Historical257 report replay validates132 compiler/66 assembly artifacts and regenerates summary,
SVG and PNG from the retained samples; it does not rerun timing. Raw maintenance evidence lives under
`build/nvvm-maintenance/slice-261-*`. No build, GPU suite, compiler change, push or driver change.

Independent read-only review identified and resolved incomplete library/cache provenance, manifest
inventory truncation, invalid numeric times, multi-execution false passes and rejected-baseline
laundering. The initial formatting attempt lacked PATH entries; the final run uses the host's existing
formatter tools. Parent acceptance/commit is followed by separately planned master integration and
postmerge full gates; the general development loop remains stopped.
