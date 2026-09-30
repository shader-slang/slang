# NVVM current status

The frac core composition and full checkpoint are accepted. Its NVVM body expresses `x - floor(x)`
through selected named floor calls and ordinary subtraction. Half preserves Float32 evaluation of
the whole expression before one narrowing. Numeric operation 59, its tag and CUDA-text recognizer
are retired. The semantic module boundary is 39; provider ABI 46 and container format 2 are unchanged.
The maintainer's resumed migration authorization continues through reviewed local commits.
Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored.
Current accepted raw evidence: `build/nvvm-frac/`.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json), [identity](accepted-identity.json) and
[focused evidence](focused-evidence.json) record the current accepted state.
The checkpoint preserves all earlier inputs, exact outcomes and failure histories.
Performance measurements were not refreshed.

| Evidence                                          | Accepted result                                                                                                                                                   |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; frozen 1,356 + discovery 384 unchanged                                                                                                         |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories retained                                                                                                      |
| Native units                                      | 1,165 identities: 1,153 pass, 12 skip; all 1,162 prior statuses unchanged plus three new passes                                                                   |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                                                  |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches; exact outcomes unchanged                                                                  |
| Focused coverage                                  | 71 units, 36 GPU cells, 6 removed-tag diagnostics, 106 capability tests and 12 independent oracle checks pass                                                     |
| Frac numerical controls                           | All nine output buffers byte-identical to baseline at module versions 38 and 39                                                                                   |
| Runtime / toolkit / material                      | 4 / 18 / 6 pass; material is compile/assembly coverage                                                                                                            |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU contracts pass                                                                                              |
| Module boundary                                   | min=max39, container 2; 38 version 38 retirement phases, 30 old-module metadata/rejection phases, 24 version 39 successor phases and 3 isolated static units pass |
| Focused features                                  | 23 total; all 22 earlier objects retain exact original identities and evidence                                                                                    |
| Last full / targeted / implementations since full | frac / frac / 0                                                                                                                                                   |
| Validation stability                              | 112 source, 2 configuration, 39 runtime hashes and 100 layout entries unchanged across final validation                                                           |

Compiler source: `c92acbe2e00fe6d84f8398b9b2598275edb0121f` plus patch
`ad0a604cce981d31d91041e4e09592dbc2c7905863216598c5fce6650e97f020`;
version `2026.18.3-343-gc92acbe2e`.
Loaded compiler SHA256 `17cecbb7f09578f38752e16968a5e3ea8a4de307863e9c3478ef5a850cf4c368`;
provider ABI 46 SHA256 `6bc198ed88272be06c45e7fe4907998455dbc6782ca005dfa1fe19dd7ec353c9`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu 24.04, L4 SM89,
driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14 and target SM80 in `build/RelWithDebInfo`.
The three static units have separate executable/configuration identity and are not added to native
counts. Their measured run followed explicit core-cache preparation and preserved all capture hashes.

Frac fixtures retain their independently reviewed pre-execution bytes. Exact integer/rational
oracles cover 64 live IEEE inputs per width and 26 observations per lane across existing public
`frac` and `fract` shapes. Each 386-word output checks finite bits, positive zero, NaN classification,
raw scalar results, completion and guards. All nine buffers remain unchanged at retirement and the
final reader boundary. Finite Half equivalence follows from exact Float32 representability of the
residual, while the implementation preserves the original promoted-expression policy. Tiny negative
inputs may round to one. NaN payloads remain unpromised, but observed raw changes require review.

Owned Half/Float32/Float64 tag/text bodies reject while the version 38 reader still accepts their
container. Version 39 then rejects all ten immutable version 38 libraries before decoding, while
four fresh public/neighbor successor libraries remain callable. Reserved 59 rejects without module
mutation, and real-provider tests preserve the typed floor call and ordered subtraction in both
serializers. The initial unit's mistaken parameter-name assertion and the evidence-reference spelling
incident are resolved with their failure histories retained; neither required production or oracle changes.

Named libdevice admission remains round/ceil/floor/trunc at Float32/64; selected definitions own
signatures and the same immutable bytes reach libNVVM. Frac composes floor with ordinary subtraction.
Earlier round, directed-rounding and sqrt contracts retain their original identities and policies.
The protected user material update remains unchanged.

## Boundaries and next action

- Investigate rsqrt as the next selected-library candidate, preserving its existing scalar symbols
  and Half evaluation policy. The preparatory audit identifies distinct CUDA Half PTX and selected-library
  numerical envelopes; freeze independently reviewed oracles before implementation. Other math,
  compound and wave recognizers remain separate work. Remove the parenthesized semantic-tag extension
  after its final users migrate; preserve ordinary comma-separated intrinsic-asm operands.
- Module version 39 is required for every backend; recompile older user and separately supplied
  built-ins. Metadata inspection and speculative source fallback remain available. The earlier
  timestamp-based cache hash is not atomic with external libdevice replacement.
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
