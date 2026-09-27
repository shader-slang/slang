# NVVM backend status

The development loop is **active**, resumed on 2026-09-26. Continue bounded reviewed local commits
under [WORKFLOW](WORKFLOW.md); skip Slack, no push or system changes.
[Slice285](report.slice-285-nested-array-stores.md) corrects padding loss in whole stores of arrays
containing nested integer records. Root, wrapped and multidimensional regressions pass at NVVM O0/O3;
NVRTC's three focused wrong-output controls remain open. Read [HANDOFF](HANDOFF.md) and [RESULTS](RESULTS.md).
Historical evidence belongs in [HISTORY](HISTORY.md).

## Accepted baseline

[Validation285](runtime-validation.slice-285.json) freshly preserves every accepted279 main outcome.
[Record contract](../docs/design/nvvm-substandard-record-contract.md) owns the qualified language domain.

| Evidence                                          | Accepted285 result                                  |
| ------------------------------------------------- | --------------------------------------------------- |
| Selected cases / source files / mode cells        | 580 / 576 / 1,740                                   |
| Frozen / discovery cells                          | 1,356 / 384                                         |
| Outcomes                                          | 1,703 correct; 37 unresolved; 20 resolved histories |
| Units / semantics                                 | 1,097 pass + 13 skip / 1,170 pass + 78 skip         |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                     |
| Last full / targeted / implementations since full | 285 / 233 / 0                                       |

All576 main input hashes and22 dependency pins are unchanged. All1109 prior unit identities and1248
semantic identities are preserved; one new unit passes39 shape/alignment cases. Focused6units,
24nativeGPU,6flat-neighbor cells and2direct-vendor promotion cells pass. Three new GPU fixtures remain
outside the main corpus, alongside earlier focused probes. Discovery stays at128 sources.
[Corpus269](report.slice-269-corpus-enumeration.md) retains its inventory snapshot and coverage limits.

Qualified source: `8fbf0f84e` plus compiler/provider/test patch
`12f503e9802014f667dcfe2a3224a892c26a567dc7af3999888fa997ef8cc7f5`, version
`2026.18.3-301-g8fbf0f84e`. Loaded compiler SHA256:
`624691257742cd8bff21c6c10b5777c50372785f2e6c21c6133128a2343b1d2c`.
Provider ABI42, SHA256 `af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`.
The launcher hash alone is not compiler identity. Qualified layout is `build/RelWithDebInfo`; all37
runtime identities are retained in validation285. Verified279 recovery is under
`build/nvvm-nested-array-stores285/accepted279-layout`; raw285 evidence is beside it.

## Results, limits and next action

Three column-major host-packing mismatches and34 infrastructure/preflight gaps remain unchanged.
Provider direct-struct splitting remains; terminal arrays containing nested struct boundaries use a
truthful alignment1 guarantee. Canonical values, allocation/load alignment, authored LLVM signatures
and ABI42 are preserved. No new FP8/BF16 record-array admission or device/shared/readonly/exported roles.
General packed/address-space and nested cache-order qualification remain limited by the contract.

All six material PTX/cubin pairs and parsed resources are byte-identical279. Material GPU binding,
texture/LUT/input and expected-output contracts remain unavailable; no runtime or performance claim.
Earlier PCH277 and language-breadth evidence retain their original identities and limits.

Research280–284 isolated the array defect and rejected expanding typed copy helpers. Slice285 closes
its bounded NVVM correction with independent constructed/snapshot/phi/unaligned and actual Slang
oracles. NVRTC optimized copies still return wrong37 in all three new fixtures. Original and candidate
N65536 O3 modules both exceed120seconds/4GiB; no identical-cause or general scalability claim.
First nine285 fixture syntax failures and five dependent unrun cells remain recorded.

[Material286](report.slice-286-generic-inference-observation.md) observes7,995 generic inferences per
run with21 strict extra events. All eight main PTX outputs and six PTX/cubin/resource controls equal285;
all1110unit/1248semantic identities are preserved. Broader overload screening is a count-based lead,
not a safe optimization or CPU-cost result. The temporary observer is removed; installed285 layout,
source/configuration/inputs/pins are exact. Recovery and experimental bytes remain under
`build/nvvm-generic-inference286`. Before a production rebuild, refresh version metadata and rebuild
the restored observer source files; their mtimes are newer than experimental objects.

Next: bounded nested FP8/BF16 dynamic-dispatch qualification, connecting flat270 AnyValue transport
with nested279 local/value records. Require actual dynamic packing/unpacking and independent raw-bit
oracles; preserve Natural payload packing separately from CUDA local layout. No new array/device/
readonly/exported roles. The material screening lead remains documented for later producer work.
Native Ubuntu24.04, L4SM89/driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14;
max4CPU workers,2unit servers, serialized suites. Independent reviews accept285 implementation and286 restored research with no findings.
