# NVVM current status

The open-ended development loop remains **stopped**. The bounded parser/ordinary-memory foundation
task authorized on 2026-09-27 is complete. No further compiler refactoring or feature work is running.
Skip Slack; no push or system changes. STATUS and WORKFLOW own authority; historical evidence fields
do not restart the loop.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records fresh full validation of the ordinary-memory
planning refactor and corrected NVRTC diagnostic parser. [Accepted identity](accepted-identity.json)
pins the installed layout, runtime/configuration/source hashes and dependency revisions.
[Focused evidence](focused-evidence.json) retains older qualifications with their original tested
identity; its parser resolution is explicitly a replay of preserved failure evidence.

| Evidence                                          | Accepted result                                                       |
| ------------------------------------------------- | --------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                     |
| Frozen / discovery cells                          | 1,356 / 384; all prior outcomes unchanged                             |
| Main outcomes                                     | 1,703 correct; 37 unresolved; 20 resolved histories                   |
| Native units                                      | 1,111 identities: 1,098 pass, 13 skip; all prior identities preserved |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                      |
| Focused storage regressions                       | 21 GPU mode cells and 11 units pass                                   |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                       |
| Runner contracts                                  | 48 pass, 1 inherited skip                                             |
| Last full / targeted / implementations since full | storage-boundary / storage-boundary / 0                               |

Compiler source: `128eca5ccc2a98099351f503ca155935d30c8920` plus patch
`b652422b68f7b530ee6cf5bf8eea0d983dabb2180fbdf9c0486026f36d37bc14`;
version `2026.18.3-314-g128eca5cc`. Loaded compiler SHA256
`c03484efe93f97a7ab0aa2a61b9262411dfc124f720319d44e56e70d7b4d3b70`;
provider ABI42 SHA256 `af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
A later Git HEAD does not identify these compiled bytes.

Host qualification: native Ubuntu24.04, L4 SM89/driver580.126.09, targetSM80,
CUDA12.9.2/NVRTC12.9.86 and LLVM14. Installed layout: `build/RelWithDebInfo`.
Version metadata and matching standard modules were rebuilt before validation. Raw acceptance
artifacts are under ignored `build/nvvm-storage-refactor/`; current machine records retain compact
outcomes, identities and failure history. No performance improvement is claimed.

## Current boundaries and next action

- Ordinary Var/Load/Store emission consumes checked allocation, alignment, conversion and
  provenance/ABI decisions. BF2 identity and BF3/BF4 lane-array recipes are selected before emission.
  Readonly access, physical storage and immutable-location metadata remain separate.
- This is analysis and backend-recipe planning, not a transforming Slang IR storage pass. Shared
  physical-storage lowering needs per-root selection and retained semantic-role admission before
  Generic local pointers can safely participate. Field/index analysis and structured conversion
  recursion remain explicit debt. This is the next design question on an explicit follow-up.
- Three column-major host-packing mismatches and 34 infrastructure/preflight gaps remain in the
  main corpus. Exact rows and resolved histories are preserved.
- Three focused NVRTC nested-array controls still return wrong37 outside the main corpus. Both
  large N65536 O3 vendor experiments exceed120seconds/4GiB. No large-GPU or speed claim follows.
- Older inheritance/initialization/constants and record-array qualifications retain their tested
  identity in focused evidence. Mixed FP8/BF16 arrays remain unsupported; Natural IR metadata is
  not emitted CUDA/provider layout proof.
- The parser now distinguishes NVRTC compiler identity from source paths, including the actual
  `error :` spelling. The preserved corruption log is a runtime mismatch, never a passing shader.
- All six material PTX/cubin/resource sets are unchanged. Runtime binding/input/output contracts
  remain missing. The aggregate-memory gap was fixed; no material GPU-speed claim.

Future authorized work updates current documents and evidence in place. Keep working plans,
report drafts and raw artifacts uncommitted; do not restart numbered slice history.
