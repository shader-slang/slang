# Start a fresh NVVM session

Current full baseline: [validation293](runtime-validation.slice-293.json),
[parser report](report.slice-293-diagnostic-colors.md), with unchanged compiler285 bytes,
[array store report](report.slice-285-nested-array-stores.md) and
[plan285](plan.slice-285-nested-array-stores.md). The user requested stopping the development loop after slice296 on 2026-09-27.
Slice296 is independently accepted and closed with this local commit; the loop is stopped under WORKFLOW. An explicit resume is required
before another slice. Skip Slack; no push or system changes.

1. Read STATUS, WORKFLOW and RESULTS; inspect HEAD, working changes and dependency pins.
2. Qualified285 layout is installed at `build/RelWithDebInfo`. Verify all37 runtime identities,
   all576 main inputs and22pins. Actual compiler62469125/version301-g8fbf0f84e and provideraf1661de/ABI42
   come from source8fbf0f84e plus patch12f503e9. A later commit number is not the compiled identity.
3. Builds follow `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`, native tools and RESULTS
   prerequisites. Verified279 recovery is under `build/nvvm-nested-array-stores285/accepted279-layout`.
   Use fresh artifact roots; preserve modes, links, configuration and loaded libraries for experiments.
4. Limit builds/jobs to4CPU workers, units to2servers, gates to30minutes. Serialize builds/GPU/suites/
   benchmarks. Preserve failed attempts and exact comparisons; no retries hiding losses.
5. Full293 preserves580cases/576sources/1740cells:1703correct,37unresolved,20resolved histories.
   Units1097pass/13skip preserve1109old IDs plus1new; semantics1170pass/78skip preserve1248IDs.
   Runtime4/toolkit18/material6 pass. Last full293/targeted233/cadence0. Frozen195 immutable; discovery128.
6. Provider terminal whole stores conservatively use alignment1 where canonical array/member types
   contain a nested struct boundary. Existing direct struct splitting and authored signatures stay.
   Three new integer GPU fixtures cover root/wrapped/multidimensional arrays and saved values; all
   six NVVM modes pass, NVRTC controls remain wrong37. Flat neighbors pass; no new FP8 array admission.
7. Direct-vendor promotion qualifies constructed values, earlier-load snapshots, phi choices and
   real unaligned destinations with canaries. One new provider unit has39 shape/alignment cases;
   count65536 stays one store. First nine fixture syntax failures and five unrun dependents are retained.
8. All six material PTX/cubins and parsed resources equal279. Material runtime lacks binding/input/
   output contracts; no speed claim. Original and candidate large N65536 O3 stores both time out at
   120seconds/4GiB; no largeGPU launch or identical-cause claim. Research280–284 stays separately recorded.
9. [Research286](report.slice-286-generic-inference-observation.md) records eight main observations
   (7,995inferences/95,295rows each),21 strict duplicate pairs and broader overload-screening frequency
   leads. No safe cache/pruning or CPU-cost claim. All six material artifacts and1110/1248 native IDs
   are exact285. The observer is removed;100layout/37runtime/11source/2config/576inputs/22pins restored.
   Recovery and observer binaries are under `build/nvvm-generic-inference286`. Before production
   rebuild, refresh cached version metadata; restored source mtimes force observer objects to rebuild.
