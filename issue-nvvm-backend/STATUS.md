# NVVM current status

The exp2 core migration and full checkpoint are accepted. The core explicitly selects
`__nv_exp2f` or `__nv_exp2` for NVVM; Half widens exactly, evaluates Float32 and narrows once.
Numeric operation 56, its semantic tag and CUDA-text recognizer are retired. The module range is
`min = max = 42`; provider ABI 46 and container format 2 remain unchanged.

**Resumed explicitly on 2026-09-30 after the reboot.** The maintainer approved the streamlined
validation in WORKFLOW: qualify the existing compiler with runtime/toolkit gates, capture log-family
baselines, then run one full checkpoint on the final compiler. Log-family preparation is active;
implementation and baseline execution have not started. No Slack, push or system changes.
Raw evidence is under ignored `build/nvvm-exp2/` and `build/nvvm-log-family/`; active plans and reports
remain uncommitted.

Read [WORKFLOW](WORKFLOW.md), the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. The [accepted baseline](accepted-baseline.json),
[identity](accepted-identity.json) and [focused evidence](focused-evidence.json) own current results.

## Accepted validation

| Evidence                                          | Result                                                                                                                 |
| ------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740; all 1,356 frozen and 384 discovery outcomes unchanged                                               |
| Main outcomes                                     | 1,704 correct; 36 unresolved and 21 resolved histories retained                                                        |
| Physical surfaces                                 | 249 cells: 214 pass, 24 compile failures, 11 retained NVRTC mismatches                                                 |
| Native units                                      | 1,177 identities: 1,165 pass, 12 inherited skips; four new exp2 passes                                                 |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 inherited skips; unchanged                                                            |
| Focused coverage                                  | 83 units covered by the full native run; 63 GPU cells, nine tag diagnostics, 106 capability tests                      |
| Oracles                                           | Seven suites / 21 fixtures; 57 exp2 controls and six retained rsqrt negative controls                                  |
| Exp2 numerical preservation                       | Nine buffers, 299 / 278 / 278 words by width, byte-identical to their respective baselines                             |
| Selected neighbors                                | 156 cells: 153 frozen and three discovery outcomes unchanged                                                           |
| Runtime / toolkit / material                      | 4 / 18 / 6 pass; material coverage remains compile/assembly only                                                       |
| Runner contracts                                  | 119 pass, one inherited skip; 83 surface CPU contracts pass                                                            |
| Module boundary                                   | 64 baseline version41 phases; 30 old metadata/rejection phases; 32 fresh version42 phases; three isolated static units |
| Focused features                                  | 26; all 25 prior feature objects retain original evidence identities                                                   |
| Last full / targeted / implementations since full | exp2 / exp2 / 0                                                                                                        |
| Validation stability                              | 126 source, two configuration, 39 runtime and 100 layout entries unchanged across final campaign and checkpoint        |

The exp2 corpus has 74 Half, 69 Float32 and 69 Float64 inputs, with 15 live observations per lane.
Half policy IDs `56042` and `1209` identify contracts, not module versions. Library admission uses
the empirical, non-guaranteed test-defined spacing/encoding union with explicit endpoint rules and
exact special-input overrides. CUDA Half preserves ex2 FTZ, Float32 bias FMA and RN16; NVVM Half
narrows the selected Float32 library result once. Equal candidate sets on this bounded corpus do
not establish universal target equivalence. All nine raw buffers, including NaN payloads, are preserved.
Preparation corrections and failed preflight attempts remain recorded with their resolutions.

Named library admission covers round, ceil, floor, trunc, rsqrt, exp and exp2. Selected definitions
own signatures. No shared signature validation, ABI or loading behavior changed. The migration used
one final compiler build; the full checkpoint was added because the maintainer requested a stopping
point. Direct retired-ID/text tests prove rejection independently of the module-version boundary.

## Tested identity

