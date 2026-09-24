# Slice 202: CUDA integer texture descriptor conversions

**Status:** Accepted on 2026-09-24 with explicit existing capability gaps. Full before/after
runtime acceptance preserves every old classification and diagnostic. The development loop stops
after this local commit at the maintainer's request.

## Motivation

Both retained tiled-brass entries construct texture descriptors from CUDA's integer texture-object
handles. Direct NVVM rejects the canonical `CastUInt64ToDescriptorHandle` operation, although the
existing texture representation already carries those same 64 bits. Consider this reduced example:

```slang
DescriptorHandle<Texture2D<float4>> inputTexture;
RWStructuredBuffer<float4> outputBuffer;

[noinline]
uint64_t readHandle(DescriptorHandle<Texture2D<float4>> handle)
{
    return uint64_t(handle);
}

[noinline]
float4 loadTexture(uint64_t handle, int2 coordinate)
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(handle);
    return texture.Load(int3(coordinate, 0));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = loadTexture(readHandle(inputTexture), int2(0, 0));
}
```

The existing render-test harness binds a real CUDA texture to `inputTexture`. Extracting its bits
and reconstructing the typed resource across noinline helpers preserves both conversion operations
through Slang optimization. The prototype executes correctly through NVRTC; direct O0/O3 reject the
conversion before any kernel launches. This makes the material's blocker independently testable.

## Proposed solution

Extend the existing descriptor-conversion resolver to admit exact UInt64 conversions for the
already-supported read-only texture family. Forward the provider value through the existing
identity-conversion path. Preserve buffer descriptor rejection and the provider ABI.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: extend the shared resolver and its preflight, value validation,
  and emission opcode switches.
- `tests/cuda/nvvm-texture-descriptor-conversion.slang`: exercise real floating-point and integer
  textures, corner coordinates, and zero/high-bit/all-one/mixed 64-bit payload round trips.
- `tests/cuda/nvvm-texture-descriptor-buffer-unsupported.slang`: preserve rejection of integer conversions for the
  pointer/count buffer representation at O0/O3.
- Discovery manifest and runner: register the new runnable source and normalize an explicit CUDA
  target without changing source-identity overlap checks. Four runner regression tests cover
  normalization, retained oracle/dispatch options, duplicate sources, and frozen overlap.
- Plan, runtime manifest, per-identity results, design ledger, and STATUS: retain full current-host
  acceptance evidence and the next independent material blockers.

## Concepts and vocabulary

A **CUDA texture object** is the opaque 64-bit value supplied by the runtime. A **typed descriptor
handle** retains the Slang resource type while carrying the resource's CUDA representation. A
**buffer descriptor** instead carries the data pointer and count; it is not interchangeable with a
single integer. **Preflight** checks canonical Slang IR before the provider creates NVVM code.

## Process report

`DescriptorHandle<T>.__init(uint64_t)` and the inverse `uint64_t` constructor in
`hlsl.meta.slang` intentionally produce the two typed conversion instructions. They are valid
canonical inputs. CUDA layout's `GetDescriptorHandleLayout` gives a descriptor the resource's
layout, and `NVVMTypeLoweringContext::lowerType` maps supported read-only textures to i64, then
maps their descriptors to the same resource type. No producer-side repair is needed.

`_getNVVMDescriptorHandleConversion` is the existing boundary for operations whose executable
representation is unchanged. Its integer cases require exact UInt64 plus the existing
`getNVVMSupportedReadOnlyTextureType` classification. Preflight, selected-value validation, and
emission all use that same decision. Broadly admitting every descriptor would be wrong:
raw buffer handles carry a pointer/count aggregate. A new provider cast API or source-text resolver
would also be unnecessary because the canonical operation already identifies the semantics.

The runnable fixture uses the existing texture binding machinery and independent expected output,
including an initialized nonzero sentinel. Arbitrary boundary payloads are transported only; only
real bound texture handles are dereferenced. The fixture covers the material's float-vector and
unsigned-integer texture use, but does not establish the full material's runtime contract.

