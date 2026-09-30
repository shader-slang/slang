# NVVM current status

The sqrt migration and full checkpoint are accepted. Its core body selects NVVM explicitly and
calls named `llvm.sqrt`; Half preserves Float32 evaluation through canonical casts. Numeric
operation 36, its tag and CUDA-text recognizer are retired. LLVM's registry owns signatures and
attributes, and sqrt-only programs require no libdevice load or query. The semantic module boundary
is 38; provider ABI 46 and container format 2 are unchanged.
The maintainer's resumed migration authorization continues through reviewed local commits.
Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored.
Current accepted raw evidence: `build/nvvm-sqrt/`.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json), [identity](accepted-identity.json) and
[focused evidence](focused-evidence.json) record the current accepted state.
The checkpoint preserves all earlier inputs, exact outcomes and failure histories.
Performance measurements were not refreshed.

| Evidence                                          | Accepted result                                                                                                                                                                               |
| ------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; frozen 1,356 + discovery 384 unchanged                                                                                                                                     |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories retained                                                                                                                                  |
| Native units                                      | 1,162 identities: 1,150 pass, 12 skip; all 1,157 prior statuses unchanged plus five new passes                                                                                                |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                                                                              |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches; exact outcomes unchanged                                                                                              |
| Focused coverage                                  | 68 units, 27 GPU cells, 5 removed-tag diagnostics, 106 capability tests and 9 independent oracle checks pass                                                                                  |
| Sqrt numerical/negative controls                  | All nine output buffers byte-identical to baseline at module 37 and 38; 22 named-signature rejections at each version                                                                         |
| Runtime / toolkit / material                      | 4 / 18 / 6 pass; material is compile/assembly coverage                                                                                                                                        |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU contracts pass                                                                                                                          |
| Module boundary                                   | min=max38, container 2; 46 version 37 retirement phases, 30 old-module metadata/rejection phases, 36 version 38 successor phases, 8 named-call replay phases and 3 isolated static units pass |
| Focused features                                  | 22 total; all 21 earlier objects retain exact original identities and evidence                                                                                                                |
| Last full / targeted / implementations since full | sqrt / sqrt / 0                                                                                                                                                                               |
| Validation stability                              | 107 source, 2 configuration, 39 runtime hashes and 100 layout entries unchanged across final validation                                                                                       |

Compiler source: `617274eb86b4a6f0ed08f4633bb3e89a559e7abf` plus patch
`db5f36454541e51aa09c6bc788ad66c27d5f24297248fd539aa095a7bd70a9d7`;
version `2026.18.3-342-g617274eb8`.
Loaded compiler SHA256 `b128d8373140c8142adf2c389a8fec41cbb1e5955fd4b8c08515c44f4ab36a7e`;
provider ABI 46 SHA256 `5141f2e5c158ae5a5afd1b5be4c34f468619914e746f742d05666d0dd32fd498`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu 24.04, L4 SM89,
driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14 and target SM80 in `build/RelWithDebInfo`.
The three static units have separate executable/configuration identity and are not added to native
counts. Their measured run followed explicit core-cache preparation and preserved all capture hashes.

Sqrt fixtures retain their reviewed pre-execution bytes. Independent integer arithmetic checks
Float32/64 expectations and both Half policies: NVVM precise Float32 evaluation followed by Half
rounding, and CUDA's bounded approximate Float32 root before narrowing. Visible mode markers enforce
the intended policy. Every observed output buffer is unchanged; agreement on these inputs does not
claim universal equivalence, and NaN payloads remain unpromised. Owned Half/Float32/Float64 tag/text
bodies reject before the reader bump. Version 38 then rejects all ten immutable version 37 libraries
before decoding, while six fresh successor libraries remain callable.

Named libdevice admission remains round/ceil/floor/trunc at Float32/64; selected definitions own
signatures and the same immutable bytes reach libNVVM. Frac retains its floor/subtract recipe.
Earlier Half round tie policies and all prior focused evidence retain their original identities.
Local CPU checker/formatter preparation incidents remain recorded separately from GPU outcomes.

## Boundaries and next action

- Investigate frac next after the sqrt local commit: preserve its floor/subtract composition and
  Half rounding policy through an explicit NVVM core body, then retire its numeric/tag/text routes.
  Rsqrt follows as a selected-library candidate with its own numerical-policy audit. Other math,
  compound and wave recognizers remain separate work. Remove the parenthesized semantic-tag extension
  after its final users migrate; preserve ordinary comma-separated intrinsic-asm operands.
- Module version 38 is required for every backend; recompile older user and separately supplied
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
