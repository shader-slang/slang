# NVVM current status

The broad core-math migration is accepted after the log family. Thirteen public operations now
select 26 named libdevice functions; `sincos` and floating/integer `mad` compose in the core.
Twelve numeric operations and thirteen tag/text routes are retired. Numeric FMOD 58 remains for
canonical `FRem`. Integer `mad` and scalar Half `sincos` are newly supported and have exact runtime
smoke coverage. Module 43, provider ABI 46 and container format 2 remain unchanged.

**Continue under the accelerated workflow authorized on 2026-09-30.** Use eight build jobs,
larger related intrinsic batches, focused compile/PTX checks and no routine module-version bumps.
The maintainer superseded the stop-after-log request and automatic full-suite cadence. Prioritize
removal of CUDA-string recognizers. No push, Slack or system changes. Preserve the user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-core-math/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence | Result |
| --- | --- |
| Build | Eight-job main build passed; later rebuilds changed only unit tests |
| Focused units | 23 pass, including 26 named signatures and 39 public width paths |
| CUDA smoke | 17 pass; 14 non-CUDA subtests explicitly excluded |
| Material PTX | One representative material compiled and assembled through NVRTC O3 and NVVM O3 |
| PTX comparison | SM80, 64-bit addresses, expected entry and live stores; no byte/numerical equivalence claim |
| Last full / targeted / implementations since full | log-family / core-math / 1 |
| Historical evidence | Full baseline/identity unchanged; all 27 earlier feature objects preserved |

The initial compile errors, fake-builder capacity/shape assumptions and intentional alias-warning
failure are retained with their resolutions. The corrected unit tests preserve direct Half
widen/call/narrow checks without assuming a fixed inlining shape. No full corpus, static-version,
old-module or new numerical-oracle campaign ran for this batch.

## Current tested identity

Revision `dc0a9acc36a74ac31912a010672a0a116f4e2883` plus compiler patch
`2b3fbdae8d3d4ef49a80822cfabb676dc5ea71a7a94b3de8fa6d280be1972226`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `fce265212542c21bd2d6d6babd7afada5c6605b2db1a840bdb56e1cf3a0f19f9`.
Provider SHA-256: `d285dd06326eabec1a35a6e32a5c004b1d7639fdff1c83003a18b19ad6e72d7b`.
The later commit does not relabel these binaries. Current source/runtime hashes and exact focused
outcomes live in `focused-evidence.json` under `features.nvvm-core-math`.

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

Move bit transport and floating classification into core expressions using existing bit casts,
shifts, masks and assignments. Reuse Half conversion/classification smoke tests. Keep pointer-result
frexp/modf, wave synchronization and resource/atomic boundaries separate where their behavior
requires different lowering. Continue larger batches, preserve numeric holes and do not bump module 43
for ordinary source migrations; stale prototype modules may need recompilation.

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