Self-review inventory: the implementation extends one existing helper and three existing
opcode consumers. No new helper, fallback, equivalence relation, type representation, or syntax
reconstruction is needed. Without the change the positive fixture fails at direct O0/O3 with
E52017, establishing the missing backend admission boundary. Negative buffer coverage prevents an
incorrect universal integer-handle interpretation.

The new CUDA fixture initially exposed a harness-selection restriction: the discovery target
adapter rejected `-cuda` even when the selected source was absent from frozen v1. The manifest and
`_load_discovery_workloads` already own exact selection, duplicate rejection, and frozen-source
exclusion. `_adapt_arguments_to_cuda` now normalizes CUDA alongside other target flags before the
shared `_directive_for_mode` chooses NVRTC O3 or direct O0/O3. This removes an obsolete proxy for
membership; it does not bypass the authoritative overlap check. Existing source inputs, output
oracles, IDs, and mode selection are preserved. The failed initial selection is retained separately
from the successful full discovery replay. No compiler rebuild is needed for this Python-only
adapter change; frozen acceptance uses the unchanged census runner and the same final compiler.

The four runner regression checks pass. Focused checks pass 8/8: the three real texture runtime
lanes, both buffer cast directions at O0/O3, and existing CUDA source-emission compatibility.
The four-fixture runtime gate, selected units (473/473, one Windows-only skip), and all 18 toolkit
cells pass. The new fixture also compiles and assembles in all three modes (six commands).
Full fresh acceptance contains exactly 1,605 runtime cells:

| Corpus                            | Identities | NVRTC O3 correct | NVVM O0 correct | NVVM O3 correct |
| --------------------------------- | ---------: | ---------------: | --------------: | --------------: |
| Frozen                            |        452 |              449 |             438 |             438 |
| Discovery, including the addition |         83 |               73 |              73 |              73 |

All 1,602 previous runtime cells retain their classifications and diagnostics, preserving all
1,541 previous correct cells. The new fixture adds three correct cells; the 61 existing failures
remain visible with exact IDs, modes, diagnostics, reproduction commands, and log hashes. All 82
old discovery contract records are exactly unchanged. Historical healthy denominators remain 427
and 72; the new identity is reported separately. Both diagnostic corpus runners return 2 because
existing failures remain, while the complex runner returns 1 for incomplete support. Neither exit
code is misrepresented as all-green support.

The [runtime manifest](runtime-validation.slice-202.json), [frozen outcomes](census.slice-202.tsv),
[discovery outcomes](discovery-census.slice-202.tsv), and [completed plan](plan.slice-202.md) retain
portable acceptance evidence. Raw data is under `build/nvvm-loop/slice-202-before` and
`build/nvvm-loop/slice-202-after`. Tested source is base `2634d3b9d` plus the exact file hashes in
the manifest. The host is Linux L4 SM89, driver 580.126.09, CUDA 12.9.2 / NVCC-NVRTC 12.9.86,
targeting SM80 with the Debug compiler and unchanged ABI-35 LLVM14 provider. The device remains
responsive after acceptance. No material runtime or performance claim is made.

### Next material blocker

All six complex cells complete. Both NVRTC entries still compile and assemble. Both entries at
both direct optimization levels now report E52017 `LoadFromUninitializedMemory`. The original
application source remains unchanged. Its `render.TextureHandle.sample` contains:

```slang
Texture2D<T> texture =
    DescriptorHandle<Texture2D<T>>(uint64_t(resolved_handle.texture_index));
SamplerState sampler;
return lod_sampler.sample(texture, sampler, uv);
```

The retained eval-buffer IR shows the integer cast, both descriptor conversions, a
`LoadFromUninitializedMemory : SamplerState` named `sampler`, and the call to
`render.ExplicitLodSampler.sample`. `readVar` / `readVarRec` in `slang-ir-ssa.cpp` produce this
instruction when an SSA read has no reaching initialization; `_validateNVVMFunction` rejects the
unsupported opcode. The IR also contains uninitialized aggregate constructor results. The generic
diagnostic does not identify which occurrence is visited first. This is a concrete next-blocker trace, not a claim that
all occurrences have the same semantics or should be replaced with zero. A future slice should
audit CUDA sampler-placeholder semantics separately from real uninitialized aggregate data.
The full material still lacks host bindings, texture/LUT inputs, and an output oracle.
