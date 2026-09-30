# NVVM current status

The masked-wave migration has **focused local acceptance**: twenty-one reduction/prefix modes,
quad votes, rotation and scalar truthiness now use typed core bodies. Six numeric IDs and obsolete
wave/truthiness reconstruction paths are removed. Module43, provider ABI46 and container2 remain
unchanged. The earlier signed16 abs O3 failure remains active.

**Continue under the accelerated workflow authorized on 2026-09-30.** Use eight build jobs,
larger related intrinsic batches, focused compile/PTX checks and no routine module-version bumps.
The maintainer superseded the stop-after-log request and automatic full-suite cadence. Prioritize
removal of CUDA-string recognizers. No push, Slack or system changes. Preserve the user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-masked-waves/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence | Result |
| --- | --- |
| Build | Eight-job production retry passed in 83 seconds after capability declarations were corrected; focused test builds took 19–23 seconds |
| Focused units | 10 pass: seven initial and two retry passes reused; final singleton passed |
| CUDA smoke | 22 NVVM pass, one NVRTC control pass; one Vulkan case excluded |
| Scope | Typed folds, legacy rejection, mask ordering, exact seeds/singletons, quad votes, rotation and scalar truthiness |
| Last full / targeted / implementations since full | log-family / masked-waves / 5 |
| Historical evidence | Full baseline/identity unchanged; all 31 earlier feature objects preserved |

No full campaign or redundant PTX assembly campaign ran. The first production build exposed missing
CUDA-parent labels and a private SM7 requirement; these declarations were corrected without widening
public admission. Unit test doubles needed larger block/phi capacities for core CFGs (162 blocks and
41 phis observed). Three old diagnostic labels and a direct-literal assertion were updated to match
the canonical representation. Exact runtime semantics remain covered. Disabled directives shifted
two prefix selectors; only the missing NVVM cells were then run. All incidents and successful-result
reuse are retained in the focused record.

This is not an all-pass repository checkpoint or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure remains active with its exact expectation: signed16 abs(INT_MIN)
returns correct low16 bits but compares/widens as +32768. Correct LLVM and an equivalent old select
control locate the issue in downstream signed16 normalization. Do not disable it or add a per-abs
workaround. Its evidence remains in `features.nvvm-core-values-and-atomics`; it was not rerun here.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `eb587784f56907d2fca32fb2d8fa3c930d1b1faa` plus compiler patch
`7005f8bf609c95a8fc37fa5ee50e7a53be0c14adf21895adc024352492d7c59f`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `7a09f238bd7e4499e7571c64499b43fe774acdf687c4f8903e0e05373c87101f`.
Provider SHA-256: `a8fd827876dc5995e82b29d638e0c3e7fb83dc901b23eefd3e690bd5f52498fe`.
The later commit does not relabel these binaries. Current source/runtime hashes, exact outcomes and
failure history live in `focused-evidence.json` under `features.nvvm-masked-waves`.

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

Migrate resource producers together: reuse canonical sampling and image load/store IR, add typed
operations for explicit level/fetch/gather/base-size queries, and retain existing physical surface
legalization. Remove the last active semantic-tag routes while preserving inert serialized slots.
Use focused resource units and a small existing texture/surface execution selection. Keep pointer-result
frexp/modf, BF16 dot, CUDA layout queries and general narrow-integer normalization as separate work.
Continue without routine module-version bumps.

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
