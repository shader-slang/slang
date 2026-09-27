# NVVM current status

The maintainer authorized continued work on 2026-09-27. Address planning is accepted; the
transforming local-storage pass is deferred after independent feasibility review. The tiled-brass
synthetic-texture eval contract now passes real three-mode GPU validation. Continue the normal
development loop with a bounded workload-driven task after independent acceptance. This supersedes the
earlier stop-after-slice instruction. Commit reviewed tasks separately; regressions or decisions
needing maintainer input stop continuation. Skip Slack; no push or system changes.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records fresh full validation of the field/index address
planning refactor. [Accepted identity](accepted-identity.json)
pins the installed layout, runtime/configuration/source hashes and dependency revisions.
[Focused evidence](focused-evidence.json) retains older qualifications with their original tested
identity; its parser resolution is explicitly a replay of preserved failure evidence.

| Evidence                                          | Accepted result                                                       |
| ------------------------------------------------- | --------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                     |
| Frozen / discovery cells                          | 1,356 / 384; all prior outcomes unchanged                             |
| Main outcomes                                     | 1,703 correct; 37 unresolved; 20 resolved histories                   |
| Native units                                      | 1,112 identities: 1,099 pass, 13 skip; all prior identities preserved |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                      |
| Focused storage regressions                       | 24 GPU mode cells and 18 units pass                                   |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                       |
| Runner contracts                                  | 48 pass, 1 inherited skip                                             |
| Last full / targeted / implementations since full | address-boundary / address-boundary / 0                               |

Compiler source: `2f33cbe5538c02482f19ee6c95fafba6f639b0b8` plus patch
`9025401d0c3bf9e0b1644d728f0ac60d9dd5929b3fd48104f61044d6419b6b56`;
version `2026.18.3-315-g2f33cbe55`. Loaded compiler SHA256
`520c123df451c326537fe94ffad532bb14d887c468bc22362452c6e6db65d289`;
provider ABI42 SHA256 `af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
A later Git HEAD does not identify these compiled bytes.

Host qualification: native Ubuntu24.04, L4 SM89/driver580.126.09, targetSM80,
CUDA12.9.2/NVRTC12.9.86 and LLVM14. Installed layout: `build/RelWithDebInfo`.
Version metadata and matching standard modules were rebuilt before validation. Raw acceptance
artifacts are under ignored `build/nvvm-address-refactor/`; current machine records retain compact
outcomes, identities and failure history. No performance improvement is claimed.

## Current boundaries and next action

- Ordinary Var/Load/Store emission consumes checked allocation, alignment, conversion and
  provenance/ABI decisions. BF2 identity and BF3/BF4 lane-array recipes are selected before emission.
  Readonly access, physical storage and immutable-location metadata remain separate.
- Field/index address recipes now feed pointer validation, ordinary memory planning and emission
  through one source-keyed index. Canonical IR still owns pointer access/address-space identity.
  Initial ancestor admission remains recursive; dedicated resource operations, field-value extraction,
  helper-signature classification and structured conversion remain explicit debt.
- A transforming local-storage pass is deferred: one-record root selection and specialization
  would add original-admission and conversion-cleanup machinery while leaving the existing backend
  BF3/BF4 conversion path necessary. This is a reviewed source-level decision, not a failed executable
  prototype. Revisit when a workload can justify retiring a complete representation family.
- Material eval: 3 fresh GPU cells pass, each with 65 active records / 260 independently checked
  components and 63 untouched guard records. Seed repeats and wrapped UVs agree exactly; maximum
  relative error is 5.09e-7. Twelve new harness CPU contracts pass. Full compiler evidence above
  remains inherited and unchanged; this isolated harness does not reset checkpoint cadence.
- Three column-major host-packing mismatches and 34 infrastructure/preflight gaps remain in the
  main corpus. Exact rows and resolved histories are preserved.
- Three focused NVRTC nested-array controls still return wrong37 outside the main corpus. Both
  large N65536 O3 vendor experiments exceed120seconds/4GiB. No large-GPU or speed claim follows.
- Older inheritance/initialization/constants and record-array qualifications retain their tested
  identity in focused evidence. Mixed FP8/BF16 arrays remain unsupported; Natural IR metadata is
  not emitted CUDA/provider layout proof.
- The parser now distinguishes NVRTC compiler identity from source paths, including the actual
  `error :` spelling. The preserved corruption log is a runtime mismatch, never a passing shader.
- The registered material source is unchanged. Synthetic-texture `eval_buffer` runtime is qualified;
  `sample_buffer`, original assets, live LUT reads and performance remain open. Compile/assembly
  evidence for both entries remains separate. Next select a bounded missing runtime contract or
  a correctness/infrastructure issue with an independent oracle.

Future authorized work updates current documents and evidence in place. Keep working plans,
report drafts and raw artifacts uncommitted; do not restart numbered slice history.
