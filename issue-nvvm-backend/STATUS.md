# NVVM current status

The development loop is **stopped after slice296**, as requested on 2026-09-27. The subsequent
finite documentation consolidation does not resume it. Skip Slack; no push or system changes.
Compiler refactoring, parser repair and feature work require an explicit resume. STATUS and
[WORKFLOW](WORKFLOW.md) own authority; loop/status strings inside historical evidence do not.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) is the byte-preserved full293 checkpoint, using accepted285
compiler/provider bytes. [Accepted identity](accepted-identity.json) preserves the later verified
runtime/layout/configuration/source hashes and 22 dependency pins. Documentation consolidation is
not a compiler rebuild or a fresh correctness result.

| Evidence                                          | Accepted result                                     |
| ------------------------------------------------- | --------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                   |
| Frozen / discovery cells                          | 1,356 / 384                                         |
| Main outcomes                                     | 1,703 correct; 37 unresolved; 20 resolved histories |
| Native units                                      | 1,110 identities: 1,097 pass, 13 skip               |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip               |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                     |
| Runner contracts                                  | 46 pass, 1 inherited skip                           |
| Last full / targeted / implementations since full | 293 / 233 / 0                                       |

Compiler source: `8fbf0f84ee46f4fd514010c09ff9bb9009f5e0e0` plus patch
`12f503e9802014f667dcfe2a3224a892c26a567dc7af3999888fa997ef8cc7f5`;
version `2026.18.3-301-g8fbf0f84e`. Loaded compiler SHA256
`624691257742cd8bff21c6c10b5777c50372785f2e6c21c6133128a2343b1d2c`;
provider ABI42 SHA256 `af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
A later Git HEAD does not identify these compiled bytes.

Host qualification: native Ubuntu24.04, L4 SM89/driver580.126.09, targetSM80,
CUDA12.9.2/NVRTC12.9.86 and LLVM14. Installed layout: `build/RelWithDebInfo`.
Local recovery: `build/nvvm-generic-inference286/accepted285-layout` (ignored, not portable).
Before a production rebuild, refresh cached version metadata and rebuild the restored profiling
sources; their mtimes are newer than experimental objects. See RESULTS for the build prerequisites.

## Current limits and next action

- Three column-major host-packing mismatches and 34 infrastructure/preflight gaps remain in the
  main corpus. Preserve their exact rows and resolved histories in the accepted baseline.
- Three focused NVRTC nested-array controls still return wrong37 outside the main corpus. Both
  large N65536 O3 vendor experiments exceed120seconds/4GiB. No large-GPU or speed claim follows.
- [Focused evidence](focused-evidence.json) retains 12 inheritance/initialization/constants mode
  cells and the 18 local record-array obligations. Mixed FP8/BF16 arrays remain rejected by NVVM;
  integer controls pass. Natural IR layout metadata is not emitted CUDA/provider layout proof.
- The focused corruption control exposed a parser false positive: `nvrtc-o3` in a source path is
  mistaken for a compiler diagnostic before FileCheck classification. Original failure and reviewed
  runtime-mismatch adjudication are retained. Repair is queued, not started.
- All six material PTX/cubin/resource sets preserve the accepted artifacts. Runtime binding/input/
  output contracts remain missing. The aggregate-memory gap was fixed; no material GPU-speed claim.
- The discarded inference study does not establish a precise removable CPU cost or safe pruning;
  it is archived. Do not restart that experiment merely from historical call counts.

On an explicit resume, review the queued parser issue and proposed lowering/analysis consolidation
against the current architecture. No compiler redesign or new feature priority was accepted by the
finite documentation request. Update these current artifacts in place; do not restart slice history.