10. [Qualification287](report.slice-287-nested-dynamic-records.md) passes six nested dynamic and six
    neighboring GPU cells, plus two expected corruption rejections. IR confirms runtime-selected
    unpack/mutation/pack-back and saved interface values. Natural payload20/CUDA local24 remain
    distinct. All285 identities exact; raw fixtures stay under `build/nvvm-nested-dynamic287`.
    [Promotion288](report.slice-288-nested-dynamic-regressions.md) adds two persistent native sources
    and six fresh passing directives, preserving source tokens and qualified scope. No compiler or
    main-corpus change. [Qualification289](report.slice-289-error-handling.md) passes six original error-handling
    sources18cells/54words. Literal inputs may fold branches; witness/parameterized cases exercise
    success only. [Repair290](report.slice-290-catchall-oracle.md) adds the missing CHECK colon: identical corrupt
    output9 passes the old oracle and fails the fixed one; four normal CPU/CUDA positives preserve7.
    The test is outside the main corpus. [Qualification291](report.slice-291-runtime-errors.md)
    passes nine GPU cells/48words plus three IR compiles. Witness v2 did not observe its tag; v3
    catch+256 preserves final PTX tag-dependent selection. Generic/aggregate v2 stays qualified.
    [Promotion292](report.slice-292-runtime-error-regressions.md) adds two permanent tests; six fresh
    native directives pass with36 exact output words.285 identities and main selection stay unchanged.
    [Parser293](report.slice-293-diagnostic-colors.md) normalizes SGR styling at all three semantic
    text readers. All1740 outcomes and1110/1248 native identities remain exact285; runtime4, toolkit18,
    material6 and runner contracts46pass/1skip pass. Original290 failure stays failed.
    [Inference294](report.slice-294-inference-cost.md) completes the discarded CPU-cost study with
    overhead controls; see the current closeout below.267 already closed the aggregate-memory gap.
11. Prefer fresh bounded delegation and separate review;286 used reused author/reviewer after a fresh
    spawn hit the thread limit. Independent final review accepts source/method/evidence/restoration.
    Skip Slack; the latest user instruction stops the loop after296 closeout.

[HISTORY](HISTORY.md) owns earlier evidence. [Record contract](../docs/design/nvvm-substandard-record-contract.md)
separates local FP8/BF16 record admission from the integer-array store correction. Earlier timing
results retain their original identity and cannot be presented as fresh285 performance measurements.

## Slice294 closeout

[Inference cost294](report.slice-294-inference-cost.md) closes the286 frequency lead as a
prioritization study: observed OR-failure14.368ms/3.593% of instrumented root; whole7995-call
timer overhead18.594ms versus count-only. No exact uninstrumented cost, removable-work or
optimization claim. All40samples and qualification passed;100layout/37runtime/11source/2config/
576inputs/22pins restored. Sources restored294 have newer mtimes than experimental objects;
refresh version metadata and rebuild before any production build. Raw evidence/recovery archive:
`build/nvvm-inference-cost294`; accepted recovery remains286/accepted285-layout.

[Language295](report.slice-295-language-surface.md) completes four unchanged inheritance, mixed-width
initialization and scoped-constant contracts:12mode cells/48words, no skips/failures, exact identities.
Default/array/call folding and two lane0zero-sentinel limits remain explicit. No production change.
The following296 gate characterizes the excluded boundary without changing admission.

## Slice296 closeout and stop

[Boundary296](report.slice-296-record-array-boundary.md) accounts for18obligations:6GPU passes/22words,
6unsupported shader compiles,3unsupported IR captures,2existing negative units and1expected corruption
rejection with full buffer[32,0,65536,9321]. Mixed local/root/wrapper refuse var/OutParam<Array<Cell,2>>/
OutParam<Wrapper>. Final O0 Slang IR retains those canonical shapes and whole-copy helpers; no mixed
LLVM/PTX or CUDA/provider layout proof. Integer control passes allthree modes.

The NVRTC classifier mistakes a source path containing nvrtc-o3 for a compiler error. Exact saved
FileCheck/output/count evidence supports a reviewed runtime-mismatch adjudication; original raw
failed/infrastructure evidence remains. Continuation runs only the five previously unrun obligations.
All100layout/37runtime/11source/2config/576input/22pin identities are unchanged285; full293 remains
inherited. Raw evidence is build/nvvm-record-arrays296; final296 independent acceptance is complete.
This local commit closes296.

**Stopped after296** under the latest user request. A bounded path-sensitive parser repair with regression
coverage and its required full checkpoint is queued, not started. Resume only on explicit user request.
No array admission; do not reopen267materialgap or largeN65536timeouts. Main580cases/576files/1740cells
and full293/targeted233/cadence0 remain inherited. SkipSlack; no push/systemchanges.
