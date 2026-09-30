# NVVM current status

The core math/layout migration has **focused local acceptance**. `frexp`/`modf` use named
libdevice calls with checked output pointers, BF16 dot is core composition of a scalar FMA,
and size/alignment use canonical IR with explicit CUDA layout. Their old text recognizers and
projection recipes are removed. **Field offset is the sole remaining CUDA-text recognizer.**
Module43, ABI46 and container2 are unchanged; the earlier signed16 abs O3 failure remains active.

The accelerated workflow authorized on 2026-09-30 remains in effect: eight build jobs, larger
related batches, focused compile/PTX/runtime checks and no routine module-version bumps or full
campaigns. The maintainer superseded the earlier stop-after-log request. This checkpoint stops at
a concrete public offset-contract decision; a question is pending. No push, Slack or system changes.
Preserve the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-core-tail/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence                                          | Result                                                                                                |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| Build                                             | Eight-job corrected production build passed in 86 seconds; final unit-tool build passed in 20 seconds |
| Focused units                                     | 10 unique passes; successful checks reused, only failures rerun                                       |
| Runtime                                           | 12 existing NVVM cases and 1 NVRTC array-layout control pass                                          |
| Diagnostics                                       | 3 initialization controls pass, including explicit address versus value assembly operands             |
| Scope                                             | Checked pointer calls, retired routes, BF16 composition and exact CUDA layout/size bounds             |
| Last full / targeted / implementations since full | log-family / core-tail / 7                                                                            |
| Historical evidence                               | Full baseline/identity unchanged; all 33 earlier feature objects preserved                            |

No full campaign, new numerical oracle, material run or performance experiment ran. Float64 modf
has provider/compiler evidence only, not fresh numerical qualification. Runtime covers Float32/64
frexp, Half scalar math, BF16 dot and selected layouts. Initial generic-cast and initialization
build failures, pointer validation/metadata-wrapper failures, fake-builder gaps and stale test
expectations are resolved and retained in `features.nvvm-core-tail`. Unsized CUDA arrays remain
16-byte pointer/count wrappers aligned to 8, not indeterminate-size arrays. Formatting after
validation changed only two C++ line wraps and did not trigger another build.

This is not an all-pass repository checkpoint or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure remains active with its exact expectation: signed16 abs(INT_MIN)
returns correct low16 bits but compares/widens as +32768. Correct LLVM and an equivalent old select
control locate the issue in downstream signed16 normalization. Do not disable it or add a per-abs
workaround. Its evidence remains in `features.nvvm-core-values-and-atomics`; it was not rerun here.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `bb8cd379340aaa6b75ffc39c92096453ffd83c39` plus compiler patch
`6cbdcf73b00a00afb3bac8fdaa825be392c9079eb283e150d5e26387909cdbbe`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `66529ca693d4e600b3f9f1367fbcbfe69dbf880280c7b2b9b77921dd2d527ef9`.
Provider SHA-256: `17ae8a44e43df07a2f329d58901eb3c54cead1d5baf238a1d02358329e3b7964`.
The later commit and whitespace-only formatting do not relabel these binaries. Source/runtime hashes,
exact outcomes, initial/intermediate identities and failure history live in `focused-evidence.json`
under `features.nvvm-core-tail`.

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

Resolve the `__offsetOf` producer contract before implementing the final migration. NVVM currently
accepts a direct field of the same base object; CUDA/CPP text also accepts address subtraction
between unrelated objects. `__offsetOf(left, left.value)` has a field-offset meaning, while
`__offsetOf(left, right.value)` does not have that same contract.

The pending choice is whether to define a same-object member offset across targets and reject
unrelated objects, or preserve CUDA/CPP address-difference behavior while retaining NVVM's narrower
contract. A canonical producer must capture the checked aggregate type and substituted field key
before SSA/value optimization. A hidden helper loses the caller's field identity; unconditional
intrinsic-op lowering would bypass a proposed target-specific fallback. Do not introduce a late
operand-graph walker, marker or new text recognizer to evade this decision. Resume the bounded
migration after the maintainer chooses the contract. Keep signed16 normalization separate.

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
