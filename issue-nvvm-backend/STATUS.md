# NVVM backend status

The development loop is **active**, resumed on 2026-09-26. Continue bounded reviewed local commits
under [WORKFLOW](WORKFLOW.md); skip Slack, no push or system changes.
[Slice279](report.slice-279-nested-records.md) qualifies nested internal FP8/BF16 records and corrects
libNVVM's loss of padding in whole nested stores, also reproduced on previously supported integer
records. Focused14 native/18 GPU cells pass. Reused independent source/material review finds no
blockers. Read [HANDOFF](HANDOFF.md) and [RESULTS](RESULTS.md); history belongs in [HISTORY](HISTORY.md).

## Accepted baseline

[Validation279](runtime-validation.slice-279.json) freshly preserves every accepted277 corpus outcome.
[Record contract](../docs/design/nvvm-substandard-record-contract.md) owns the qualified language domain.

| Evidence                                          | Accepted279 result                                  |
| ------------------------------------------------- | --------------------------------------------------- |
| Selected cases / source files / mode cells        | 580 / 576 / 1,740                                   |
| Frozen / discovery cells                          | 1,356 / 384                                         |
| Outcomes                                          | 1,703 correct; 37 unresolved; 20 resolved histories |
| Units / semantics                                 | 1,096 pass + 13 skip / 1,170 pass + 78 skip         |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                     |
| Last full / targeted / implementations since full | 279 / 233 / 0                                       |

All576 main input hashes and22 dependency pins are unchanged. All1103 prior unit identities and1248
semantic identities are preserved; six new units pass. Two exhaustive279 GPU fixtures remain outside
the main corpus, as do focused268/270 fixtures and breadth271/278 probes. Discovery stays at128 sources.
[Corpus269](report.slice-269-corpus-enumeration.md) retains its inventory snapshot and coverage limits.

Qualified source: `0043e8d17dc1a49870c7eeb652dbfad21562378f` plus compiler/provider/test patch
`62ae64735b12f5777c4c2789dd0beef44d5835480c6b3b6af0a1eb0b31208f64`, version
`2026.18.3-295-g0043e8d17`. Loaded compiler SHA256:
`9e013b2c27f8f467af781a54177138dcdda1c15e0c09cc557d428c21b72065ba`.
Provider ABI42, SHA256 `a861b242b2eb730448a5c1afa3780eed9804412edb55574675ccaa6ebfcb428d`.
The launcher hash alone is not compiler identity. Qualified layout is `build/RelWithDebInfo`; all37
runtime identities are retained in validation279. Verified277 recovery is under
`build/nvvm-nested-records279/accepted277-layout`; raw279 evidence is beside it.

## Results, limits and next action

Three column-major host-packing mismatches and34 infrastructure/preflight gaps remain unchanged.
Nested support retains local/value roles; arrays, readonly/device/shared/exported signatures and
BF3/BF4 whole values gain no new support. The provider splits direct nested structs; array subtrees
remain opaque. Both nested cache visitation orders and packed structs are not newly qualified;
synthetic Generic pointer-result exclusion remains source-reviewed.

Material NVRTC/O3 PTX and cubins are byte-identical277. TwoO0 artifacts change through field-store
expansion; independent review accepts those changes with all parsed resources equal. Material GPU
binding/input/output contracts remain unavailable; no runtime or performance claim. PCH277 ownership
and research278 language evidence retain their original identities and limits.

[Research280](report.slice-280-array-record-stores.md) finds a preexisting padding-loss bug in whole
arrays of nested integer records: seven of nine focused cells pass; NVRTC O3 and NVVM O3 fail the
nested-array case. A separate fresh-source out-copy diagnostic fails both optimized paths at the same
child-field offset; NVVM O0 passes. Accepted277 reproduces the defect. These failures remain outside
the main corpus and explicitly open; accepted279 compiler/corpus identities are unchanged.

[Prototype281](report.slice-281-array-copy-prototypes.md) finds three passing small-copy remedies:
pointer memcpy, pointer field loops (both backends), and a typed NVVM noinline/optnone store helper.
Pointer IR stays compact at65536elements, but local storage grows; large modules were never launched.
All23attempts (six declaration failures retained) yield17effective cells:9correctGPU,2failing controls,
6compile-only. No production change; full279/targeted233/cadence0 remains inherited.

[Gate282](report.slice-282-ssa-copy-boundary.md) qualifies the small helper for constructed/phi/earlier-load
values and realalignment1/canaries, but rejects it as a general correction: N17caller ABI expands,
N65536O0 times out at120seconds under4GiB, and O3 is explicitly not run after that size-gate failure.
No arbitrary cutoff or fallback is selected; accepted279 remains unchanged.

Next: isolate whether conservative whole-store align1 annotations fix actually aligned roots without
new helper ABI. The passing ordinary underaligned path is a lead, not proof of this counterfactual.
Pointer methods still require correct snapshot materialization; NVRTC's production defect stays open.
Native Ubuntu24.04, L4SM89/driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14;
max4CPU workers,2unit servers, serialized suites. Accepted279 local commit46e58db06.
