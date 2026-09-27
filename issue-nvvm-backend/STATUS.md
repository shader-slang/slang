# NVVM current status

The maintainer authorized continued work on 2026-09-27. Continue the normal development loop through
independently reviewed local commits. Regressions or decisions requiring maintainer input stop
continuation. This supersedes the earlier stop-after-slice instruction. Skip Slack; no push or system
changes. Keep working plans, reports and raw artifacts ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records full validation of canonical multi-lane vector source emission. [Accepted identity](accepted-identity.json) pins installed compiler/provider/modules/configuration
and layout. [Focused evidence](focused-evidence.json) preserves separate qualifications and their original
identities; older timing results have not been remeasured under this compiler.

| Evidence                                          | Accepted result                                                                    |
| ------------------------------------------------- | ---------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                  |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; one reviewed transition       |
| Frozen / discovery                                | 1,356 / 384 cells                                                                  |
| Native units                                      | 1,118 identities: 1,105 pass, 13 skip; unchanged                                   |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                   |
| Focused regression                                | 54 neighbor GPU cells, 28 shared units, 8 new runtime/source tests pass            |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                    |
| Material runtime                                  | 12 cells pass across default/filtering profiles; 65 records and 63 guards per cell |
| Runner contracts                                  | 97 pass, 1 inherited skip                                                          |
| Last full / targeted / implementations since full | cuda-swizzle-set / cuda-swizzle-set / 0                                            |

Compiler source: `2b8dafc69fc78454d685a37defbb089cac5f1630` plus patch
`7101a82b9f7baa664aa967a9bc66ad1ef81fdb60c7da1419d26388d888ee4aff`; version `2026.18.3-328-g2b8dafc69`.
Loaded compiler SHA256 `415eacb9da4d41afcec36afb626a9f17d3121ea97c43761f3451366df043ebab`; provider ABI42 SHA256
`af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
Later Git commits do not identify rebuilt bytes. Qualification uses native Ubuntu24.04, L4 SM89,
driver580.126.09, CUDA12.9.2/NVRTC12.9.86 and LLVM14, targeting SM80. Installed layout is
`build/RelWithDebInfo`; current raw acceptance artifacts are under `build/nvvm-cuda-swizzle-set/`.
The two direct static tests retain their prior compiler identity; this source-emitter change does not
alter NVVM type roles or address plans. Both material profiles and the mini-LUT/dielectric fixtures
are included in fresh correctness regressions. All 576 corpus inputs and native/semantic identities
are preserved. Exactly one original NVRTC half-vector cell changes from invalid generated source to
correct output; the other 1,739 outcomes are unchanged. Older device-event measurements retain their
original compiler identity.

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
- Three original graphics-packed column-major mismatches and 33 infrastructure/preflight gaps remain
  in the main corpus. The permanent compact CUDA matrix test passes; raw uploads do not repack host
  data. Three focused NVRTC nested integer-array wrong-output controls remain outside the main corpus;
  two large vendor experiments timed out. Their histories and limits remain in focused evidence.
- The tiled-brass material's synthetic-texture eval/sample contracts pass texel-center and fixed
  off-center filtering correctness gates. Interpolation precedes color decode; both wrap seams are
  exercised by the selected footprint. Arbitrary UVs and texture quantization remain unqualified.
  Prior device-event measurements, eval local-store deletion control and dynamic-index source
  reduction retain their tested compiler identities. They explain a bounded synthetic eval gap,
  without justifying a production PTX deletion or changing receiver-copy semantics. A separate imported
  mini-LUT fixture qualifies synthetic interpolation and compensation. A separate dielectric fixture
  qualifies reflection/transmission values and PDFs for 12 selected queries, including disabled modes
  and anisotropic directions. The registered graph remains analytic and reflection-only. Sampling
  distributions, TIR/backside behavior, original assets, larger LUT families, arbitrary inputs and
  application performance remain open.
- Shared CLike emission now handles canonical multi-lane SwizzleSet with legal scalar assignments
  on CUDA/CPU/WGPU. Snapshot/runtime and five-target source checks pass; HLSL/GLSL and scalar updates
  retain their supported spelling. WGSL has source checks only.
- Next isolate the separate native Half-vector helper ABI failure: a half4 parameter/half3 result
  gives wrong output under NVVM O3 before and after the source-emitter repair. NVVM O0 and repaired
  NVRTC pass. Capture the existing LLVM handoff, separate argument/result transport, and preserve
  the original failure. Exported Half-vector functions are currently admitted, so a repair must
  explicitly establish their ABI behavior; a type-only internal mapping cannot silently preserve it.
  Continue through independently reviewed bounded work; no general helper-role expansion follows.
