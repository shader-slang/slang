# NVVM backend status

The development loop is **active**, resumed on 2026-09-26. Slice270 qualifies FP8/BF16 records;
[research271](report.slice-271-language-breadth.md) adds twelve focused language passes.
[Material profile272](report.slice-272-material-profile.md) is accepted and the compiler layout is
restored exactly. [Semantic research273](report.slice-273-semantic-profile.md) qualifies16 profiles
and two recurring paths. [Research274](report.slice-274-inheritance-observation.md) finds each of6230
canonical inheritance keys computed once; accepted compiler restored exactly and local audits pass.
[Research275](report.slice-275-pch-reproduction.md) reproduces shared-directory PCH ownership failures:
8 native failures+2 crashes/16 shared processes; all24 serial/private controls pass. Next: qualify
private namespace implementation and actual adapter qualification.
[Research276](report.slice-276-pch-lifetime.md) qualifies20 lifecycle processes/100compiles: retiring
an owner directory while NVRTC remains loaded preserves other-owner reuse and final cleanup.
Continue reviewed local commits under [WORKFLOW](WORKFLOW.md); skip Slack, no push.
Read [RESULTS](RESULTS.md) and [HANDOFF](HANDOFF.md).

## Accepted baseline

[Validation270](runtime-validation.slice-270.json) preserves 1,738 outcomes and resolves the original
dynamic-dispatch test at NVVM O0/O3. [Report270](report.slice-270-fp8-aggregate.md) and the
[record contract](../docs/design/nvvm-substandard-record-contract.md) define the bounded value/local
storage domain. [Corpus269](report.slice-269-corpus-enumeration.md) and
[correction268](report.slice-268-borrowed-vector-storage.md) remain prior accepted work.

| Evidence                                          | Accepted270 result                                       |
| ------------------------------------------------- | -------------------------------------------------------- |
| Selected cases / source files / mode cells        | 580 / 576 / 1,740                                        |
| Frozen / discovery cells                          | 1,356 / 384                                              |
| Outcomes                                          | 1,703 correct;37 unresolved;20 resolved histories        |
| Units / semantics                                 | 1,087 pass+13 skip /1,170 pass+78 skip; exact identities |
| Runtime / toolkit / material compile and assembly | 4 /18 /6 pass                                            |
| Last full / targeted / implementations since full | 270 /233 /0                                              |

All576 main input hashes and 22 dependency pins are unchanged. Discovery remains at 128sources.
Main selection covers576/1,852 compute-comparison files and517/795 explicit CUDA files; these are
file-selection ratios, not semantic coverage. The new270 fixture remains outside the main corpora,
as do the two268 fixtures. Original review-required comparisons and resolved failures are preserved.

Qualified source: `06a26a3f7af603de518cfe82c981d527b580e451` plus compiler patch
`a35e26dca958361bc80afb5aa540708ebad7492213fd91d1552957a4150f17e1`, version
`2026.18.3-286-g06a26a3f7`. Loaded compiler SHA256:
`74bb18f34a900790d7af2b3edf9567c69c8bedd8a347c45d0e08c8adb193667f`.
Provider ABI42 unchanged: `fbef1a9e22f3ac0cd42d3ffbade22470f7143930608924fc39b5bc57e40eb913`.
The launcher hash alone is not compiler identity. The final focused fixture refinement is separately
qualified; full-gate and final-focused input identities are explicit in validation270.

The qualified layout is `build/RelWithDebInfo`, including four269 numerics modules. The accepted269
layout is preserved under `build/nvvm-fp8-aggregate270/accepted269-layout`; [RESULTS](RESULTS.md)
documents build/module prerequisites. All270 raw evidence is under `build/nvvm-fp8-aggregate270`.

## Results and limits

Three column-major host-packing mismatches and 34 infrastructure/preflight gaps remain. Concurrent
NVRTC automatic-PCH reliability remains open. Material GPU runtime lacks binding, texture/LUT and
input/output contracts. Generic record-pointer result exclusions are code-reviewed only. Opposite
Value/Storage cache orders are executed for mixed Payload; AlignedPair is storage-first.
Fresh272 instrumented wall medians: NVRTC/NVVM O3 evaluation1395.34/1362.80ms and
sampling1424.52/1454.46ms. Semantic checking costs385–397ms; target preparation/emission about3%.
These are within-session compile observations, not causal cross-session or GPU-speed claims.
Earlier262 AST proof and267 timing retain original identities;272 confirms267 static O3 resources. [HISTORY](HISTORY.md) links prior evidence and presentation packages.

Environment: native Ubuntu24.04, L4 SM89/driver580.126.09, SM80 target, CUDA12.9.2,
NVRTC12.9.86, LLVM14, RelWithDebInfo; max four CPU workers, two unit servers, serial GPU suites.
