# NVVM current status

The log/log2/log10 core migration and full environment checkpoint are accepted. Core NVVM branches
select six libdevice names; Half widens exactly, evaluates Float32 and narrows once. Numeric
operations 60/61/62, their semantic tags and CUDA-text recognizers are retired. The module range
is `min = max = 43`; provider ABI 46 and container format 2 remain unchanged.

**Continuation explicitly resumed under the faster workflow on 2026-09-30.** The maintainer
superseded the stop-after-log request: use eight build jobs, larger related intrinsic batches,
focused compile/PTX checks and no routine module-version bumps. Prioritize removal of CUDA-string
recognizers. No Slack, push or system changes. Raw log-family evidence stays under ignored
`build/nvvm-log-family/`; active plans and report drafts remain uncommitted. The user's untracked
`tests/cuda/complex/tiled_brass_material_mtlx_update.slang` remains unchanged.

Read [WORKFLOW](WORKFLOW.md), the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. The [accepted baseline](accepted-baseline.json),
[identity](accepted-identity.json) and [focused evidence](focused-evidence.json) own current results.

## Accepted validation

| Evidence | Result |
| --- | --- |
| Cases / sources / mode cells | 580 / 576 / 1,740; all 1,356 frozen and 384 discovery outcomes unchanged |
| Main outcomes | 1,704 correct; 36 unresolved and 21 resolved histories retained |
| Physical surfaces | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches |
| Native units | 1,189 identities: 1,177 pass, 12 inherited skips; 12 new log-family passes |
| Semantic regressions | 1,248 identities: 1,170 pass, 78 inherited skips; unchanged |
| Focused coverage | 95 units mapped to full native; 90 GPU cells, 12 tag diagnostics, 106 capability tests |
| Oracles | Eight suites / 30 fixtures; log checker reference/path controls and fixture/source mutations passed |
| Log-family preservation | All 27 raw mode buffers byte-identical to their respective baselines |
| Selected neighbors | 156 cells: 153 frozen and three discovery outcomes unchanged |
| Runtime / toolkit / material | 4 / 18 / 6 pass; material coverage remains compile/assembly only |
| Runner contracts | 119 pass, one inherited skip; 83 surface CPU contracts pass |
| Module boundary | 192 baseline module42 phases; 90 old rejection phases; 96 fresh module43 phases; three static units |
| Focused features | 27; all 26 prior feature objects retain their original evidence identities |
| Last full / targeted / implementations since full | log-family / log-family / 0 |
| Validation stability | 139 source, two configuration, 39 runtime and 100 layout entries unchanged |

The numerical corpus has 91/62/62 Half/Float32/Float64 inputs for log and log2 and 91/62/80 for
log10: 663 inputs with 15 live observations each. Independent signed references qualify the
empirical, non-guaranteed library spacing/encoding union; CUDA Half checks ideal RN-even results
on this finite corpus. All such checks passed without tolerance changes. Bounded correction/path
controls preserve distinct CUDA Half algorithms without claiming complete PTX approximation
qualification. CUDA double log10 retains its existing RN32 input / Float32 evaluation / Float64
widening path, confirmed in emitted source/PTX; preservation is not true-double accuracy evidence.
All 27 raw buffers, including NaN payloads, retain their own target/mode baseline bytes.

Named library admission now covers round, ceil, floor, trunc, rsqrt, exp, exp2, log, log2 and log10.
Selected definitions own signatures. No shared signature validation, ABI or loading behavior
changed. One final main build and one isolated static build were used. Direct retired-ID/text
negatives prove rejection independently of the module-version boundary.

The initial runtime4/toolkit18 qualification used the unchanged accepted compiler on the new
machine. The full final checkpoint supplies environment requalification and the originally requested
stop-point gate. The campaign's full native run satisfies focused units; checkpoint cells satisfy the selected
neighbor/runtime/material obligations. Those checks were not rerun separately. Preserved failures
include an invalid source-probe profile, a review
path-binding preflight failure, a source-gate filename collision and a static-cache wrapper
regex/detail-path failure. The cache initialized without running tests; a separate receipt validates
that observation while preserving the failed wrapper. Reviewed recovery reused
completed gates with unchanged identities/timestamps instead of repeating successful tests.

## Tested identity

Source revision `76f3cb8763f5847c133b91a69d47b87675f2a068` plus compiler patch
`1d27642656a5990e0b9d8dac54a04a55c6392a1ff43762d134ebd2f54141d56d`, compiler version
`2026.18.3-349-g76f3cb876`. The later acceptance commit does not relabel these compiled bytes.
Compiler SHA-256: `29f55117eba63e74eeb350a624ea732a00f281dad44cf4844167b5be203824dc`.
Provider SHA-256: `2bedd7348e2c04a7cc10686f1d577831bb759ab60e50e74294b0b9bf95aa404f`.

Qualification: native Ubuntu 24.04, L4 SM89, driver 595.71.05, CUDA 12.9.2 / NVRTC 12.9.86,
LLVM 14 and target SM80. GPU UUID `GPU-7e9accb6-0e0f-7bb1-cafe-c1d02947b736`; host has eight
logical AMD EPYC CPUs and about 30 GiB RAM. Earlier feature, static, material-runtime and performance
claims retain their original tested identities. No new performance claim follows from the upgrade.

## Next action

Continue the core-intrinsic migration in larger batches under WORKFLOW. Inventory remaining math,
compound and wave consumers; use existing named calls and core composition, then retire their
legacy CUDA-text/tag routes. The earlier single-atan proposal is superseded by the larger-batch
priority. Use focused compile/PTX/unit checks and small runtime selections where semantics require
execution. Do not repeat this full checkpoint for each addition or bump module43 routinely.
Preserve the user's untracked material and keep current tested identities distinct from historical
full evidence.

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
