# NVVM backend status

The development loop is **active**, resumed on 2026-09-26. Continue bounded reviewed local commits
under [WORKFLOW](WORKFLOW.md); skip Slack, no push or system changes. Read [HANDOFF](HANDOFF.md)
and [RESULTS](RESULTS.md). [HISTORY](HISTORY.md) owns earlier evidence and failure histories.

## Accepted baseline

[Validation285](runtime-validation.slice-285.json) owns the last full checkpoint.
[Record contract](../docs/design/nvvm-substandard-record-contract.md) owns the qualified language domain.

| Evidence                                          | Accepted285 result                                  |
| ------------------------------------------------- | --------------------------------------------------- |
| Selected cases / sources / mode cells             | 580 / 576 / 1,740                                   |
| Frozen / discovery cells                          | 1,356 / 384                                         |
| Outcomes                                          | 1,703 correct; 37 unresolved; 20 resolved histories |
| Units / semantics                                 | 1,097 pass + 13 skip / 1,170 pass + 78 skip         |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                     |
| Last full / targeted / implementations since full | 285 / 233 / 0                                       |

All576 main input hashes and22 dependency pins remain unchanged. Discovery stays128 sources; later
focused tests are outside this main selection. [Inventory269](report.slice-269-corpus-enumeration.md)
retains its historical authored-test counts and selection-versus-semantic-coverage limits.

Qualified source: `8fbf0f84e` plus patch
`12f503e9802014f667dcfe2a3224a892c26a567dc7af3999888fa997ef8cc7f5`, version
`2026.18.3-301-g8fbf0f84e`. Loaded compiler SHA256:
`624691257742cd8bff21c6c10b5777c50372785f2e6c21c6133128a2343b1d2c`.
Provider ABI42, SHA256 `af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
Installed layout is `build/RelWithDebInfo`; all37 runtime identities and100 layout entries are exact285.
Verified285 recovery is `build/nvvm-generic-inference286/accepted285-layout`. A later source HEAD or
launcher hash is not compiled identity. Before a production rebuild, refresh cached version metadata
and rebuild restored286 observer source files; their mtimes are newer than experimental objects.

## Results, limits and next action

[Correction285](report.slice-285-nested-array-stores.md) fixes NVVM nested integer-array store padding
without array expansion or changed SSA/signatures. NVRTC's three optimized wrong37 controls remain
open; original/candidate N65536 O3 modules both exceed120seconds/4GiB. No FP8/BF16 record-array or
new resource/shared/readonly/exported-role admission. All six material PTX/cubin/resource sets equal279.
Material runtime still lacks binding/input/output contracts; no runtime or speed claim.
Three column-major host-packing mismatches and34 infrastructure/preflight gaps remain unchanged.

[Material286](report.slice-286-generic-inference-observation.md) retains a broader overload-screening
frequency lead, not a safe optimization or CPU-cost result. Temporary instrumentation is removed.
[Nested dynamic287](report.slice-287-nested-dynamic-records.md) proves live runtime selection,
mutation pack-back and preserved snapshots across Natural20/CUDA24 layouts. [Promotion288](report.slice-288-nested-dynamic-regressions.md)
adds two persistent native fixtures with six passing focused directives; qualified exclusions remain.

[Error contracts289](report.slice-289-error-handling.md) passes18cells/54words from six unchanged
sources. Literal inputs may fold control flow; witness/parameterized cases exercise success only.
[Oracle repair290](report.slice-290-catchall-oracle.md) adds one missing CHECK colon. Identical corrupt
output9 passes the old check and fails the fixed check; four normal CPU/CUDA positives preserve7.
Broader289 evidence retains its original source hash. Main corpus and checkpoint cadence are unchanged.
[Runtime errors291](report.slice-291-runtime-errors.md) passes nine GPU cells/48words and three IR
captures across two versions. Generic/aggregate calls consume live tags/payloads. Initial witness
payload-only output did not observe its tag; refined catch+256 now retains tag-based PTX selection.
All original results/limits remain recorded and accepted285 identities are exact.
[Promotion292](report.slice-292-runtime-error-regressions.md) adds the final two native tests; all six
directives pass with36 exact output words. Inputs, noinline calls and tag-distinct catches remain.
Next: repair maintained census ANSI-color normalization using290’s preserved FileCheck failure.
Shared-runner changes require a full checkpoint. No compiler change is motivated; main corpus and
checkpoint cadence remain unchanged.

Native Ubuntu24.04, L4SM89/driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14;
max4CPU workers,2unit servers, serialized suites. Independent review owns each accepted slice;
raw artifacts and recovery layouts remain under ignored build roots.
