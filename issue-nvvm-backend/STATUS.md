# NVVM current status

The maintainer authorized continued work on 2026-09-27. Continue the normal development loop through
independently reviewed local commits. Regressions or decisions requiring maintainer input stop
continuation. This supersedes the earlier stop-after-slice instruction. Skip Slack; no push or system
changes. Keep working plans, reports and raw artifacts ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records full validation of local fixed identity-record
arrays. [Accepted identity](accepted-identity.json) pins installed compiler/provider/modules/configuration
and layout. [Focused evidence](focused-evidence.json) preserves separate qualifications and their original
identities; older timing results have not been remeasured under this compiler.

| Evidence                                          | Accepted result                                                              |
| ------------------------------------------------- | ---------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                            |
| Main outcomes                                     | 1,703 correct; 37 unresolved; 20 resolved histories; all old cells preserved |
| Frozen / discovery                                | 1,356 / 384 cells                                                            |
| Native units                                      | 1,115 identities: 1,102 pass, 13 skip; three new units                       |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                             |
| Focused regression                                | 45 GPU mode cells, 24 shared units, 2 direct static units pass               |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                              |
| Material runtime                                  | 6 fresh default cells pass; both entries, 65 records and 63 guards per cell  |
| Runner contracts                                  | 90 pass, 1 inherited skip                                                    |
| Last full / targeted / implementations since full | local-record-arrays / local-record-arrays / 0                                |

Compiler source: `01ecb6e8badbf3cea4770ed8e047693ca0123cb5` plus patch
`989b5d610b6a596afdc7f8e04bd87207ad82e99c0e00d79bc1809960f13ee7da`; version `2026.18.3-323-g01ecb6e8b`.
Loaded compiler SHA256 `b918103eafb90edd687a2f6ded774730209a8b96a17edc853db32090cf489d9d`; provider ABI42 SHA256
`af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
Later Git commits do not identify rebuilt bytes. Qualification uses native Ubuntu24.04, L4 SM89,
driver580.126.09, CUDA12.9.2/NVRTC12.9.86 and LLVM14, targeting SM80. Installed layout is
`build/RelWithDebInfo`; raw acceptance artifacts are under `build/nvvm-local-record-arrays/`.
The direct static tests use a separately pinned compiler with the same production source and provider.

## Boundaries and next action

- Ordinary allocation/load/store and field/index emission consume checked plans. Recursive initial
  admission, helper-signature classification, dedicated resources and structured conversions remain
  explicit debt. A transforming local-storage pass remains deferred: the reviewed bounded rewrite
  would add machinery while leaving the existing conversion family necessary.
- Local fixed arrays of existing integer/FP8/BF16/BF2 identity records support dynamic field mutation
  and whole snapshots. Exactly two historical mixed-local NVVM cells are now correct. Array-bearing
  helper signatures, wrapper fields, multidimensional arrays, BF3/BF4 elements and external storage
  remain excluded. Two-element exhaustive bit transport does not establish large-array scalability.
- Three original graphics-packed column-major mismatches and 34 infrastructure/preflight gaps remain
  in the main corpus. The permanent compact CUDA matrix test passes; raw uploads do not repack host
  data. Three focused NVRTC nested integer-array wrong-output controls remain outside the main corpus;
  two large vendor experiments timed out. Their histories and limits remain in focused evidence.
- The tiled-brass material's synthetic-texture eval/sample contracts pass fresh correctness gates.
  Prior device-event measurements, eval local-store deletion control and dynamic-index source
  reduction retain their tested compiler identities. They explain a bounded synthetic eval gap,
  without justifying a production PTX deletion or changing receiver-copy semantics. Original assets,
  live LUTs, arbitrary inputs and application performance remain open.
- Select the next bounded workload-driven capability through the existing checked type/address
  boundaries. Array helper roles are a separate decision; do not broaden generic helper predicates
  merely because local Value/Storage now work. Further vendor optimization research is secondary.
