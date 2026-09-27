# NVVM backend status

The development loop is **active**, resumed on 2026-09-26. Continue bounded reviewed local commits
under [WORKFLOW](WORKFLOW.md); skip Slack, no push or system changes. Read [HANDOFF](HANDOFF.md)
and [RESULTS](RESULTS.md). [HISTORY](HISTORY.md) owns earlier evidence and failure histories.

## Accepted checkpoint

[Validation293](runtime-validation.slice-293.json) refreshes full correctness with the corrected
SGR diagnostic parser. Compiler/provider bytes remain accepted285; this was not a rebuild.
[Record contract](../docs/design/nvvm-substandard-record-contract.md) owns the qualified language domain.

| Evidence                                          | Accepted293 result                                  |
| ------------------------------------------------- | --------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                   |
| Frozen / discovery cells                          | 1,356 / 384                                         |
| Outcomes                                          | 1,703 correct; 37 unresolved; 20 resolved histories |
| Units / semantics                                 | 1,097 pass + 13 skip / 1,170 pass + 78 skip         |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                     |
| Runner contracts                                  | 46 pass + 1 inherited skip                          |
| Last full / targeted / implementations since full | 293 / 233 / 0                                       |

All 576 main inputs, 22 pins and five-field outcomes are unchanged from285. Discovery stays128 sources;
focused later tests are outside this selection. [Inventory269](report.slice-269-corpus-enumeration.md)
retains its historical authored-test counts and selection-versus-semantic-coverage limits.

Qualified compiler source: `8fbf0f84e` plus patch
`12f503e9802014f667dcfe2a3224a892c26a567dc7af3999888fa997ef8cc7f5`, version
`2026.18.3-301-g8fbf0f84e`. Loaded compiler SHA256:
`624691257742cd8bff21c6c10b5777c50372785f2e6c21c6133128a2343b1d2c`.
Provider ABI42, SHA256 `af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
Installed `build/RelWithDebInfo` has 100 layout entries/37 runtime identities exact285. Recovery:
`build/nvvm-generic-inference286/accepted285-layout`. A later HEAD is not compiled identity.
Before a production rebuild, refresh version metadata and rebuild restored294 observer sources;
their mtimes are newer than experimental objects.293 preserves both configuration hashes.

## Results, limits and next action

[Parser293](report.slice-293-diagnostic-colors.md) fixes recognition of colored FileCheck errors while
preserving raw logs, failure precedence and strict execution counts. The original290 failed shader
stays failed; this is reporting accuracy. [Promotion292](report.slice-292-runtime-error-regressions.md)
and [promotion288](report.slice-288-nested-dynamic-regressions.md) retain runtime error and nested
dynamic-dispatch regressions with original qualification limits. Their evidence is inherited here.

[Inference294](report.slice-294-inference-cost.md) measures failed OR spans at14.368ms/3.593% of
instrumented semantic CPU. Global7995-call timer overhead is18.594ms versus count-only; no precise
uninstrumented saving or safe pruning follows. All40samples passed; accepted layout/source/config
restored exactly. Deprioritize OR pruning and select bounded runtime-observable language interactions
from sparse inheritance/initialization/type regions. The aggregate-memory gap remains fixed by267.

Three column-major host-packing mismatches and34 infrastructure/preflight gaps remain.285's three
focused NVRTC nested-array controls still produce wrong37 outside the main corpus; both large
N65536 O3 vendor experiments exceed120seconds/4GiB. No new FP8/BF16 record-array or resource/shared/
readonly/exported-role admission. All six material PTX/cubin/resource sets equal285 (and279).
Material runtime lacks binding/input/output contracts; no runtime or speed claim.

Native Ubuntu24.04, L4SM89/driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14;
max4CPU workers,2unit servers, serialized suites. Independent review owns each accepted slice;
raw artifacts and recovery layouts remain under ignored build roots.
