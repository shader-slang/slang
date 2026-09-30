# NVVM current status

The resource migration has **focused local acceptance**: sampling, fetch, gather, base dimensions
and surface producers now emit typed operations. Five CUDA-text texture recognizers and the final
three active semantic tags are removed. Serialized AST/IR slots remain reserved. Direct 2D-array
Store now shares the existing subscript store path. Module43, ABI46 and container2 are unchanged;
the earlier signed16 abs O3 failure remains active.

**Continue under the accelerated workflow authorized on 2026-09-30.** Use eight build jobs,
larger related intrinsic batches, focused compile/PTX checks and no routine module-version bumps.
The maintainer superseded the stop-after-log request and automatic full-suite cadence. Prioritize
removal of CUDA-string recognizers. No push, Slack or system changes. Preserve the user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-core-resources/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence | Result |
| --- | --- |
| Build | Eight-job production build passed first try in 323 seconds; test build passed in 22 seconds |
| Focused units | 7 pass |
| Runtime | 7 NVVM texture cases and 4 bounded surface cells pass |
| Compile checks | 2 gather PTX checks and 6 diagnostic cases pass |
| Scope | Typed resource operands/admission, retired-route rejection, surface provenance/conversion and exact shape diagnostics |
| Last full / targeted / implementations since full | log-family / core-resources / 6 |
| Historical evidence | Full baseline/identity unchanged; all 32 earlier feature objects preserved |

No full campaign, fresh NVRTC controls or broad surface matrix ran. Canonical ImageLoad inlines the
old helper, so unsupported Texture1DArray now rejects at its collected field address rather than a
helper parameter. The one stale exact diagnostic expectation was corrected and rerun; all other
passes and production binaries were reused. Gather evidence is compile/PTX only. Surface execution
covers native float32 whole store and Half component store at O0/O3 using existing fixtures.

This is not an all-pass repository checkpoint or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure remains active with its exact expectation: signed16 abs(INT_MIN)
returns correct low16 bits but compares/widens as +32768. Correct LLVM and an equivalent old select
control locate the issue in downstream signed16 normalization. Do not disable it or add a per-abs
workaround. Its evidence remains in `features.nvvm-core-values-and-atomics`; it was not rerun here.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `4f25d04719c325883eb099fba40db913ea557d90` plus compiler patch
`31cbb06b5c504bb7c82f8034151af68f4d502c9e06632c6db795c802a182a0e0`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `9b38d177c52188f963504fc263083f99872252f766fe592d1ead45ff4a502426`.
Provider SHA-256: `a8fd827876dc5995e82b29d638e0c3e7fb83dc901b23eefd3e690bd5f52498fe`.
The later commit does not relabel these binaries. Current source/runtime hashes, exact outcomes and
failure history live in `focused-evidence.json` under `features.nvvm-core-resources`.

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

Finish the remaining text routes in one bounded batch: named frexp/modf calls with checked local
output pointers, exact BF16 dot composition through a typed scalar FMA, and CUDA layout queries.
Reuse existing SizeOf/AlignOf with explicit CUDALayout; preserve field keys for offsets. Audit actual
libdevice pointer attributes before widening named-call admission. Keep general signed16 normalization
separate. Use focused existing checks and no routine module-version bump.

## Retained boundaries

Module43 requires older user modules and separately supplied built-ins to be recompiled for every
backend. Metadata inspection and source fallback remain available. The active NVVM semantic-tag
extension is removed; inert serialized slots and ordinary explicit intrinsic arguments remain.

Preserve all 36 main gaps and focused NVRTC narrow-bit/nested-array failures and timeouts.
Packed/normalized surfaces, general aliases, resource provenance, dynamic components and
three-channel transfers remain outside current physical legalization. Checked address/memory plans
remain authoritative; recursive admission and structured/resource conversion debt remain.
Barrier convergence, external Half ABI, numeric sweep, material-runtime and performance conclusions
retain prior qualifications. Snapshot caching remains non-atomic with external libdevice replacement.
Repeated bare-static-state dispatch in nvvm-copyable-kernel-context remains unresolved. Relinking a
compiled requirement-free component may retain cached target output after option changes.
CUDA `dim3 == uint3` source emission remains unsupported.
