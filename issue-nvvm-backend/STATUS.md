# NVVM current status

The maintainer authorized continued work on 2026-09-27. Address planning is accepted; the
transforming local-storage pass is deferred after independent feasibility review. The tiled-brass
synthetic-texture eval/sample contracts and bounded device-event measurements pass three-mode GPU
validation. Continue the normal development loop with a bounded workload-driven task after independent
acceptance. This supersedes the
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
outcomes, identities and failure history. No performance improvement from that refactor is claimed.

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
- Material runtime: both registered entries pass all 6 fresh GPU cells through one shared harness.
  Each cell checks 65 records and 63 untouched guards; eval checks 260 components, sample checks
  455 floats and 65 flags. Sample O0 matches the contraction candidate; NVRTC/O3 match the separate
  candidate, each consistently across its whole run. Matched sample error uses at most 0.67% of the
  frozen budget. Twenty-nine runtime and thirteen measurement CPU contracts pass. Full compiler
  evidence above remains inherited and unchanged; isolated harness work does not reset checkpoint cadence.
- Three column-major host-packing mismatches and 34 infrastructure/preflight gaps remain in the
  main corpus. Exact rows and resolved histories are preserved. A separate permanent compact CUDA
  `float3x2` fixture passes all three GPU modes (24 exact components) and three 24-byte reflection
  checks. The original graphics-authored input, discovery selection and failure rows remain intact;
  raw buffer uploads do not imply target-dependent repacking.
- Three focused NVRTC nested-array controls still return wrong37 outside the main corpus. Both
  large N65536 O3 vendor experiments exceed120seconds/4GiB. No large-GPU or speed claim follows.
- Older inheritance/initialization/constants and record-array qualifications retain their tested
  identity in focused evidence. Mixed FP8/BF16 arrays remain unsupported; Natural IR metadata is
  not emitted CUDA/provider layout proof.
- The parser now distinguishes NVRTC compiler identity from source paths, including the actual
  `error :` spelling. The preserved corruption log is a runtime mismatch, never a passing shader.
- Synthetic device-event qualification passes 6 fresh default regressions, 18 small/enlarged cells
  and 24 timing cells (216 samples, 72 warmups). All outputs are checked after every launch. At
  N=1,048,577, eval medians are 2.826–2.831 ms NVRTC O3 versus 0.314–0.315 ms NVVM O3;
  sample medians are 2.476–2.478 ms versus 0.392 ms. Reversed-round ratios are 8.98–8.99 and
  6.31–6.32. Small NVVM O3 intervals fail the predeclared 0.1 ms throughput gate and remain
  timing observations. Both complete prototype and maintained runs are retained in focused evidence.
- The registered material source is unchanged. These timings cover hot 2×2 textures and repeated
  verified inputs with correctness transfers between launches. Original assets, live LUTs, arbitrary
  inputs and application performance remain open.
- A reviewed eval-only control deletes 65 PTX stores to proven never-read local bytes, preserving
  every other PTX byte. All 9 correctness cells and 12 timing cells pass with byte-identical outputs.
  Reassembly removes the reported 592-byte stack; at N=1,048,577 NVRTC drops from 2.825–2.832 ms
  to 0.347 ms (8.14–8.16×), versus paired NVVM O3 at 0.315 ms. This deletion plus reoptimization
  accounts for most of the observed eval gap, without establishing a unique hardware bottleneck.
  No compiler/shader source changed; sample was not transformed. Keep the NVVM lowering boundary.
  Source reduction isolates bounded dynamic indexing: all 8 bounded variants retain 27 proven
  never-read stores (304 executed bytes per active thread) and a 584-byte stack; all 8 constant-index
  variants eliminate local memory. Initialization, receiver-copy and branch spelling do not change
  PTX within either group. Three full-material force-inline controls reproduce the original PTX.
  All 19 CUDA candidates compile/assemble; none has a GPU result. Two separate permanent Slang
  fixtures qualify bounded/literal indexing and copy-then-mutate semantics: all 6 mode cells and
  96 integer components pass, plus 6 neighboring GPU cells and one IR check. These focused tests
  do not change the main corpus totals or compiler checkpoint. Any production fix must preserve
  receiver value semantics and prove general field liveness; the PTX deletion remains a diagnostic.
  The compact matrix-packing positive is now permanent. Select the next coherent feature boundary
  from workload value and existing checked-plan ownership; further vendor optimization research is
  secondary to compiler capability.

Future authorized work updates current documents and evidence in place. Keep working plans,
report drafts and raw artifacts uncommitted; do not restart numbered slice history.
