# NVVM backend status

The general development loop is **stopped**. Bounded [slice267](report.slice-267-receiver-snapshot.md)
is complete and accepted: internal NVVM helpers can receive only the struct fields they consume,
while preserving value snapshots and the existing helper-type contract. No push or further slice
is authorized. Read [WORKFLOW](WORKFLOW.md), [RESULTS](RESULTS.md) and [HANDOFF](HANDOFF.md).

## Accepted compiler baseline

[Validation267](runtime-validation.slice-267.json) is authoritative. It preserves every accepted262
outcome and failure history and adds the reviewed optimization evidence. The qualified compiler was
built from `d937c9f5a02fe2bc3e6fcb34ea11c6b9488f8979` plus source patch
`fe504217967549990a4310645c9903ce321f7b23b3af7f5f5af9b0d17e665635`, version
`2026.18.3-283-gd937c9f5a`. That exact source is retained by the slice's closing commit.
Provider ABI42, all 22 dependency pins and support through scalar FP8 widening260 are unchanged.

| Evidence                                          | Accepted267 result                                               |
| ------------------------------------------------- | ---------------------------------------------------------------- |
| Frozen / discovery                                | 1356 / 357 exact outcome matches                                 |
| Combined                                          | 1713 total; 1674 correct; 39 unresolved; 18 resolved histories   |
| Units / semantics                                 | 1086 pass + 13 skip / 1170 pass + 78 skip; exact test identities |
| Focused runtime and IR / standalone assembly      | 31 / 18 pass                                                     |
| Runtime / toolkit / material support              | 4 / 18 / 6 pass                                                  |
| Last full / targeted / implementations since full | 267 / 233 / 0                                                    |

All 567 prior runtime input hashes are unchanged. The three column-major semantic mismatches and
36 infrastructure/preflight gaps remain. Accepted262's concurrent NVRTC automatic-PCH incident stays
inherited history; a successful final run does not establish concurrency reliability. Two temporary
parameter-block regressions in the first267 prototype were fixed before acceptance, with that rejected
checkpoint retained. The AST subtype proof remains inherited from262, not rerun in267.

Loaded compiler library SHA256: `1cba6a5119a4058b449f70ed79a242fc13ddccff382e77132e58bdcbad8d97ed`.
Provider SHA256: `fbef1a9e22f3ac0cd42d3ffbade22470f7143930608924fc39b5bc57e40eb913`.
The unchanged `slangc` launcher hash alone does not identify the compiler. The accepted267 layout is
installed at `build/RelWithDebInfo`; accepted262 is preserved under
`build/nvvm-receiver-snapshot267/baseline-layout`.

## Material result and limits

For both unchanged material entries at NVVM O3, retained exponentials change from 6 to 0 and entry
stack from 784 to 0 bytes. Evaluation registers change 67 to 52; sampling 86 to 62. Two reversed
compile-timing rounds show 1.45–2.35% lower medians, satisfying all four declared 5% slowdown limits.
The fixed 36-cell quality subset has unchanged named resources and module sizes. No measured mode
adds spills. At NVVM O0, both material entry stacks grow by 320 bytes and modules grow; four existing
helpers remain separate in each module. This explicit tradeoff is accepted and the raw review flag
is retained. No general compiler-speed or material GPU-speed claim is made.

Material runtime remains unassessed: binding, texture/LUT, input and output contracts are unavailable.
An explicit copied constref float3 helper exposes a separate compact-storage classification failure;
it is recorded for a future authorized correctness slice, not repaired here.

The bounded [plan267](plan.slice-267-receiver-snapshot.md) is complete. Closing commit and Slack
delivery are recorded in the ignored task closeout. Raw attempts, binaries and repeated samples are
under `build/nvvm-receiver-snapshot267`. Historical263/265 presentation packages retain their original
measurements; [HISTORY](HISTORY.md) links them and the earlier research.

Environment: native Ubuntu24.04, L4 SM89, driver580.126.09, SM80 target, CUDA12.9.2,
NVRTC12.9.86, LLVM14, RelWithDebInfo. Four CPU workers maximum, two unit servers, sequential GPU
suites and isolated measurements. The next action is to await a new bounded request.
