# NVVM current status

The maintainer authorized continued work on 2026-09-27. Continue the normal development loop through
independently reviewed local commits. Regressions or decisions requiring maintainer input stop
continuation. This supersedes the earlier stop-after-slice instruction. Skip Slack; no push or system
changes. Keep working plans, reports and raw artifacts ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records full validation of internal value parameters for fixed
identity-record arrays. [Accepted identity](accepted-identity.json) pins installed compiler/provider/modules/configuration
and layout. [Focused evidence](focused-evidence.json) preserves separate qualifications and their original
identities; older timing results have not been remeasured under this compiler.

| Evidence                                          | Accepted result                                                                    |
| ------------------------------------------------- | ---------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                  |
| Main outcomes                                     | 1,703 correct; 37 unresolved; 20 resolved histories; all old cells preserved       |
| Frozen / discovery                                | 1,356 / 384 cells                                                                  |
| Native units                                      | 1,118 identities: 1,105 pass, 13 skip; three new units                             |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                   |
| Focused regression                                | 51 GPU mode cells, 28 shared units, 2 direct static units pass                     |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                    |
| Material runtime                                  | 12 cells pass across default/filtering profiles; 65 records and 63 guards per cell |
| Runner contracts                                  | 97 pass, 1 inherited skip                                                          |
| Last full / targeted / implementations since full | record-array-parameters / record-array-parameters / 0                              |

Compiler source: `cef0ffbdfd2d9e2f3b820967bb05ebbe225d07dd` plus patch
`3a1a750089b782f3486c0ac6afac2e50a28157f2f95831dfd9246e0dffa06453`; version `2026.18.3-326-gcef0ffbdf`.
Loaded compiler SHA256 `a924e219d49407a54febe9eb49520fe361e4a102e7ea294190a3c58915cb659d`; provider ABI42 SHA256
`af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
Later Git commits do not identify rebuilt bytes. Qualification uses native Ubuntu24.04, L4 SM89,
driver580.126.09, CUDA12.9.2/NVRTC12.9.86 and LLVM14, targeting SM80. Installed layout is
`build/RelWithDebInfo`; current raw acceptance artifacts are under `build/nvvm-record-array-helpers/`.
The direct static tests use a separately pinned compiler with the same production source and provider.
Both material profiles and the separate imported mini-LUT fixture are included in fresh correctness
regressions. Original full corpus inputs/outcomes and all old native/semantic identities are preserved;
older device-event measurements retain their original compiler identity.

## Boundaries and next action

- Ordinary allocation/load/store and field/index emission consume checked plans. Recursive initial
  admission, helper-signature classification, dedicated resources and structured conversions remain
  explicit debt. A transforming local-storage pass remains deferred: the reviewed bounded rewrite
  would add machinery while leaving the existing conversion family necessary.
- Local fixed arrays of existing integer/FP8/BF16/BF2 identity records support dynamic field mutation
  and whole snapshots. Internal value parameters also preserve caller snapshots and callee-local
  mutation; two newly probed NVVM parameter cells become correct. Array results still lower to
  OutParam and remain excluded, along with other references, exports, wrapper fields, multidimensional
  arrays, BF3/BF4 elements and external storage. Two-element bit transport is not large-array evidence.
- Three original graphics-packed column-major mismatches and 34 infrastructure/preflight gaps remain
  in the main corpus. The permanent compact CUDA matrix test passes; raw uploads do not repack host
  data. Three focused NVRTC nested integer-array wrong-output controls remain outside the main corpus;
  two large vendor experiments timed out. Their histories and limits remain in focused evidence.
- The tiled-brass material's synthetic-texture eval/sample contracts pass texel-center and fixed
  off-center filtering correctness gates. Interpolation precedes color decode; both wrap seams are
  exercised by the selected footprint. Arbitrary UVs and texture quantization remain unqualified.
  Prior device-event measurements, eval local-store deletion control and dynamic-index source
  reduction retain their tested compiler identities. They explain a bounded synthetic eval gap,
  without justifying a production PTX deletion or changing receiver-copy semantics. A separate imported
  mini-LUT fixture qualifies synthetic table interpolation and compensation for 12 queries in all three modes. The registered graph remains analytic; original assets, larger
  LUT families, arbitrary inputs and application performance remain open.
- Select the next bounded workload-driven capability through the existing checked type/address
  boundaries. Array reference and result roles remain separate decisions; do not broaden generic helper
  predicates from internal value-parameter evidence. Further vendor optimization research is secondary.
