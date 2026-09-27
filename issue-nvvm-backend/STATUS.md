# NVVM current status

The maintainer authorized continued work on 2026-09-27. Continue the normal development loop through
independently reviewed local commits. Regressions or decisions requiring maintainer input stop
continuation. This supersedes the earlier stop-after-slice instruction. Skip Slack; no push or system
changes. Keep working plans, reports and raw artifacts ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records full validation of Half-vector helper bit transport.
[Accepted identity](accepted-identity.json) pins compiler/provider/modules/configuration and layout.
[Focused evidence](focused-evidence.json) preserves qualifications and failure histories under their
actual compiler identities. Older timing results have not been remeasured.

| Evidence                                          | Accepted result                                                                        |
| ------------------------------------------------- | -------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                      |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; all outcomes and inputs unchanged |
| Frozen / discovery                                | 1,356 / 384 cells                                                                      |
| Native units                                      | 1120 identities: 1107 pass, 13 skip; two new tests pass, all old identities unchanged  |
| Semantic regressions                              | 1248 identities: 1170 pass, 78 skip; unchanged                                         |
| Focused regressions                               | 62 neighbor GPU/source cells, 32 shared units, 12 new permanent GPU cells pass         |
| Direct static units                               | 5 pass, no skip; role/cache, classifier and address-plan contracts                     |
| Exported Half ABI                                 | 4 separate PTX caller cells pass; 65,536 records and 64 uint32 guards each             |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                        |
| Material runtime                                  | 12 cells pass across default/filtering profiles; 65 active and 63 guard records each   |
| Runner contracts                                  | 97 pass, 1 inherited skip                                                              |
| Last full / targeted / implementations since full | half-vector-helper-abi / half-vector-helper-abi / 0                                    |

Compiler source: `3c250bff847674a72bb95c89b19841620c920d72` plus patch
`0e2ef2f0b68008d8dad4159c4449dd19a63a378dc094b0ab1055f4be1f40cc4d`; version `2026.18.3-329-g3c250bff8`.
Loaded compiler SHA256 `a38fbec6243e37886b67c0ba803c193d0178819752a2deda93ea154aa7b67654`; provider ABI42 SHA256
`af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`. Later Git commits do not identify rebuilt bytes.
Qualification uses native Ubuntu24.04, L4 SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86,
LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`; current raw artifacts are under
`build/nvvm-half-vector-helper-abi/`. Pre-format prototype evidence keeps its separate identity. A later test-only plugin refresh
migrates a stale unit expectation; compiler, provider, modules and corpus bytes are unchanged.

## Boundaries and next action

- Checked memory/address plans remain authoritative. A transforming local-storage pass is deferred
  because the reviewed rewrite would leave existing conversion responsibilities in place. Recursive
  admission, dedicated resources and structured conversions remain explicit architectural debt.
- Half2/3/4 helper parameters/results now transport integer lane bits while values, arithmetic,
  storage and type admission retain their roles. Parameter-only and result-only tests span every
  16-bit encoding per lane in two correlated families; this is not Cartesian coverage. The original
  effectful-call failure is resolved with unchanged source/oracle and retained before failures.
  Separate callers preserve the existing direct-NVVM PTX export ABI; CUDA-prelude interoperability
  and unspecified return padding are not qualified. No provider ABI change or extra cache is needed.
- The 36 main gaps remain: three original graphics-packed column-major mismatches and 33
  infrastructure/preflight gaps. Three focused NVRTC nested integer-array failures and two large
  vendor timeout histories remain separate. Do not repack original uploads or relabel these cells.
- Internal identity-record array parameters, compact matrix layout, legal vector source updates,
  synthetic-texture material eval/sample, mini-LUT and selected dielectric eval/PDF queries retain
  their tested boundaries in the feature matrix. Full applications, arbitrary inputs, sampling
  distributions and unmeasured performance remain open.
- Surface investigation found a separate format-contract mismatch. Unannotated int4 surface kernels
  assume native 32-bit channels; RGBA8Sint allocations are actually four bytes per texel. Independent
  host readback fails all five packed-format cells and passes all five matching RGBA32Sint controls.
  Historical shader self-check passes do not qualify physical packed texels. The original NVRTC
  component-source failure also remains open. No compiler or original corpus input was changed.
- This diagnostic closeout does not replace the full baseline or reset implementation cadence.
  The next implementation awaits the maintainer's format-scope choice: explicit static-format
  conversion (recommended foundation) or separate native-width component legalization. Static format
  support would still require annotations; it would not silently fix undecorated runtime bindings.
  Generic runtime formats need their own qualified design. Keep arbitrary C RequirePrelude rejected,
  and return to a material integration requirement after the selected bounded work.
