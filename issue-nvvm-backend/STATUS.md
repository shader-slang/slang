# NVVM current status

The CUDA-text route migration is **complete with focused local acceptance**. NVVM no longer
infers operations from CUDA assembly-body strings or active semantic tags. The final field-offset
recognizer is replaced by a typed query that preserves the exact field key before optimization.
Explicit LLVM/libdevice names and genuine primitive PTX remain intentional backend interfaces.
Module43, ABI46 and container2 are unchanged. The signed16 O3 failure is corrected by provider-side
normalization at exact-width integer consumers. The consolidated integration checkpoint is accepted;
operation dispatch, shared type-role admission and structured-buffer planning are accepted. Fake-provider maintenance and architecture refresh are next.

The accelerated workflow authorized on 2026-09-30 remains in effect: eight build jobs, larger
related batches, focused compile/PTX/runtime checks and no routine module-version bumps or full
campaigns. The maintainer superseded the earlier stop-after-log request. The maintainer has resolved the
offset scope: preserve existing NVVM restrictions and leave CUDA/CPP behavior unchanged. The final
offset migration is accepted. The maintainer has now authorized the ordered correctness,
integration, cleanup and Float16 work below; continue through bounded reviewed local commits,
stopping only for a decision that actually needs human input. After all queued cleanup and Float16
work, the maintainer additionally requests a full NVVM validation checkpoint, fixes for discovered
issues, and resumption of feature development under the standard workflow. No push, Slack or system changes.
Preserve the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-storage-planning/`; earlier cleanup evidence remains in its recorded ignored paths, and integration evidence remains under `build/nvvm-integration/run-1/`.
Plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence                                          | Result                                                                                                                  |
| ------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| Build                                             | Eight-job shared build 30 seconds; isolated static refresh 318 seconds; test-only corrections 19/10 seconds             |
| Focused units                                     | 23 shared and 5 direct static pass; only affected units rerun after diagnostic corrections                              |
| Runtime / PTX                                     | 7 NVVM runtime cells and matrix PTX check pass; new recursive bool/float3 fixture also passed unchanged before refactor |
| Ownership                                         | Structured load/root facts and conversion recipes retained; child addresses reuse checked parent facts                  |
| Retained                                          | Canonical types, role caches, raw-view access, Half ABI, physical layout and preflight no-mutation contracts            |
| Last full / targeted / implementations since full | log-family / structured storage planning / 12                                                                           |
| Historical evidence                               | Full baseline/identity and all 39 earlier feature objects retained                                                      |

The structured-buffer load/store family now selects conversions during preflight; emission executes
the retained recipes. The emitter no longer recursively rediscovers parent address provenance.
Dimensions, surfaces and general physical-storage legalization remain separate scope.
Three external BF16 negatives now diagnose the unsupported resource root before deferred field
selection; one direct global-array negative now diagnoses unavailable `global_var` before its child.
Only those exact test expectations changed. Initial 22/23 shared and 4/5 static outcomes remain
recorded; all rejection/no-output/no-mutation checks already passed. No production fix was needed.

Earlier dispatch and shared type-admission acceptance remains recorded with its own identity.
The user-requested full checkpoint follows the remaining fake/docs cleanup and Float16 batch;
then fix discovered issues and resume standard feature work.

The bounded integration checkpoint is accepted in `features.nvvm-post-migration-integration`.
Five native failures were outdated migration assertions: early-folded size/alignment constants,
removed SampleLevel helper call, and three AllEqual checks requiring CUDA's instruction sequence.
Exact values, helper ABI/storage/texture checks and backend-specific PTX assertions now pass; the
initial failures remain recorded. The other 1,167 native passes were reused without a second full
run. Full corpus, numerical, material and performance campaigns were not repeated.

Earlier core-tail qualifications remain, including Float64 modf's lack of fresh numerical
qualification. Unsized CUDA arrays remain 16-byte pointer/count wrappers aligned to 8.

This bounded checkpoint does not establish full-repository or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure now passes with its exact original expectation. The former
failure returned correct low16 bits but compared/widened as +32768. Canonical LLVM i16 was correct;
the provider now enforces signed/unsigned interpretation at consumers through an explicit PTX
widen/narrow pair that LLVM cannot remove. Half/BF16 transport and storage/helper ABI are unchanged.
The original evidence remains in `features.nvvm-core-values-and-atomics`; its historical unresolved
status is superseded by the new `features.nvvm-integer16-normalization` resolution record.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `18fbbf036995c502a5874beb465bcd00e20a7f5a` plus compiler patch
`427b9e476dfd01462505b2c16f114f9d8f0b217a889d5763bda6bb2a62003d9b`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `42e1d1106cbe72d3333e88a48d703077c1ba5bb2df0ec1d5a0ca559ed2fef756`.
Provider SHA-256: `4294999dab9e0576f44b2d4c7823d68830ac3870fb6cfb1249f0ca9651f34a72`.
Source/runtime/test/configuration hashes are in `features.nvvm-structured-storage-planning`.
Static tests retain separate compiler/core/provider identities and explicit reuse after test-only fixes.
Later commits do not relabel these binaries. Earlier integration and failure evidence retain their
actual identities, including the explicit signed16 resolution.

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

The agreed migration and residual-text audit are finished. The maintainer authorized this sequence
on 2026-09-30. Steps 1–5 are complete; steps 6–7 are next:

1. Fix the known signed16 O3 normalization failure at its responsible layer, retaining its failure
   history and avoiding an abs-specific workaround.
2. Run one consolidated integration checkpoint for the accumulated migrations. Reuse existing
   runners and compare exact outcomes; no repeated numerical/material/performance campaign by default.
3. Consolidate generic/exact operation resolution in the real and fake providers. Delete shadowed
   arithmetic/comparison/conversion/bitcast catalog rows and unreachable Float32 helpers; retain
   genuinely distinct canonical wave operations and independent semantic assertions.
4. Give role-specific type admission one provider-independent analysis reusable by preflight and
   lowering. Preserve local-storage, helper-parameter/result and resource-storage distinctions,
   Half helper ABI transport, representation caches and rejection before provider mutation.
5. Complete checked address/storage planning for one resource family. Child addresses reuse parent
   facts; emission executes selected resource/storage conversions instead of rediscovering them.
   Do not introduce a general physical-storage legalization pass for this maintenance.
6. Separate fake-provider fixtures from recording infrastructure and reduce repetitive behavior and
   duplicate libdevice signatures. Keep independent expectations and real-provider coverage.
7. Refresh architecture around surviving ownership/invariants. Remove stale scalar/wave resolution
   descriptions; keep numerical qualifications with maintained contracts/evidence and preserve histories.
8. Address Float16 instruction selection after cleanup, using the bounded sequence below.
9. Run the full NVVM validation checkpoint once the queued work is complete; preserve exact failure
   outcomes and fix discovered issues, rerunning the affected checks before accepting the checkpoint.
10. Resume bounded reviewed feature development under WORKFLOW, selecting the next concrete gap
    from the capability ledger and retained failures. Economical per-batch validation remains in force.

Each cleanup uses existing positive, negative and no-mutation tests with focused runtime checks
where semantics require them. Eight-job incremental builds and economical testing remain the default.
LLVM-dialect adaptation and the concrete ABI/layout boundaries remain intentional.

### Float16 follow-up

CUDA C++ is a comparison backend, not an automatic semantic oracle. Prefer Slang behavior aligned
with Vulkan and D3D12 contracts, verify the relevant specifications, and document deliberate differences.
The maintainer's CUDA12.9/SM80 probes are leads to reproduce, not new accepted validation:

- First investigate direct `llvm.ceil.f16`, `llvm.floor.f16` and `llvm.trunc.f16`, reported to emit
  `cvt.rpi/rmi/rzi.f16.f16` without Float32 promotion. Extend checked named-intrinsic admission as needed.
- Then select direct Half FMA with an explicit rounding contract. The current promoted NVVM path
  can double-round: a=1.0009765625, b=1.5, c=-2^-24 gives 1.501953125 through Float32 versus
  1.5009765625 with direct Half rounding. CUDA's `__hfma` already uses the direct form.
- Treat approximate Half `exp2`/`tanh` as a separate accuracy/capability decision. The reported
  SM75+ PTX forms compile as inline assembly; named NVVM forms did not in the maintainer's probes.
- Keep `round` separate: current NVVM ties-away and CUDA Half ties-even differ. Establish the
  intended Slang/Vulkan/D3D12 contract instead of assuming CUDA is correct.
- Do not assume a Half API implies Half instructions. CUDA sqrt/rsqrt/reciprocal widen; sin/cos/log/exp
  can require Float32 calculations and input-specific corrections. Preserve or change these policies
  only with explicit semantic and accuracy evidence.

CUDA/CPP offset helper bodies remain
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
