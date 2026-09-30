# NVVM current status

The combined values/clock/atomic migration has **qualified local acceptance with one inherited
failure**. Core abs/min/max/sign, two named clocks, nine atomic reduction families and two ByteAddress
helpers replace their CUDA-text/tag paths. Four numeric IDs and thirteen semantic tags are retired;
wave MIN43/MAX44 remain. Unsigned abs, integer sign, shared integer reductions and integer64 reduction
increment/decrement gain canonical paths. Module 43, provider ABI 46 and container 2 remain unchanged.

**Continue under the accelerated workflow authorized on 2026-09-30.** Use eight build jobs,
larger related intrinsic batches, focused compile/PTX checks and no routine module-version bumps.
The maintainer superseded the stop-after-log request and automatic full-suite cadence. Prioritize
removal of CUDA-string recognizers. No push, Slack or system changes. Preserve the user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-core-values/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence | Result |
| --- | --- |
| Build | Eight-job production build passed in 301 seconds; test builds took 23 and 19 seconds |
| Focused units | 13 pass: ten initial results reused, three corrected/new checks passed |
| CUDA smoke | 22 pass, 1 inherited downstream failure; 18 non-CUDA cases excluded |
| Failure | Signed16 abs(INT_MIN) at O3; exact active test remains failing |
| Diagnostic | Correct i16 LLVM; equivalent old select control also fails; SASS lacks signed16 normalization after IABS |
| Last full / targeted / implementations since full | log-family / core-values-and-atomics / 3 |
| Historical evidence | Full baseline/identity unchanged; all 29 earlier feature objects preserved |

This is not an all-pass checkpoint or publication/CI readiness. The failure returns correct low16
bits but compares/widens the value as +32768 rather than -32768 at O3; O0 passes. General narrow-integer
lowering remains separate from producer migration. Do not disable the test, change its expectation,
or add a special-case abs workaround to claim success. Two stale unit expectations were corrected:
integer/Half abs needs no fabsf call, and local atomic destinations now diagnose earlier with E41403.
Ignored diagnostic NVRTC controls also failed narrow-select C++ overload resolution and are not passes.
No full campaign ran; no full suite rerun follows from these focused results.

## Current tested identity

Revision `4d18503346028be698054c1800ad3a0e142d6019` plus compiler patch
`3106b1dac662315749de32c2f4e09ccaba94b165ab3d0ca831d290b28586cabd`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `2c6b63e007000d5afedeb3b4e77e245e9bbb759cd1fbb43986577731c68a74ea`.
Provider SHA-256: `1b542e29de5338b29ade9fe4f5402933fab2bb4475a0f69d43d74fc16abbcad0`.
The later commit does not relabel these binaries. Current source/runtime hashes, exact outcomes and
failure history live in `focused-evidence.json` under `features.nvvm-core-values-and-atomics`.

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

Migrate wave producer identities and straightforward compositions through named NVVM calls and
existing canonical operations. Preserve convergence, explicit masks and reduction/scan combination
order; validate the affected contracts with a compact existing runtime selection. Keep pointer-result
frexp/modf, resource producer representation and general narrow-integer normalization as separate
bounded design work. Continue larger batches without routine module-version bumps.

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
