# NVVM current status

The CUDA-text route migration is **complete with focused local acceptance**. NVVM no longer
infers operations from CUDA assembly-body strings or active semantic tags. The final field-offset
recognizer is replaced by a typed query that preserves the exact field key before optimization.
Explicit LLVM/libdevice names and genuine primitive PTX remain intentional backend interfaces.
Module43, ABI46 and container2 are unchanged; the earlier signed16 abs O3 failure remains active.

The accelerated workflow authorized on 2026-09-30 remains in effect: eight build jobs, larger
related batches, focused compile/PTX/runtime checks and no routine module-version bumps or full
campaigns. The maintainer superseded the earlier stop-after-log request. The maintainer has resolved the
offset scope: preserve existing NVVM restrictions and leave CUDA/CPP behavior unchanged. The final
offset migration is accepted and the migration loop is stopped. No push, Slack or system changes.
Preserve the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-offset/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence                                          | Result                                                                                           |
| ------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| Build                                             | Eight-job production build passed in 313 seconds; focused test-tool build passed in 23 seconds   |
| Focused units                                     | 2 passes: exact layout/key preservation and wrong-base/retired-text rejection                    |
| Runtime                                           | 2 NVVM and 2 NVRTC existing layout cases pass                                                    |
| PTX smoke                                         | O0 and O3 store distinct offsets 0/4; O0 is byte-identical to the pre-migration control          |
| Scope                                             | Same-base field identity, generic specialization, dead-query removal, unchanged CUDA restoration |
| Last full / targeted / implementations since full | log-family / offset / 8                                                                          |
| Historical evidence                               | Full baseline/identity unchanged; all 34 earlier feature objects preserved                       |

No full campaign, new numerical oracle, material run or performance experiment ran. The final
offset batch had no validation failures. Production and tests were formatted before building;
documentation/evidence updates did not trigger another build. Earlier core-tail qualifications and
resolved failure histories remain in `features.nvvm-core-tail`, including Float64 modf's lack of
fresh numerical qualification. Unsized CUDA arrays remain 16-byte pointer/count wrappers aligned to 8.

This is not an all-pass repository checkpoint or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure remains active with its exact expectation: signed16 abs(INT_MIN)
returns correct low16 bits but compares/widens as +32768. Correct LLVM and an equivalent old select
control locate the issue in downstream signed16 normalization. Do not disable it or add a per-abs
workaround. Its evidence remains in `features.nvvm-core-values-and-atomics`; it was not rerun here.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `04a2fb8d0b9f8ffc25d23f4567bad32724bbbff6` plus compiler patch
`689fda7deffb9099a407a609f71f0042dbf999c983f7e20853f363c173e5d5de`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `1378468640c4b444809566aade1257d2cb77682faf1d0a120c19439e96ddc528`.
Provider SHA-256: `17ae8a44e43df07a2f329d58901eb3c54cead1d5baf238a1d02358329e3b7964`.
The later commit does not relabel these binaries. Source/runtime hashes and exact outcomes live in
`focused-evidence.json` under `features.nvvm-offset`; earlier identities and failure histories remain
in their original feature objects.

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

The agreed migration and residual-text audit are finished. Stop here. Suggested next bounded work:

1. Investigate the known signed16 O3 normalization failure at its responsible layer.
2. Select one consolidated integration checkpoint for the accumulated migrations.
3. Prioritize the remaining type/resource/ABI capability gaps from the feature matrix.

These are proposals, not automatically started campaigns. CUDA/CPP offset helper bodies remain
unchanged; their raw address-subtraction template does not establish a broader API contract for
unrelated-object calls. Stable IR915 carries the original callee/arguments and exact field key as
ordinary identity operands; no decoration-only identity or CUDA-body matching remains.

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
