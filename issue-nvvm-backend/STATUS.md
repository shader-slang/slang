# NVVM backend status

The general development loop is **stopped**. The maintainer authorized only: consolidate reusable
results tooling, merge master, validate correctness, produce the results package, then stop. No push.
Read [WORKFLOW](WORKFLOW.md), [RESULTS](RESULTS.md), and the current
[maintenance plan261](plan.slice-261-results-harness.md). [HANDOFF](HANDOFF.md) covers a fresh session.

## Accepted compiler baseline

Slice260 adds scalar E4M3/E5M2 -> Float32 widening under provider ABI42. Finite values, signed zeros
and infinities are exact; NaNs preserve classification. Reverse narrowing, aggregate/vector/storage
and external-helper roles remain excluded. [Report260](report.slice-260-fp8-widening.md) explains the
change; [validation260](runtime-validation.slice-260.json) is the authoritative outcome/history ledger.

| Evidence                                         | Accepted260 result                                             |
| ------------------------------------------------ | -------------------------------------------------------------- |
| Frozen / discovery                               | 1356 / 357 cells                                               |
| Combined                                         | 1713 total; 1674 correct; 39 unresolved; 18 resolved histories |
| Units / semantic regressions                     | 1051 pass + 13 skip / 1052 pass + 77 skip                      |
| Runtime / toolkit / contracts / material support | 4 / 18 / 6 / 6 pass                                            |
| Last full / targeted / implementation cadence    | 260 / 233 / 0                                                  |
| Rolling implementations                          | 256 local BF16 vectors; 259 local records; 260 FP8 widening    |

Compiler SHA256: `4b59d082df60a6862e684f63f099105228d0b820113aba456e83abc3661aca7d`.
Provider SHA256: `2e54768ba323ba86cb7767a0a4405c9d4212e36f67e62786fd812dd59532e9f1`.
Material PTX/cubins remain exact259. Material runtime is unassessed because binding, texture/LUT,
input and output contracts are unavailable. The failure ledger retains texture/column-major and
other existing gaps; unchanged counts do not mean all workloads are supported.

## Maintenance handoff

Slice261 adds maintained results commands and compact documentation; it changes no compiler behavior.
Its accepted260 replay is historical validation, not a new GPU checkpoint. Tooling acceptance and
review are complete; the next authorized step is separately planned master integration. After merging, rebuild matching tools and
run full acceptance before freezing any new baseline or measurements. Review every upstream input,
submodule, diagnostic and outcome delta. Do not imply old binaries validate merged source.

Environment last measured: native Ubuntu24.04, L4 SM89, driver580.126.09, SM80 target, CUDA12.9.2,
NVRTC12.9.86, LLVM14, RelWithDebInfo. Recheck it; commands are in RESULTS. Four CPU workers total,
two unit servers, sequential GPU suites and isolated timings. Raw260 evidence lives under
`build/nvvm-loop/slice-260-{before,after}`; maintenance evidence under `build/nvvm-maintenance`.

Research258's discarded material experiment explains the recent capability-cadence exception.
Historical candidates are evidence, not permission to restart. [HISTORY](HISTORY.md) links durable
architecture, inventories and selected prior findings without duplicating their reports.
