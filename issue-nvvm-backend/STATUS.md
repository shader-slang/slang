# NVVM backend status

The finite float3 correction and corpus follow-up are complete. The general development loop remains
**stopped**. Both slices are local commits; the user requested skipping both Slack notifications.
Read [WORKFLOW](WORKFLOW.md), [RESULTS](RESULTS.md) and [HANDOFF](HANDOFF.md). No further slice is active.

## Accepted baseline

[Validation269](runtime-validation.slice-269.json) preserves all 1,713 accepted268 outcomes and adds
27 passing cells. [Corpus269](report.slice-269-corpus-enumeration.md) repairs directive enumeration
and adds five CUDA backfills plus four language cases. [Correction268](report.slice-268-borrowed-vector-storage.md)
preserves native borrowed float3 storage, with six focused GPU and eight assembly cells passing.
The two268 regression sources remain focused fixtures outside the main corpora.

| Evidence                                          | Accepted269 result                                            |
| ------------------------------------------------- | ------------------------------------------------------------- |
| Selected cases / source files / mode cells        | 580 / 576 / 1,740                                             |
| Frozen / discovery cells                          | 1,356 / 384                                                   |
| Outcomes                                          | 1,701 correct; 39 unresolved; 18 resolved histories           |
| Units / semantics                                 | 1,086 pass + 13 skip / 1,170 pass + 78 skip; exact identities |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                               |
| Last full / targeted / implementations since full | 269 / 233 / 0                                                 |

All 567 prior runtime input hashes remain unchanged; nine new source hashes are recorded explicitly.
The frozen inventory and all old selection/oracle contracts are unchanged. Discovery now reaches its
128-source cap. Main selection covers 576/1,852 compute-comparison files and 517/795 explicit CUDA
files; these are file-selection ratios, not semantic coverage. The original additions-only
review-required comparison is preserved alongside the reviewed acceptance.

Compiler/provider bytes are inherited268. Qualified source: `0aff56e26405e08b47971a6c6a46bfc81fca8fd5`
plus patch `dfb3b91cfd3d36af94c884a5ca08c5c087d5c9576d39d11b4868dac953140883`, version
`2026.18.3-284-g0aff56e26`. Loaded compiler SHA256:
`07881e5f945ee7028e1bc913ec127405422cd014ddc624b13a4d308f8670b822`.
Provider ABI42 SHA256: `fbef1a9e22f3ac0cd42d3ffbade22470f7143930608924fc39b5bc57e40eb913`.
All 22 dependency pins are unchanged. The launcher hash alone is not compiler identity.

The qualified layout is `build/RelWithDebInfo`. Slice269 supplies four missing standard numerics
modules, built from its existing graph/bootstrap/core; no preexisting installed artifact changes.
[RESULTS](RESULTS.md) documents this prerequisite. The original268 layout is preserved under
`build/nvvm-corpus269/numerics-prerequisite/accepted268-layout`.

## Results and limits

Three column-major semantic mismatches and 36 infrastructure/preflight gaps remain. The inherited262
concurrent NVRTC automatic-PCH incident remains open. Material GPU runtime still lacks its binding,
texture/LUT/input/output contracts. AST subtype proof262 and optimization267 timing/quality retain
their original identities; neither was remeasured here. No new performance claim is made.

Raw evidence and commit/notification closeouts are under `build/nvvm-borrowed-vector268` and
`build/nvvm-corpus269`. [HISTORY](HISTORY.md) links earlier evidence and presentation packages.
Environment: native Ubuntu24.04, L4 SM89/driver580.126.09, SM80 target, CUDA12.9.2,
NVRTC12.9.86, LLVM14, RelWithDebInfo; max four CPU workers, two unit servers, serial GPU suites.
