# NVVM current status

The exp core migration and full checkpoint are accepted. The core selects `__nv_expf` or
`__nv_exp` explicitly for NVVM; Half preserves exact widening, selected Float32 evaluation and one
narrowing. Numeric operation 55, its semantic tag and CUDA-text recognizer are retired. The module
range is `min = max = 41`; provider ABI 46 and container format 2 remain unchanged.

The maintainer resumed the remaining migration on 2026-09-30 under the streamlined
[workflow](WORKFLOW.md): exp2 next, then the log/log2/log10 family. The earlier stop-after-exp
request is satisfied and superseded. No Slack, push or system changes. Plans, reports and raw
artifacts remain ignored. Current accepted raw evidence: `build/nvvm-exp/`.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. The [accepted baseline](accepted-baseline.json),
[identity](accepted-identity.json) and [focused evidence](focused-evidence.json) remain authoritative.

## Accepted state

| Evidence                                          | Accepted result                                                                                                                                           |
| ------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; all 1,356 frozen and 384 discovery outcomes unchanged                                                                                  |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories retained                                                                                              |
| Native units                                      | 1,173 identities: 1,161 pass, 12 skip; all 1,169 prior statuses unchanged plus four new passes                                                            |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                                          |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches; exact preservation                                                                |
| Focused coverage                                  | 79 units, 54 GPU cells, eight tag diagnostics, 106 capability tests                                                                                       |
| Oracles                                           | Six suites / 18 fixtures; 38 exp synthetic controls and six retained rsqrt negative controls                                                              |
| Exp numerical preservation                        | Nine buffers, with 323 / 250 / 250 words by width, unchanged per mode at versions 40 and 41                                                               |
| Runtime / toolkit / material                      | 4 / 18 / 6 pass; material remains compile/assembly coverage only                                                                                          |
| Runner contracts                                  | 119 pass, one inherited skip; 83 surface CPU contracts pass                                                                                               |
| Module boundary                                   | 64 baseline phases at version 40; 46 retirement phases; 30 old-version metadata/rejection phases; 32 fresh version 41 phases; three isolated static units |
| Focused features                                  | 25 total; all 24 prior objects retain their original evidence identities                                                                                  |
| Last full / targeted / implementations since full | exp / exp / 0                                                                                                                                             |
| Validation stability                              | 122 source, two configuration, 39 runtime and 100 layout entries; unchanged across final before/after captures                                            |

The exp corpus has 80 Half, 62 Float32 and 62 Float64 inputs, with 15 live observations per lane.
Output lengths are 323 / 250 / 250 words. Half policy IDs `55040` and `1209` identify contracts,
not module versions. Library admission uses an empirical, non-guaranteed, test-defined union of
reference-spacing and encoding-step bounds. Endpoint rules cover zero, maximum finite and infinity;
exact special-input rules override approximate admission.

NVVM Half narrows the selected Float32 `expf` result once. CUDA Half preserves its initial FMA,
`ex2` input/output FTZ, narrowing and four Half correction FMAs. All four correction inputs distinguish
the candidate sets; baseline scalar results at inputs `0x1f79` and `0x25cf` intentionally differ
between targets. Require each mode's raw bytes to remain unchanged, including unpromised NaN payloads.

Retirement at version 40 and final version 41 validation passed. All nine output buffers remain
byte-identical to each mode's baseline. Source, configuration, runtime and layout captures match.
Original preparation corrections and prior failure histories remain preserved in the evidence.

Named-library admission covers `round`, `ceil`, `floor`, `trunc`, `rsqrt` and `exp`; selected
definitions own signatures. `frac` remains named `floor` plus ordinary subtraction. The protected
user material update remains unchanged.

## Tested identity

Source revision `6ab51dcb36307a7922bcfc280a87055823041bdc` plus compiler patch
`b5996feb9b9636bfc7956d5131935b80c07b4180aa424a353adb61ef64bce897`.
Compiler version `2026.18.3-345-g6ab51dcb3`; loaded compiler SHA256
`dc1e492ac310dfb6b3d90fa43beb8398491f93bf50ad6d562ed435a359f5124b`;
provider ABI 46 SHA256 `ae85ecb92635f26d41246cbd8951f41b7ffa7aec7044802404b4acf636cab887`.
These identify the final `final41-2` validation bytes, not a later commit label. Earlier focused
features retain their original tested identities. Qualification remains native Ubuntu 24.04,
L4 SM89, driver 580.126.09, CUDA 12.9.2 / NVRTC 12.9.86, LLVM 14 and target SM80.

The isolated static executable and configuration have separate captures and remain outside native
unit counts. Their historical tests cover versions 31–38 and a dynamic future version; immutable
module probes cover exact version 40. Performance and material-runtime evidence are not refreshed.

## Boundaries and next action

- Resume with exp2 using existing preparation and validation machinery, then the logarithm family.
  Combine migration and version bump in one final compiler build, use focused acceptance where
  shared semantics are unchanged, and apply the full-checkpoint cadence in WORKFLOW. Keep build
  tuning and other investigations outside this continuation.
- Module version 41 requires older modules and separately supplied built-ins to be recompiled for
  every backend. Metadata inspection and source fallback remain available. The semantic-tag extension
  remains until its final consumers migrate; ordinary comma-separated intrinsic arguments remain.
  Timestamp-based snapshot caching is still non-atomic with external libdevice replacement.
- Preserve all 36 main gaps and prior focused NVRTC narrow-bit/nested-array failures and timeouts.
  Packed/normalized surface domains, general aliases, resource provenance, dynamic components and
  three-channel transfers remain outside current physical legalization. Checked address/memory
  plans remain authoritative; recursive admission and structured/resource conversion debt remain.
- Existing barrier helper convergence, external Half ABI, numeric sweep, material-runtime and
  performance qualifications keep their original identities. No new broad control-flow or
  performance conclusion follows from this migration.
- Repeated dispatch of bare static state in `nvvm-copyable-kernel-context` remains unresolved.
  Relinking an already-compiled requirement-free component can retain cached target output after
  option changes. CUDA `dim3 == uint3` source emission remains unsupported; retain these boundaries.