Source revision `966cea8ce0ebd3617a87eb0e82de17b57bf1567c` plus compiler patch
`c997f97460f6720658a8f004d6c52c7f0b0ed2ed3e04104ede963e4d6c09b274`, compiler version
`2026.18.3-347-g966cea8ce`. The later acceptance commit does not relabel these compiled bytes.
Compiler SHA-256: `8f9b499065c114bdad3eeaad7a974f89dcc39d1d577861788356ecfcc00d0a08`.
Provider SHA-256: `31028c56cfe81687f33bac8de3f08a8a9684f67edd880540f1d0c470270bb1b6`.

Qualification: native Ubuntu 24.04, L4 SM89, driver 580.126.09, CUDA 12.9.2 / NVRTC 12.9.86,
LLVM 14 and target SM80. Static executable/configuration captures remain separate from native units;
static tests cover historical versions 31–38 and a dynamic future version. Immutable module probes
cover exact version41 rejection. Earlier feature, material-runtime and performance evidence retains
its original tested identity.

## Current continuation

1. Resume is authorized. Read this file and WORKFLOW. Check the working tree and preserve
   the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.
2. Verify actual compiler, provider, core-module, configuration and runtime-layout identities against
   accepted-identity.json. Recheck CUDA/toolkit, driver, device and target architecture after reboot.
   Requalify changed artifacts or environment before baseline observations; Git HEAD alone is not
   proof of the loaded compiler identity.
3. The next bounded slice is **log/log2/log10 together**, using the existing selected-library path.
   Admit the six `__nv_log*` names and retire numeric IDs 60/61/62, their tags and text recognizers
   without renumbering. Expect one module42-to43 boundary after verifying entry state. Preserve
   ordinary comma-separated `__intrinsic_asm` arguments.
4. Read the unaccepted preparation under `build/nvvm-log-family/worker/`: `policy-proposal.md`,
   `plan-proposal.md` and `scope.json`. Create a bounded ExecPlan. Independently review and freeze
   signed logarithm references, special values and bounded CUDA correction/path controls
   before capturing old-module or GPU baselines. No prepared proposal is accepted runtime evidence.
5. Preserve distinct CUDA Half algorithms: log2 corrections use the evolving result; log/log10
   corrections use the input. Confirm the emitted CUDA `double log10` path: its existing
   `F64_log10(float)` wrapper appears to narrow input, evaluate Float32 log10f and widen, while NVVM
   evaluates true double. Keep that existing limitation separate from this migration.
6. Reuse approved runners with a small manifest, capture controls with accepted exp2 before edits,
   and combine the family migration/version bump in one build. Keep direct retired-ID/text negatives
   independent of version rejection. Then continue remaining math, compound-text migrations and
   recorded wave work as separately bounded slices, subject to checkpoint cadence and review.

Post-reboot inspection found all 126 source, 39 runtime/toolkit, two configuration and 100 layout
entries unchanged against accepted-identity.json. HEAD is `af34a0ef4`. The current host has eight
logical AMD EPYC CPUs and about 30 GiB RAM. Device remains L4 SM89, but UUID changed to
`GPU-7e9accb6-0e0f-7bb1-cafe-c1d02947b736` and driver changed to 595.71.05. These observations are
not new accepted GPU results. Full requalification is pending; the accepted identities above retain
their historical driver and device. The worker proposal's older cadence is superseded: exp2 is
the last full checkpoint and there are zero accepted implementations since it.

## Retained boundaries

Module42 requires older user modules and separately supplied built-ins to be recompiled for every
backend. Metadata inspection and source fallback remain available. Remove the NVVM semantic-tag
extension only after its last consumers migrate; pre-existing explicit intrinsic arguments remain.

Preserve all 36 main gaps and focused NVRTC narrow-bit/nested-array failures and timeouts.
Packed/normalized surfaces, general aliases, resource provenance, dynamic components and
three-channel transfers remain outside current physical legalization. Checked address/memory plans
remain authoritative; recursive admission and structured/resource conversion debt remain.
Barrier convergence, external Half ABI, numeric sweep, material-runtime and performance conclusions
retain prior qualifications. Snapshot caching remains non-atomic with external libdevice replacement.
Repeated bare-static-state dispatch in nvvm-copyable-kernel-context remains unresolved. Relinking a
compiled requirement-free component may retain cached target output after option changes.
CUDA `dim3 == uint3` source emission remains unsupported.
