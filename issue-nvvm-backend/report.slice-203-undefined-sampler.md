# Slice 203: Undefined CUDA sampler placeholders

Status: accepted with targeted validation on 2026-09-24.

## Motivation

The tiled-brass material declares an uninitialized ordinary sampler and passes it alongside a real
texture to an explicit-LOD helper. Direct NVVM rejects the canonical undefined read even though
CUDA texture objects already own sampling state. Consider this complete reduced path:

```slang
DescriptorHandle<Texture2D<float4>> inputTexture;
RWStructuredBuffer<float4> outputBuffer;

[noinline]
float4 sampleTexture(Texture2D<float4> texture, SamplerState sampler, float2 uv)
{
    return texture.SampleLevel(sampler, uv, 0);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Texture2D<float4> texture = inputTexture;
    SamplerState sampler;
    outputBuffer[0] = sampleTexture(texture, sampler, float2(0.125));
}
```

The executable fixture binds a gradient texture, samples the four corner centers, and expects
`(xCorner, yCorner, 0, 1)`. A separate bound zero texture verifies that the resource binding actually
selects the sampled image. The sampler travels through a branch join and noinline helper; the
texture descriptor also follows the material's UInt64 conversion path. This supplies an independent
output oracle for the exact material boundary without claiming material runtime correctness.

## Proposed solution

Extend the existing ephemeral-value resolver and chosen-value emitter to ordinary SamplerState,
using its established i64 representation. Select zero through the existing integer constant
operation and retain that value in the SSA map. The IR contract permits a consistent concrete
choice. CUDA sampling never consumes the sampler payload, so this choice cannot change the texture
object's filtering or address modes.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: admit the existing ordinary sampler value classification in
  `_resolveNVVMEphemeralValue` and `_emitNVVMChosenUndefinedValue`.
- `tests/cuda/nvvm-undefined-sampler.slang`: actual gradient and zero texture sampling through
  descriptor, branch and helper boundaries in all three modes.
- `tests/cuda/nvvm-undefined-resource-unsupported.slang`: undefined texture and resource-aggregate
  values remain rejected at O0/O3 without dispatch.
- `tests/cuda/nvvm-undefined-comparison-sampler-unsupported.slang`: comparison-sampler helper values
  remain rejected at O0/O3; existing comparison-sampler storage coverage is also replayed.
- Discovery manifest: one eligible source, selected once with its oracle and input bindings intact.
  Exact subset selections, runtime evidence, plan, design note and STATUS preserve the handoff.

## Concepts and vocabulary

A **chosen undefined value** is a single concrete value selected for one
LoadFromUninitializedMemory instruction; every use observes the same choice, matching the IR's
freeze(undefined)-like contract. A **sampler placeholder** is the ordinary Slang sampler argument
that CUDA carries for source compatibility while the texture object owns the actual state.
The **copyable-value algebra** is the established finite numeric scalar/vector/aggregate domain;
resource-containing aggregates are outside that classification.

## Process report

`readVar` and `readVarRec` in `slang-ir-ssa.cpp` construct LoadFromUninitializedMemory when a read has
no reaching initialization. That shape is canonical and intentionally valid, as documented in
`slang-ir-insts.lua`; the backend does not need to repair its producer. In the final focused IR,
`sampleHandle` reconstructs its texture, creates two undefined SamplerState values, branches into a
sampler phi, and passes the selected value to `sampleTexture`. That helper contains the canonical
SampleLevel operation. The final code-generation IR retains this boundary, so the test is not
passing merely because Slang eliminated the undefined reads.

The existing `asNVVMSupportedSamplerValueType` admits exactly SamplerState. Type lowering already
maps it to i64 in value and helper-parameter roles. `_resolveNVVMTextureGenericAsm` validates the
sampler parameter but intentionally omits it from the texture-level provider operation. The CUDA
prelude documents the same texture-owned sampling state. `_resolveNVVMEphemeralValue` is therefore
the responsible admission boundary, and `_emitNVVMChosenUndefinedValue` already owns concrete
selection and the SSA map's consistent reuse. No new helper, value representation or syntax
reconstruction is necessary.

Helper/special-case inventory: two existing functions gain the exact ordinary-sampler case and
the emission assertion follows that same accepted domain. Both survive the input-shape audit.
No fallback, custom equivalence, arbitrary graph search, provider callback or shared type helper
changes. The existing copyable-value recursion remains unchanged, so a struct containing a texture
or sampler is not accidentally admitted. Negative fixtures preserve that boundary. Comparison
samplers still have storage support only; their helper parameter fails the existing precise
classification rather than entering ordinary sampler materialization.

