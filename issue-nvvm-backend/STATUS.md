# NVVM current status

The rsqrt core migration and full checkpoint are accepted. Explicit selected
`__nv_rsqrtf`/`__nv_rsqrt` calls preserve the existing scalar library operations. Half widens to
Float32 before one narrowing. Numeric operation 65, its tag and CUDA-text recognizer are retired.
The semantic module boundary is 40; provider ABI 46 and container format 2 are unchanged.
The maintainer's resumed migration authorization continues through reviewed local commits.
Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored.
Current accepted raw evidence: `build/nvvm-rsqrt/`.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json), [identity](accepted-identity.json) and
[focused evidence](focused-evidence.json) record the current accepted state.
All prior inputs, exact outcomes and failure histories are preserved.

| Evidence                                          | Accepted result                                                                                                                                                   |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; frozen 1,356 + discovery 384 unchanged                                                                                                         |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories retained                                                                                                      |
| Native units                                      | 1,169 identities: 1,157 pass, 12 skip; all 1,165 prior statuses unchanged plus four new passes                                                                    |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                                                  |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches; exact outcomes unchanged                                                                  |
| Focused coverage                                  | 75 units, 45 GPU cells, 7 tag diagnostics, 106 capability tests, 15 oracle fixtures and 6 negative oracle controls pass                                           |
| Rsqrt numerical controls                          | All nine output buffers byte-identical to baseline at module versions 39 and 40                                                                                   |
| Runtime / toolkit / material                      | 4 / 18 / 6 pass; material is compile/assembly coverage                                                                                                            |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU contracts pass                                                                                              |
| Module boundary                                   | min=max40, container 2; 46 version 39 retirement phases, 30 old-module metadata/rejection phases, 32 version 40 successor phases and 3 isolated static units pass |
| Focused features                                  | 24 total; all 23 earlier objects retain exact original identities and evidence                                                                                    |
| Last full / targeted / implementations since full | rsqrt / rsqrt / 0                                                                                                                                                 |
| Validation stability                              | 117 source, 2 configuration, 39 runtime hashes and 100 layout entries unchanged across four final validation captures                                             |

Rational/integer oracles cover 64 unique IEEE inputs per width and 15 live observations per lane.
Float32/64 acceptance uses an explicitly test-defined union of reference-spacing and encoding-step
bounds, not a universal vendor error metric. NVVM Half retains the selected Float32 library result
before narrowing; CUDA Half retains its distinct PTX approximation envelope. All selected Half
admission sets agree, without implying universal equivalence. Raw scalar buffers, completion and
guards remain checked; NaN payloads are not promised but observed changes require review.

The original final harness rejected a relative-versus-absolute boundary-review reference before any
module workload. Its failed gates and original contracts remain preserved. The reviewed continuation
corrected the record producer and reused successful focused results with their original identities.
Four source/configuration/runtime/layout captures prove that the continuation used the same built
bytes. No compiler or oracle change was needed. Earlier preparation corrections and prior failure
histories also remain in the evidence.

Named libdevice admission covers round/ceil/floor/trunc/rsqrt at Float32/64; selected definitions own
signatures and the same immutable bytes reach libNVVM. Frac composes floor with ordinary subtraction.
Earlier features retain their original tested identities and policies. The protected user material
update remains unchanged.

## Tested identity

Source revision `b43ebe8cf8e82583d8d99c8f146ac6854d31c7e5` plus compiler patch
`062129763ad0026dcb5217e69996b87caba73c1c6b27747fff3daabaf3d4ea1b`.
Compiler version `2026.18.3-344-gb43ebe8cf`; loaded compiler SHA256
`db128742e2ad4a8fba4c145d87c23bcf032bfe75e7124ab26df0f77c4b5f1d6f`;
provider ABI 46 SHA256 `fe8442ae252639fe90641768c592b50ede017030745e1a1ca72bd6a5d60d7447`.
These are final40-1 source/runtime-before identities reused by final40-2, not a later commit label.
Qualification remains native Ubuntu 24.04, L4 SM89, driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86,
LLVM 14 and target SM80 in `build/RelWithDebInfo`. Static executable/configuration has its own
capture and is not added to native counts. After explicit core-cache preparation, scored
before/after captures preserve exact identity;
historical versions 31–38 and a dynamic future version are covered, while immutable module probes
cover exact version 39. Performance measurements were not refreshed.

## Boundaries and next action

- Investigate exp (numeric operation 55) as the next bounded candidate. Reviewed preparatory notes
  identify distinct CUDA Half corrections and NVVM selected-Float32 behavior; independently review
  and freeze numerical policy and fixtures before implementation. Other math, compound and wave
  recognizers remain separate work.
- Module version 40 requires older user modules and separately supplied built-ins to be recompiled
  for every backend. Metadata inspection and source fallback remain available. Other math/compound/
  wave recognizers remain; retire the semantic-tag extension only after its final consumers migrate,
  preserving ordinary comma-separated intrinsic-asm operands. The earlier timestamp-based cache
  hash is not atomic with external libdevice replacement.
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
