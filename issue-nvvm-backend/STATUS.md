# NVVM current status

The wave producer migration has **focused local acceptance**: eight named wave primitives and core
raw-payload/aggregate composition replace nine tags and six numeric IDs. A genuine effectful
hardware-mask IR operation preserves the distinction from logical active-mask synthesis. Module43,
provider ABI46 and container2 remain unchanged. The earlier signed16 abs O3 failure remains active.

**Continue under the accelerated workflow authorized on 2026-09-30.** Use eight build jobs,
larger related intrinsic batches, focused compile/PTX checks and no routine module-version bumps.
The maintainer superseded the stop-after-log request and automatic full-suite cadence. Prioritize
removal of CUDA-string recognizers. No push, Slack or system changes. Preserve the user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-core-waves/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence | Result |
| --- | --- |
| Build | Eight-job production build passed in 316 seconds; successful focused test builds took 20 and 19 seconds |
| Focused units | 13 pass: ten initial results reused, three corrected assertions passed |
| CUDA smoke | 18 pass; 8 non-CUDA cases excluded |
| Scope | Named signatures/both serializers, legacy rejection, masks, raw-bit equality and aggregate transport |
| Last full / targeted / implementations since full | log-family / core-waves / 4 |
| Historical evidence | Full baseline/identity unchanged; all 30 earlier feature objects preserved |

No full campaign or redundant PTX assembly campaign ran. A test-build macro-brace error and three
stale unit assertions were corrected without changing production: named versus numeric lane reads,
the hardware-mask catalog name, and exact early retired-tag diagnostics. Successful units and GPU
runs were reused through those test-only corrections.

This is not an all-pass repository checkpoint or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure remains active with its exact expectation: signed16 abs(INT_MIN)
returns correct low16 bits but compares/widens as +32768. Correct LLVM and an equivalent old select
control locate the issue in downstream signed16 normalization. Do not disable it or add a per-abs
workaround. Its evidence remains in `features.nvvm-core-values-and-atomics`; it was not rerun here.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `44c7461777b6bba99fb86580f5c6f84f2047abfa` plus compiler patch
`bdd8324aec1f79521c30e3314d11d2404e5262079805c66da7eb3bde54540b8c`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `17128411d8d8a0ecfbc9bd78bea81a1cf4b0d69f924cbcf5dd4048444bfebebf`.
Provider SHA-256: `dcdd63edf1bbfabc675ca08a289587c6ceb7637f2a71c305d4c6a0fe4d7cb1cb`.
The later commit does not relabel these binaries. Current source/runtime hashes, exact outcomes and
failure history live in `focused-evidence.json` under `features.nvvm-core-waves`.

The [accepted baseline](accepted-baseline.json) and [accepted identity](accepted-identity.json)
still identify the earlier **full log-family checkpoint**, not these current binaries. That full
run preserved all 1,740 corpus and 249 surface outcomes: 1,704 correct main outcomes, 36 unresolved
and 21 resolved histories; surfaces 214 pass, 24 compile failures and 11 NVRTC mismatches.
It passed 1,177 native tests with 12 inherited skips and 1,170 semantic tests with 78 skips,
plus runtime 4 / toolkit 18 / material 6. Runner checks passed 119 with one skip, plus 83 surface
contracts. Log numerical preservation covered all 27 buffers. Earlier feature/static/material-runtime/performance claims
retain their original identities.

Environment: native Ubuntu 24.04, eight logical AMD EPYC CPUs, about 30 GiB RAM, L4 SM89,
UUID `GPU-7e9accb6-0e0f-7bb1-cafe-c1d02947b736`, driver 595.71.05, CUDA 12.9.2 / NVRTC 12.9.86,
LLVM 14, target SM80. No performance claim follows from the upgrade.

## Next action

Move masked reductions/scans, quad votes, rotation and scalar truthiness into core in one substantial
batch. Preserve current mask traversal, floating combination order, special-value selection and
Half seeds. Use a compact existing runtime selection and focused unit negatives; no new oracle or
full campaign. Keep pointer-result frexp/modf, resource representation and general narrow-integer
normalization as separate bounded work. Continue without routine module-version bumps.

## Retained boundaries

Module43 requires older user modules and separately supplied built-ins to be recompiled for every
backend. Metadata inspection and source fallback remain available. Remove the NVVM semantic-tag
extension only after its last consumers migrate; ordinary explicit intrinsic arguments remain.

Preserve all 36 main gaps and focused NVRTC narrow-bit/nested-array failures and timeouts.
Packed/normalized surfaces, general aliases, resource provenance, dynamic components and
three-channel transfers remain outside current physical legalization. Checked address/memory plans
remain authoritative; recursive admission and structured/resource conversion debt remain.
Barrier convergence, external Half ABI, numeric sweep, material-runtime and performance conclusions
retain prior qualifications. Snapshot caching remains non-atomic with external libdevice replacement.
Repeated bare-static-state dispatch in nvvm-copyable-kernel-context remains unresolved. Relinking a
compiled requirement-free component may retain cached target output after option changes.
CUDA `dim3 == uint3` source emission remains unsupported.