A prototype sampler-return helper exposed an independent unsupported helper result type and was
removed from this fixture. It is not required by the material's argument path and does not justify
an ABI expansion. The final test suppresses only the two warnings for its deliberately uninitialized
placeholder reads. No warnings or failures are suppressed globally.

The actual revert drill restored HEAD's original emitter, rebuilt RelWithDebInfo and recovered the
accepted optimized compiler-library and slangc hashes exactly. Against the byte-identical final
fixtures, NVRTC O3 and six negatives pass; both direct positive lanes reject E52017
LoadFromUninitializedMemory. Restoring the narrow emitter change and rebuilding supplies the
pass-after evidence. The fixture SHA and before/after identities are recorded in the manifest.

Targeted final validation passes 17/17 focused checks, 4/4 runtime fixtures, 473/473 selected units
with one Windows-only skip, and 18/18 toolkit cells. Exactly 72 runtime cells were freshly executed:

| Corpus                                        | Identities | NVRTC O3 correct | NVVM O0 correct | NVVM O3 correct |
| --------------------------------------------- | ---------: | ---------------: | --------------: | --------------: |
| Frozen selected domain                        |         16 |               15 |              16 |              16 |
| Discovery selected domain, including addition |          8 |                6 |               6 |               6 |

The 69 old cells retain every classification, return code, execution count and parsed diagnostic;
62 are correct and seven remain known failures. The three added cells are correct. The other 1,536
old cells (1,482 correct and 54 failures) retain the accepted optimized checkpoint by reference;
they were not replayed on this source. Thus all 1,544 previous correct cells remain preservation
obligations with fresh or inherited evidence, and the new fixture adds three correct cells. This
is targeted acceptance, not a full checkpoint. The checked-in manifest and outcome TSVs retain
exact keys, no omissions/duplicates, failure references and hashes.

The seven freshly replayed failures are the frozen NVRTC texture-subscript infrastructure failure;
discovery texture-get-dimensions runtime mismatch at NVRTC O3 and preflight rejection at direct
O0/O3; and discovery multisample texture infrastructure failure at NVRTC O3 and preflight rejection
at direct O0/O3. No baseline was reset. The unchanged discovery runner requires 50--100 manifest
contracts before applying its substring filter, so a small subset-manifest attempt failed before
execution. Final runs use the authoritative full manifest with five disjoint existing `--match`
filters; merged keys exactly match the predeclared eight identities. All 83 old manifest records
are unchanged and the addition is eligible without frozen overlap.

All six complex support cells complete. Both NVRTC entries still compile and assemble, with
unchanged PTX. All four direct cells now reject this exact independent helper:

```text
GenericAsm assembly={uint32_t w, h; asm("txq.width.b32 %0, [%2]; txq.height.b32 %1, [%2];"
: "=r"(w), "=r"(h) : "l"($0)); *($1) = w;*($2) = h;},
signature=Void(Texture2D, OutParam<int>, OutParam<int>)
```

The minimal application trace is `render.TextureHandle.resolve_udim`: it constructs a
`Texture2D<uint>` indirection texture from the integer descriptor, declares `int2 dim`, then calls
`indirection_texture.GetDimensions(dim.x, dim.y)`. The final IR calls `_Texture.GetDimensions`
with that texture and the two element addresses; its canonical CUDA GenericAsm contains the width
and height queries shown above. This is the next candidate, not part of slice 203. No further
support or material runtime claim follows from reaching it.

The other retained undefined material values are specialized MxCompensatedConductorBSDF and
MxCompensatedDielectricBSDF constructor results. The final type audit shows only float vectors,
booleans and nested Fresnel structs (two float3 fields or one float), already in the numeric
copyable domain. They require no new handling. A genuinely resource-containing undefined struct
is audited separately and remains rejected by the negative fixture.

Tested source is base `60e2277f1522fd64a062960b43471d8a4ef33423` plus the exact hashes in
[runtime-validation.slice-203.json](runtime-validation.slice-203.json). The final compiler library
SHA256 is `fe80f943831841eb419cf2317037d7d83f57339545fb854581ca0175b513bb57`; provider hash and ABI 35
are unchanged. Native Linux RelWithDebInfo, CUDA 12.9.2/NVRTC 12.9.86, L4 SM89, driver 580.126.09,
target SM80 are unchanged. All builds cap outer jobs at four and inherited provider jobs at one;
suites run sequentially. The GPU remains healthy. Raw evidence lives under
`build/nvvm-loop/slice-203-before` and `build/nvvm-loop/slice-203-after`. No push, reboot, driver
change or performance experiment was performed.
