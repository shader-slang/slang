# NVVM backend status

The development loop is **active**, resumed on 2026-09-26. Continue bounded reviewed local commits
under [WORKFLOW](WORKFLOW.md); skip Slack, no push or system changes.
[Slice277](report.slice-277-pch-ownership.md) fixes default automatic-PCH ownership: each NVRTC
compiler gets one private directory, preserving same-owner reuse and explicit caller options.
The fixed40-process comparison now passes every cell, including16 shared-cwd processes that formerly
produced8 native failures and2 crashes. Next: probe existing accessor/generic language interactions
outside the main corpus, preserving their original output oracles. Read [HANDOFF](HANDOFF.md) and
[RESULTS](RESULTS.md); historical research and closed leads belong in [HISTORY](HISTORY.md).

## Accepted baseline

[Validation277](runtime-validation.slice-277.json) freshly preserves every accepted270 corpus outcome.
[Record contract270](../docs/design/nvvm-substandard-record-contract.md),
[corpus269](report.slice-269-corpus-enumeration.md) and
[borrowed-vector correction268](report.slice-268-borrowed-vector-storage.md) retain their feature scopes.

| Evidence                                          | Accepted277 result                                  |
| ------------------------------------------------- | --------------------------------------------------- |
| Selected cases / source files / mode cells        | 580 / 576 / 1,740                                   |
| Frozen / discovery cells                          | 1,356 / 384                                         |
| Outcomes                                          | 1,703 correct; 37 unresolved; 20 resolved histories |
| Units / semantics                                 | 1,090 pass + 13 skip / 1,170 pass + 78 skip         |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                     |
| Last full / targeted / implementations since full | 277 / 233 / 0                                       |

All576 main input hashes and22 dependency pins are unchanged. The three new units pass; all1100 prior
unit identities and1248 semantic identities are preserved. Material6 PTX, cubins and reported resources
match270 exactly. Discovery stays at128 sources. Main selection covers576/1852 compute-comparison
files and517/795 explicit CUDA files; these are file-selection ratios, not semantic coverage.
Focused268/270 fixtures and twelve breadth271 cells remain outside the main corpus.

Qualified source: `9210ef5a1d18b5051a81a58311166166504e02d5` plus compiler/test patch
`30e611080fb76fe95a6dfc60f9bc1ac3a01121cfdb088fbbcc2ab856cc4ed8c4`, version
`2026.18.3-293-g9210ef5a1`. Loaded compiler SHA256:
`aa1fe42eda3a4ee6bacdf7a6408493971f3e2a882b7f7e6fd5fedabc986644d5`.
Provider ABI42 unchanged: `fbef1a9e22f3ac0cd42d3ffbade22470f7143930608924fc39b5bc57e40eb913`.
The launcher hash alone is not compiler identity. Qualified layout is `build/RelWithDebInfo`, including
four269 numerics modules; actual37 runtime identities are retained in validation277. Previous270 layout
is preserved at `build/nvvm-pch-ownership277/accepted270-layout`; raw277 evidence is beside it.

## Results and limits

Three column-major host-packing mismatches and34 infrastructure/preflight gaps remain. The demonstrated
shared-default-PCH ownership failure is corrected on installed NVRTC12.9.86. Original262's exact deletion
signature was not separately reproduced; its incident history remains. Explicit caller-shared directories
and concurrent calls through one adapter have no new reliability guarantee. Windows acquisition and
default directory-acquisition error reporting are source-audited only. Separate compiler objects now
have independent default caches; no compile-speed claim is made.

Material GPU runtime still lacks binding, texture/LUT and input/output contracts. Earlier272 compile
profiles and267 timing/quality keep their original identities; the inheritance-cache lead was closed
by274 without an optimization. Thread limits prevented fresh worker/reviewer delegation for277;
root performed separate local audits plus author self-review, without claiming independent review.

Environment: native Ubuntu24.04, L4 SM89/driver580.126.09, SM80 target, CUDA12.9.2,
NVRTC12.9.86, LLVM14, RelWithDebInfo; max four CPU workers, two unit servers, serialized GPU suites.
