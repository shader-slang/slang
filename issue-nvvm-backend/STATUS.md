# NVVM current status

The authorized NVVM-only surface-format legalization pass is complete and accepted. This bounded
task stops here; it does not resume the general feature loop. Skip Slack; no push or system changes.
Keep plans, reports and raw artifacts ignored and update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records the current full correctness checkpoint.
[Accepted identity](accepted-identity.json) pins compiler/provider/modules/configuration and layout.
[Focused evidence](focused-evidence.json) preserves qualifications and failure histories under their
actual compiler identities. Performance measurements have not been refreshed for this compiler.

| Evidence                                          | Accepted result                                                                           |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                         |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; exact outcomes and inputs unchanged  |
| Frozen / discovery                                | 1,356 / 384 cells                                                                         |
| Native units                                      | 1125 identities: 1112 pass, 13 skip; five new tests pass, all old identities unchanged    |
| Semantic regressions                              | 1248 identities: 1170 pass, 78 skip; unchanged                                            |
| Surface host-readback matrix                      | 43 cases × 3 modes: 106 passes, 14 compile failures, 9 retained NVRTC rounding mismatches |
| NVVM surface qualification                        | 41 supported cases pass in each mode; two dynamic-index negatives remain per mode         |
| Focused units / surface source / Half GPU         | 7 / 6 / 15 pass                                                                           |
| Direct static units                               | 5 pass, no skip; role/cache, classifier and address-plan contracts                        |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                           |
| Material runtime                                  | 12 cells pass; two entries × two input profiles × three modes                             |
| Runner contracts                                  | 97 pass, 1 inherited skip; 43 surface oracle/ABI CPU case contracts pass                  |
| NVRTC surface source preservation                 | 36 emitted CUDA sources and reflection records byte-identical                             |
| Last full / targeted / implementations since full | surface-legalization / surface-legalization / 0                                           |

Compiler source: `9ebdb016d10fc68172bec515cda7e43d8477789c` plus patch
`555d6eac1de332680880084610a20da66a6d5bb1870abe1c0e554e326f65b5d8`; version `2026.18.3-334-g9ebdb016d`.
Loaded compiler SHA256 `15db5def7dd9278a71c51dfe2d009cb8a6598c0a57f246114cbf46df70749bb7`; provider ABI43 SHA256
`042479d996c7f886bd97dd1a7a06f8a56398211948f8c2eaf86bcb9727aae1a1`. Later commits do not identify rebuilt bytes.
Qualification uses native Ubuntu24.04, L4 SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86,
LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`; raw validation is under
`build/nvvm-surface-legalization/`.

The pass emits physical typed accesses, ordinary Half conversion, byte-X scaling and exact physical
component merges before emission. Matching Float32 stays Float32. Half stores deliberately use
RN-even instead of the old formatted-store truncation. Two generic conversion fixes make constant
ties/subnormal sticky bits and NVVM O3 NaNs agree with that contract. The compiled core helper matches
394,356 independent reference inputs. NVRTC surface-format handling remains unchanged; shared Half
constant folding receives the rounding correction.

## Boundaries and next action

- Packed/normalized formats, arbitrary user-helper resource provenance, dynamic component indices,
  three-channel transfers and general aliases remain outside this pass. The next format expansion
  should add explicit IR conversions with independent physical storage oracles.
- Unannotated int4 still requires matching 32-bit channels. All five native signed32 controls pass;
  all five packed signed8 mismatches and the NVRTC component compile failure remain visible. The
  original RGBA8Sint corpus input is unchanged; shader round-trip equality is not packed qualification.
- The 36 main gaps remain: three original graphics-packed column-major mismatches and 33
  infrastructure/preflight gaps. Focused NVRTC nested integer-array failures and vendor timeout
  histories remain separate. Do not repack original uploads or relabel these cells.
- Checked memory/address plans remain authoritative. A transforming local-storage pass stays
  deferred; recursive admission, dedicated resources and structured conversions remain architectural debt.
- Earlier separate-caller Half export ABI and focused qualifications keep their original tested
  identities. No fresh CUDA-prelude interoperability or arbitrary Half arithmetic claim is made.
- Compilation, original-input dispatch and code-quality measurements remain under the earlier
  compiler identity in focused evidence. Their wave and interface-dispatch optimization leads are
  separate follow-ups, with fresh measurement required after a bounded implementation.
- The shared repeated-dispatch initialization issue in `nvvm-copyable-kernel-context` remains open.
  Resolve the intended bare-static initialization contract before selecting a producer or fixture fix.
