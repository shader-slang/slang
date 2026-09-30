# NVVM current status

The selected-libdevice round migration and full checkpoint are accepted.
The implementation adds immutable selected-library validation, named round calls and canonical Half
promotion, retires numeric operation 64, and moves the semantic module boundary to 36 with provider ABI 46.
The next bounded family is ceil/floor/trunc.
The maintainer's resumed migration authorization continues through reviewed local commits.
Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored.
Active plan: `plan.libdevice-round.md`; raw evidence: `build/nvvm-libdevice-round/`.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json), [identity](accepted-identity.json) and
[focused evidence](focused-evidence.json) record the current accepted state.
The checkpoint preserves all earlier inputs, exact outcomes and failure histories.
Performance measurements were not refreshed.

| Evidence                                          | Accepted result                                                                                                                                                                     |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; frozen 1,356 + discovery 384 unchanged                                                                                                                           |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories retained                                                                                                                        |
| Native units                                      | 1,152 identities: 1,140 pass, 12 skip; all 1,140 prior statuses unchanged plus 12 new passes                                                                                        |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                                                                    |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches; exact outcomes unchanged                                                                                    |
| Focused round coverage                            | 58 units, 9 GPU cells, 1 removed-tag diagnostic, 26 named-call/assembly phases and 3 independent oracle checks pass                                                                 |
| Capability / runtime / toolkit / material         | 106 / 4 / 18 / 6 pass; material is compile/assembly coverage                                                                                                                        |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU contracts pass                                                                                                                |
| Module boundary                                   | min=max36, container format 2; 19 version 35 retirement phases, 15 old-module metadata/rejection phases, 21 version 36 successor/source-tag phases and 3 isolated static units pass |
| Focused features                                  | 20 total; all 19 earlier feature objects retain their exact original identities and evidence                                                                                        |
| Last full / targeted / implementations since full | libdevice-round / libdevice-round / 0                                                                                                                                               |
| Validation stability                              | 95 source, 2 configuration, 39 runtime hashes and 100 layout entries unchanged across final validation                                                                              |

Compiler source: `814e6145a1c0dbabdeb53df4575a385003214b6f` plus patch
`e48480a48d52f028e9f47feaba864eb516f3d9a0ff3e94e39727aa4d23d91ed1`;
version `2026.18.3-340-g814e6145a`.
Loaded compiler SHA256 `3fb5a4cf3e926d51c1a8fc4b676b39af5f3601be5d597eccc83945c00fc21f99`;
provider ABI 46 SHA256 `ec7b2c233ff580d1e90087569a434cd2acb33de60bf3e8bb8f56581e6e7a947e`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu 24.04, L4 SM89,
driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14 and target SM80 in `build/RelWithDebInfo`.
The three static units have separate executable/configuration identity and are not added to native
counts. Their measured run followed explicit core-cache preparation and preserved all capture hashes.

The selected downstream compiler now owns an immutable per-compilation libdevice snapshot. The
provider parses/verifies a separate input module and queries real definitions before output creation;
the same bytes reach libNVVM. The pilot admits `__nv_roundf`/`__nv_round`, with no duplicate signature
map. Public Half retains Float32 evaluation through canonical casts. NVVM Half preserves ties-away;
CUDA Half preserves ties-even. All three numerical fixtures retain their pre-execution oracle bytes.
Old owned Half/Float32/Float64 round bodies reject before the reader bump; module version 36 then rejects
all five immutable module version 35 libraries before decoding. Fresh successors remain callable.

## Boundaries and next action

- Create the next bounded ExecPlan for ceil/floor/trunc after this slice is locally committed.
  Preserve generic Half behavior, retire trunc's exact catalog entry as well as its family entry,
  and keep frac's distinct floor-symbol/subtract implementation. Other math, compound and wave
  recognizers remain separate work. No next-family implementation has started.
- Module version 36 is required for every backend; recompile older user and separately supplied built-ins.
  Metadata inspection and speculative source fallback remain available. The earlier timestamp-based
  cache hash is not made atomic with external libdevice replacement by the snapshot guarantee.
- Preserve all 36 main gaps and prior focused NVRTC narrow-bit/nested-array failures and timeouts.
  Packed/normalized surface domains, general aliases, resource provenance, dynamic components and
  three-channel transfers remain outside current physical legalization. Checked address/memory plans
  remain authoritative; recursive admission and structured/resource conversion debt remain.
- Existing barrier helper convergence, external Half ABI, numeric sweep, material-runtime and
  performance qualifications keep their original identities. No new broad control-flow or performance
  conclusion follows from this migration.
- Repeated dispatch of bare static state in `nvvm-copyable-kernel-context` remains unresolved.
  Relinking an already-compiled requirement-free component can retain cached target output after
  option changes. CUDA `dim3 == uint3` source emission remains unsupported; retain these boundaries.
