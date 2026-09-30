# NVVM current status

The ceil/floor/trunc migration and full checkpoint are accepted. Their core bodies select NVVM
explicitly and call named selected-libdevice functions; Half preserves Float32 evaluation through
canonical casts. Numeric operations 42/54/57, their tags and CUDA-text recognizers are retired.
Frac keeps its separate floor-and-subtract recipe. The semantic module boundary is 37; provider ABI 46
and container format 2 are unchanged.
The maintainer's resumed migration authorization continues through reviewed local commits.
Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored.
Current accepted raw evidence: `build/nvvm-directed-rounding/`.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json), [identity](accepted-identity.json) and
[focused evidence](focused-evidence.json) record the current accepted state.
The checkpoint preserves all earlier inputs, exact outcomes and failure histories.
Performance measurements were not refreshed.

| Evidence                                          | Accepted result                                                                                                                                                                                       |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; frozen 1,356 + discovery 384 unchanged                                                                                                                                             |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories retained                                                                                                                                          |
| Native units                                      | 1,157 identities: 1,145 pass, 12 skip; all 1,152 prior statuses unchanged plus five new passes                                                                                                        |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                                                                                      |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches; exact outcomes unchanged                                                                                                      |
| Focused rounding coverage                         | 63 units, 18 GPU cells, 4 removed-tag diagnostics, 106 capability tests and 6 independent oracle checks pass                                                                                          |
| Runtime / toolkit / material                      | 4 / 18 / 6 pass; material is compile/assembly coverage                                                                                                                                                |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU contracts pass                                                                                                                                  |
| Module boundary                                   | min=max37, container format 2; 98 version-36 retirement phases, 66 old-module metadata/rejection phases, 60 version-37 successor phases, 24 named-call replay phases and 3 isolated static units pass |
| Focused features                                  | 21 total; all 20 earlier feature objects retain their exact original identities and evidence                                                                                                          |
| Last full / targeted / implementations since full | directed-rounding / directed-rounding / 0                                                                                                                                                             |
| Validation stability                              | 102 source, 2 configuration, 39 runtime hashes and 100 layout entries unchanged across final validation                                                                                               |

Compiler source: `f8fd09e5e1b2ee4d69463f8757999f6c32f35bb3` plus patch
`edde8f26c8531ecebfbe5ac934d0837a955ad426afc163e2a0c695b83998a2e7`;
version `2026.18.3-341-gf8fd09e5e`.
Loaded compiler SHA256 `4bb8277d91da4d6a93a8cba1f854e6bd74dec80af28d826213cc9826fa205e26`;
provider ABI 46 SHA256 `4c957a5a218f11ffea105c9dccbfce1c56c7a0404ae79178943e0d534327199b`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu 24.04, L4 SM89,
driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14 and target SM80 in `build/RelWithDebInfo`.
The three static units have separate executable/configuration identity and are not added to native
counts. Their measured run followed explicit core-cache preparation and preserved all capture hashes.

The selected downstream compiler owns an immutable per-compilation libdevice snapshot. The provider
queries real definitions before output creation; the same bytes reach libNVVM. Named admission covers
Float32/Float64 round, ceil, floor and trunc, with no duplicate signature map. Public Half retains
Float32 evaluation through canonical casts. The earlier round evidence preserves NVVM Half ties-away
and CUDA Half ties-even behavior. All six numerical fixtures retain their pre-execution oracle bytes.
Owned Half/Float32/Float64 ceil/floor/trunc bodies reject before the reader bump; version 37 then
rejects all 22 immutable version-36 libraries before decoding. Ten fresh successor libraries remain
callable. The initial empty baseline invocation is retained as rejected validation, resolved by the
actual nine-cell run.

## Boundaries and next action

- Start a bounded sqrt migration after the directed-rounding local commit. Its existing LLVM sqrt
  implementation makes it the next named-intrinsic candidate; first qualify scalar Float32/Float64
  admission, canonical Half promotion and an independent numerical oracle. Retire every exact,
  family, tag and text consumer only after auditing them. Other math, compound and wave recognizers
  remain separate work; remove the parenthesized semantic-tag extension after its final users migrate.
- Module version 37 is required for every backend; recompile older user and separately supplied built-ins.
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
