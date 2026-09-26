# Publish the local NVVM results package and stop

## Motivation

Monday's presentation needs reproducible evidence for material compilation and simple-shader code
quality after master integration. The same workflow must refresh the package after later accepted
changes and let a fresh session continue without reconstructing chat history.

## Proposed solution

Use accepted262's explicitly composite correctness baseline and unchanged optimized binaries.
Run the maintained material, quality and bounded shared-session commands in isolation, retain every
attempt, and export compact reports/charts with a short narrative and presentation outline. No
compiler optimization, workload change or general development-loop restart belongs in this slice.

## Change summary

[The package](results/2026-09-26/README.md) contains generated material/quality summaries, SVG/PNG
figures, structured comparison metadata, a shared-session appendix and a six-slide outline.
RESULTS documents refresh/copy commands and reviewed baseline selection; STATUS/HANDOFF/HISTORY
point to current evidence. Historical ledgers and raw failed attempts remain intact.

## Concepts and vocabulary

Fresh-process wall time includes process creation through exit, with warmed filesystem/toolkit
caches. Request service time measures a narrower shared-process operation. Named-entry registers,
stack and spills differ in scope from whole-module executable text. These are compile/resource
observations, not GPU execution performance. Accepted262 includes a retained parallel PCH incident
with one explicitly substituted serial outcome; it is not a clean parallel-run claim.

## Process report

Workspace 201cea6c9 measured compiler 49593da72 (`2026.18.3-275-g49593da72`) with runner c3455e606,
provider ABI42, CUDA12.9.2/NVRTC12.9.86, SM80 target on L4/driver580.126.09. No competing build,
GPU suite or profiler ran. Material completes132 compiles and 66 assemblies; quality36/36; shared
execution66 requests in 11 completed batches plus 6 fresh references. All succeed; output hashes are
stable and shared PTX matches fresh references. No sample is removed or retried.

Material NVVM O3 medians are 1355.04ms evaluation and 1460.18ms sampling versus NVRTC 1362.76/1390.16ms.
Evaluation is near parity (overlapping IQR); sampling is 5.04% slower. NVVM assembly is slower for both.
O3 material entry registers48→67 and63→86, stack592/624→784bytes, with zero spills. The fixed 12-shader
O3 subset has 1 lower/10 equal/1 higher register counts; executable text is smaller in 3 and equal in 9.
All 24 O3 entries have zero spills. No broad speed/code-quality advantage is claimed. SASS is unavailable
because the selected toolkit lacks cuobjdump; no kernel timing or material runtime contract exists.

Shared six-request process lifetime median is 6.263s; fresh-reference work 11.130s; all 11 batch lifetimes
68.922s; runner work 83.296s; whole command 83.356s. These overlapping scopes stay separate. Fixed request
order exposes the first request to different initialization work, so the appendix is not a backend
ranking or paired end-to-end speedup. CLI logs do not establish automatic-PCH cache state; default
math settings are source-inferred rather than an observed downstream option trace.

Independent review recomputed medians, checked all inventories/output hashes and reviewed quality
tradeoffs and shared accounting. Both rendered figures were inspected. SVG trailing whitespace was normalized; parsed XML tokens
match the originals, while summaries and PNGs remain byte-identical. No new compiler helper,
fallback or special case exists in this slice. The package uses maintained commands; the build-local
export was ordinary copying/summary assembly, not new benchmark machinery. Complete source/tool hashes
remain in raw evidence, with compact tool identities and hashed references in exported reports.

The finite consolidation→merge→validation→results sequence is complete. Further priorities are
recorded for discussion, not selected as implementation work. The development loop is **stopped**.
