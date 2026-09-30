# NVVM current status

The core bit/classification migration is accepted after core math. Seven bit transport/conversion
spellings and three classification families now use core casts, masks and assignments. Nine backend
recipe kinds and numeric IS_NAN 67 are retired. The preceding math batch moved thirteen public
operations to named libdevice calls and composed sincos/mad in core. Module 43, provider ABI 46 and
container format 2 remain unchanged.

**Continue under the accelerated workflow authorized on 2026-09-30.** Use eight build jobs,
larger related intrinsic batches, focused compile/PTX checks and no routine module-version bumps.
The maintainer superseded the stop-after-log request and automatic full-suite cadence. Prioritize
removal of CUDA-string recognizers. No push, Slack or system changes. Preserve the user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-core-bits/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence | Result |
| --- | --- |
| Build | Eight-job incremental build passed in 317 seconds |
| Focused units | 6 pass, including retired numeric/text rejection before output mutation |
| Shader smoke | 11 CUDA cases and 1 PTX file check pass across four small fixtures |
| Material PTX | One representative material compiled and assembled through NVVM O3 |
| Test time | About 20 seconds for units and shader smoke combined |
| Last full / targeted / implementations since full | log-family / core-bits / 2 |
| Historical evidence | Full baseline/identity unchanged; all 28 earlier feature objects preserved |

This batch had no validation failures. Exact bit transport, double word/alias order, packed Half
conversion edges and all-width IEEE classification have focused execution coverage. No full corpus,
static-version, old-module or new numerical-oracle campaign ran. The prior math batch's resolved
failures remain recorded with their original identities.

## Current tested identity

Revision `2bf556480e04b1451d328a8104f82cbbc93a543a` plus compiler patch
`5f3e4d820dafee3ba55c2c95c26b464a38f0c8eed08cf0d138484a74f97c6f70`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `690caf01988714cccd4113e78aebe714ff39fc438b6707bdb465c4907e9ddff5`.
Provider SHA-256: `0c137803f2ffd728339e9a6d563820ebc78416e53a37fe32ca6d9a6f74dc187b`.
The later commit does not relabel these binaries. Current source/runtime hashes and exact focused
outcomes live in `focused-evidence.json` under `features.nvvm-core-bits`.

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

Move abs/min/max/sign and clocks together using named calls and core expressions. Preserve Half
absolute-value bits, floating min/max selection, sign behavior and side-effecting clock observations.
Then migrate atomic producers and wave compositions through existing canonical operations. Keep
pointer-result frexp/modf and resource producer identity separate where their representation needs
explicit design. Continue larger batches without routine module-version bumps.

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
