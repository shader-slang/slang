# Probe four language interactions on the accepted NVVM compiler

This bounded research ExecPlan follows `.agent/PLANS.md` and the explicit AGENTS exception requiring
completed NVVM plans/reports in slice commits. The development loop remains authorized by the user's
2026-09-26 resume instruction. Lead owns scope, records and local commits; one worker owns execution.
Fresh worker `/root/breadth_writer271` owns raw execution; `/root/breadth_review271` independently
reviews source/oracle/results without execution. Skip Slack notifications. No compiler, provider,
main inventory or system changes are authorized here.

## Purpose and Observable Result

Determine whether four existing Slang language interactions execute correctly at NVRTC O3 and NVVM
O0/O3 on accepted270. Reuse complete source bodies and original expected-output contracts. Preserve
all twelve outcomes, including failures, and identify the next responsible boundary from evidence.
This is selected-test coverage, not a percentage of the whole language or a new full checkpoint.

## Progress

- [x] 2026-09-26: Accepted270 committed as17d33c857; full1,740 cells qualified, cadence zero.
      Read WORKFLOW/STATUS and prepare four exact source/directive/oracle contracts.
- [x] Verify accepted runtime artifacts, four source hashes and exact original directives.
- [x] Execute exactly twelve cells with maintained parser/adaptation/oracle mirroring; inspect outputs.
- [x] Independent source/oracle/results review, compact outcome/provenance record and five-part report.
- [x] Format, local research commit, update STATUS/HANDOFF/HISTORY and choose next bounded slice.

## Context, Scope and Invariants

Accepted270 has580 cases/576 files/1,740 cells; discovery remains at128 sources. Do not replace
manifest entries, change capacity constants, filter away failed cells or rewrite shader bodies.
The following native zero-based test ordinals select CPU COMPARE_COMPUTE directives; only their
backend option is adapted by maintained discovery logic. Other source directives are not executed.

| Source under tests/                                                    | Ordinal | Preserved independent oracle                                                 |
| ---------------------------------------------------------------------- | ------- | ---------------------------------------------------------------------------- |
| language-feature/switch-fallthrough/fallthrough-loop-interaction.slang | 3       | Hex73,64,C8,FFFFFFFF:115,100,200,-1                                          |
| language-feature/lambda/lambda-0.slang                                 | 1       | Captured scalar2 maps local array; selected output8.0                        |
| language-feature/tuple/tuple-basic.slang                               | 2       | Unanchored CHECK hex4; independently inspect first output for mutation proof |
| language-feature/error-handling/defer-interaction.slang                | 2       | Normal/throw paths and nested defer order:hex2,4,FF,80,3                     |

Tuple's unanchored CHECK4 can match the unchanged fourth input element; passing that original
contract alone does not prove mutation. Independently inspect the first actual output in each mode
before claiming selected-result correctness. Lambda also searches a selected value; do not claim
whole-buffer coverage.
Preserve shader-object/output-format flags, CHECK prefixes, input buffers and expected sidecars.
No fixture or production implementation is planned. A failure leads to a separate bounded slice,
not an unreviewed expansion of this probe. The accepted270 compiler identity remains precommit
06a26a3f7 plus patcha35e26dc; the installed version is2026.18.3-286-g06a26a3f7.

## Milestones and Execution

1. Verify all37 runtime artifact hashes and22pins against validation270. Record fresh HEAD/source
   hashes and installed compiler/provider identities under ignored `build/nvvm-breadth271`.
2. Build an ignored focused driver from maintained `run-compute-discovery.py` helpers
   `_find_compare_directive`, `_adapt_arguments_to_cuda`, `_prepare_mirror_tree` and
   `_populate_mirror_for_mode`, plus census `run_mode`, `inventory_matches` and `_write_result_files`.
   Construct exactly four explicit workloads. Do not monkeypatch manifest limits or use padding.
   Preserve original/adapted command, source body identity and copied oracle sidecars.
3. Execute the three modes serially with jobs1 and bounded subprocesses; retain exactly12 requested
   cells. Use accepted `build/RelWithDebInfo/bin/slang-test` and provider ABI42 under CUDA12.9.
   No concurrent build/GPU/performance work. Bound the whole experiment to30minutes.
4. Check each `(id,mode)`, return code, executed/passed/ignored counts, diagnostic and canonical shape.
   Independently read each oracle/source and raw logs. Successful cells require actual GPU execution;
   unsupported/reference failures remain failures rather than skips or inferred passes.
5. Record a compact twelve-cell research result with full source/provenance identities and references
   to accepted270. Inherit main corpus/failure maps with their original identities; do not rerun or
   reset full-checkpoint cadence without a trigger. Root/independent review then local commit.

## Decision Log

- 2026-09-26, lead: Select four runtime language interactions missing from the earlier breadth slice,
  preserving existing oracles and discovery capacity. Research only: no speculative compiler change.
- Next priority after this bounded evidence is material-driven work, with current profiling before
  selecting an optimization. Newly found correctness failures can take priority under WORKFLOW.

## Surprises and Discoveries

Independent source review found the tuple CHECK false-positive possibility above; retain original
contract and supplement with actual output inspection. Root preparation is `build/nvvm-fp8-aggregate270/next-breadth-notes.md`.

## Validation, Failure and Recovery

Acceptance means twelve complete truthful outcomes with reviewed source/oracle/provenance, not that
all twelve must pass. Retain any failed attempts and diagnose enough to classify the responsible
boundary. A source modification, runner change or compiler fix requires a separately reviewed plan.
Unavailable resources or an external API decision requiring human input is a stopping condition;
routine local execution, review and commits are already authorized.

## Outcomes and Retrospective

All twelve cells passed with independently verified actual outputs. All four sources are absent
from the main inventory. The tuple oracle weakness was resolved as an evidence issue through raw
first-element inspection, without source changes. Lead and independent review accepted all35
compact references, source bodies and identities. Local research commit closes271; next is material
profiling/code-quality refresh. Main accepted baseline remains270; this research cannot erase its37 unresolved failures,
20 resolved histories or qualification limits. Continue the loop after the bounded research closeout.
