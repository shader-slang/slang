# NVVM current status

The CUDA-text route migration is **complete with focused local acceptance**. NVVM no longer
infers operations from CUDA assembly-body strings or active semantic tags. The final field-offset
recognizer is replaced by a typed query that preserves the exact field key before optimization.
Explicit LLVM/libdevice names and genuine primitive PTX remain intentional backend interfaces.
Module43, ABI46 and container2 are unchanged. The signed16 O3 failure is corrected by provider-side
normalization at exact-width integer consumers; consolidated integration is now in progress.

The accelerated workflow authorized on 2026-09-30 remains in effect: eight build jobs, larger
related batches, focused compile/PTX/runtime checks and no routine module-version bumps or full
campaigns. The maintainer superseded the earlier stop-after-log request. The maintainer has resolved the
offset scope: preserve existing NVVM restrictions and leave CUDA/CPP behavior unchanged. The final
offset migration is accepted. The maintainer has now authorized the ordered correctness,
integration, cleanup and Float16 work below; continue through bounded reviewed local commits,
stopping only for a decision that actually needs human input. No push, Slack or system changes.
Preserve the user's untracked `tests/cuda/complex/tiled_brass_material_mtlx_update.slang` unchanged.

Read [WORKFLOW](WORKFLOW.md), [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery. Raw current evidence is under ignored
`build/nvvm-signed16/` and `build/nvvm-integration/run-1/`; plans and reports remain uncommitted.

## Latest focused acceptance

| Evidence                                          | Result                                                                                                       |
| ------------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| Build                                             | Provider-only eight-job build passed in 8 seconds; compiler and test tools unchanged                         |
| Focused units                                     | Numeric type-family and invalid-scalar provider units passed inside the consolidated native run              |
| Runtime                                           | 4 NVVM O0/O3 cases pass: existing core-values and new narrow-integer consumer regression                     |
| Primitive probes                                  | Signed and unsigned widen/narrow normalization survive libNVVM O3 and GPU execution                          |
| Scope                                             | Integer comparisons, widening, float conversion, division/remainder and right-shift consumers; ABI unchanged |
| Last full / targeted / implementations since full | log-family / signed16 / 9                                                                                    |
| Historical evidence                               | Full baseline/identity and all 35 earlier feature objects retained                                           |

The consolidated integration run is a bounded checkpoint, not a new full corpus/numerical/material
baseline. Its native gate has five expectation failures under investigation (layout constants,
resource-helper call count and three AllEqual PTX checks); no pass claim is made for that run. Earlier core-tail qualifications remain, including Float64 modf's lack of fresh
numerical qualification. Unsized CUDA arrays remain 16-byte pointer/count wrappers aligned to 8.

This is not an all-pass repository checkpoint or publication/CI readiness. The inherited
`nvvm-core-values.slang.1` O3 failure now passes with its exact original expectation. The former
failure returned correct low16 bits but compared/widened as +32768. Canonical LLVM i16 was correct;
the provider now enforces signed/unsigned interpretation at consumers through an explicit PTX
widen/narrow pair that LLVM cannot remove. Half/BF16 transport and storage/helper ABI are unchanged.
The original evidence remains in `features.nvvm-core-values-and-atomics`; its historical unresolved
status is superseded by the new `features.nvvm-integer16-normalization` resolution record.
CUDA WaveMaskMatch historically uses match-all while NVVM uses match-any; the shared wave fixture
covers AllEqual without claiming differing-value Match masks agree.

## Current tested identity

Revision `de493da9e0951bfcd3ecba6848ae0f433f741c0e` plus the provider patch recorded in
`features.nvvm-integer16-normalization`; compiler version `2026.18.3-350-gdc0a9acc3`.
Compiler SHA-256: `1378468640c4b444809566aade1257d2cb77682faf1d0a120c19439e96ddc528`.
Provider SHA-256: `ce0c6bf613372977c4f985b8d6853fe36aa24f8e83181392091859555e66589c`.
The later commit does not relabel these binaries. Source/runtime hashes and exact outcomes live in
`focused-evidence.json` under `features.nvvm-integer16-normalization`; earlier identities and failure histories remain
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

The agreed migration and residual-text audit are finished. The maintainer authorized this sequence
on 2026-09-30; signed16 correction is complete and the integration checkpoint is active:

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
