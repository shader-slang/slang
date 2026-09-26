# Refresh the material explanation and reproducible stage report

## Motivation

Monday's material results need an explanation of shared compiler time, direct backend costs,
aggregate layout and retained arithmetic. Future work must refresh the evidence without manually
reconstructing charts or accidentally presenting source-level probes as compiler improvements.

## Proposed solution

Add explicit optional stage reporting to the existing harness. Present the original accepted263
baseline alongside qualified research264 in a new explanatory package. Keep accepted262 correctness,
all original results and the restored compiler unchanged, then stop the finite follow-up.

## Change summary

`nvvm-results.py report --stage-attribution` validates scope logs and generates structured/Markdown
tables plus SVG/PNG charts. Focused contracts cover aggregation and rejected inconsistent evidence.
[The new package](results/2026-09-26-attribution/README.md) contains a six-slide outline, complete
stage tables and resource explanation. RESULTS/WORKFLOW/HANDOFF/STATUS preserve refresh and stop rules.

## Concepts and vocabulary

Disjoint stages partition each sample into builtin loading, Slang front end, Slang IR linking,
target preparation/emission, separate vendor verification, vendor compile and remaining wall time.
Marginal medians are independent summaries and need not sum to the wall median. Vendor APIs include
opaque work; NVRTC's missing separate verify call is not absence of validation. Inherited results
keep their original identities; a source counterfactual changes the shader, not the compiler.

## Process report

The reporter reuses existing measurement validation, statistics and provenance helpers. Optional
stage reporting requires material records and raw phase names/counts/values that agree. Nested
scopes and outer builtin/front/output intervals must fit their parent durations, with explicit
0.01ms display-resolution tolerance. Fractions and residuals are formed per sample; tiny negative
rounding residuals are counted and clamped only within the documented tolerance. Actual264 has no
such rounded residuals. Both rounds and IQRs remain visible; the chart uses independent bars.

Independent review found a missing outer-interval containment check; the final implementation
rejects that case and adds a regression contract. Sixteen harness contracts pass, including
per-sample versus marginal arithmetic, warmup exclusion, round separation, NVVM nested serialization,
missing/mismatched raw scopes and impossible timings. All252 real stage distributions independently
match the raw measurements. Old material/quality JSON, Markdown and PNG replays are byte-identical.
The optional reporting path does not alter collection, classifiers, accepted outcomes or old reports.

The package inherits accepted263 timing/quality and262 correctness. New attribution exports use
264's132 compiles/66assemblies and exact-output qualification; no compiler timing is rerun here.
Resource claims remain bounded: evaluation's192 extra stack bytes are layout, zero spills; six
extra exponentials are concrete retained work, but exact register cost and GPU performance are
unknown. The general probe is not yet differential. No compiler optimization was promoted.

Generated JSON/Markdown/PNG copies are unchanged; SVG trailing whitespace is normalized with
recursive XML-token equivalence checks. The chart is visually inspected. A first export check
compared whitespace inside SVG path attributes literally and failed; the corrected check compares
their whitespace-separated tokens, without changing geometry or measurements. Raw reports remain
under `build/nvvm-material-followup/package265`; original263 is untouched.

New helpers are reporting transformations only, with explicit validated input contracts; no compiler
helper/fallback/special case is added. A full compiler checkpoint is unnecessary for this report-only
change. Accepted262 and implementation cadence0 remain current; the finite follow-up and general
development loop are **stopped** after local commit and the detailed completion DM. No push.
