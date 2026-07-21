// Feature test for shader-slang/slang#12051: a resource loaded from a `UniformConstant` resource
// array (a bindless `DescriptorHandle<T>`, or a plain `Texture2D t[N]`) into a local and used N
// times should load the resource ONCE on SPIR-V, matching HLSL, which already reuses a hoisted
// `ResourceDescriptorHeap[i]`. Before the fix the SPIR-V legalizer (`insertLoadAtLatestLocation` in
// slang-ir-spirv-legalize.cpp) re-emitted a fresh `OpLoad` from the one shared element access chain
// at every use, so a texture+sampler sampled 3x produced 3 texture loads + 3 sampler loads (six
// loads, two access chains). The reporter worked around it with an inline-SPIR-V `OpCopyObject` pin;
// this makes the compiler coalesce the load automatically when the resource address is a read-only
// `UniformConstant` element whose definition dominates all its uses.
//
// The uses here are straight-line (three explicit samples), NOT an unrolled loop: the fix does not
// depend on `[ForceUnroll]`. Coalescing is done by the general redundancy-removal pass
// (`removeRedundancyInFunc`), which CSEs the movable, same-address element loads regardless of how
// the repeated uses arose. (A non-unrolled `for` loop would instead have a single static load in the
// loop body -- one load per iteration is already correct -- so it would not exercise the coalescing;
// repeated straight-line uses are what collapse from six loads to two.)
//
// Scope note: the fix keys on the *general* invariant -- a load of a read-only `UniformConstant`
// resource-element address that dominates its uses is safe to reuse -- so it applies to BOTH bindless
// descriptor handles (the motivating case) AND plain resource arrays. The `plainArrayMain` entry
// below pins that generalization.
//
// The coalescing is per-lane-correct even for a divergent (`NonUniformResourceIndex`) index: a
// single hoisted conversion produces one `getElement(array, index)` whose index carries that lane's
// value, so one load yields that lane's resource and every use reuses it. Divergence is *across
// lanes*, not across the repeated uses, so a single load per lane is right; a divergent case is
// asserted below to pin that it coalesces to the same two loads (parity with the already-shipping
// `spvDescriptorHeapEXT` path). On the `DescriptorHandle`/heap path the `NonUniformResourceIndex`
// marker is stripped during handle specialization before SPIR-V legalization, so uniform and
// divergent heap accesses are identical here and neither carries a `NonUniform` decoration (a
// pre-existing, documented gap -- see desc-heap-nonuniform.slang:8-12 -- tracked as a separate
// follow-up).
//
// These use the DEFAULT SPIR-V path (no `spvDescriptorHeapEXT`): that path lowers a
// `DescriptorHandle<T>` to `getElement(__slang_resource_heap, index)`, which the legalizer turns
// into one `OpAccessChain` + a load, and is where the per-use reload was introduced. The EXT path
// (`__spirvLoadDescriptorFromHeap`) already reuses and is unaffected.

//TEST:SIMPLE(filecheck=UNIFORM): -target spirv-asm -stage compute -entry uniformMain
//TEST:SIMPLE(filecheck=DIVERGENT): -target spirv-asm -stage compute -entry divergentMain
//TEST:SIMPLE(filecheck=PLAINARR): -target spirv-asm -stage compute -entry plainArrayMain

RWStructuredBuffer<float> outputBuffer;

// Uniform index, conversion hoisted into a local: the descriptor load must be coalesced to one per
// resource and reused across all three straight-line samples.
[shader("compute")]
[numthreads(1, 1, 1)]
void uniformMain(uint3 tid: SV_DispatchThreadID)
{
    Texture2D.Handle texH = { uint2(tid.x, 0) };
    SamplerState.Handle sampH = { uint2(tid.y, 0) };
    Texture2D tex = texH;
    SamplerState s = sampH;
    float acc = 0;
    acc += tex.SampleLevel(s, float2(0.0, 0), 0).x;
    acc += tex.SampleLevel(s, float2(0.1, 0), 0).x;
    acc += tex.SampleLevel(s, float2(0.2, 0), 0).x;
    outputBuffer[0] = acc;
}

// The texture and sampler descriptors are each loaded exactly once and reused across all three
// samples. A descriptor load disassembles as `OpLoad %<result-type-id> %<address-id>` (both operands
// numeric ids); the `gl_GlobalInvocationID` loads use a named `%v3uint` type operand and so do not
// match this pattern. Pre-fix there were six such loads (three texture + three sampler, one pair per
// sample); post-fix exactly two remain (one texture + one sampler), so the `-NOT` fences any third.
// UNIFORM-COUNT-2: OpLoad %{{[0-9]+}} %{{[0-9]+}}
// UNIFORM-NOT: OpLoad %{{[0-9]+}} %{{[0-9]+}}
// UNIFORM: OpFunctionEnd

// Divergent index (`NonUniformResourceIndex`), conversion hoisted into a local exactly as the
// uniform case: the single per-lane descriptor address is loaded once and reused across all three
// samples, so this coalesces to the same two descriptor loads. This pins per-lane correctness (the
// divergence is across lanes, not across the repeated uses) and parity with the EXT path.
[shader("compute")]
[numthreads(1, 1, 1)]
void divergentMain(uint3 tid: SV_DispatchThreadID)
{
    Texture2D tex = ResourceDescriptorHeap[NonUniformResourceIndex(tid.x)];
    SamplerState s = SamplerDescriptorHeap[NonUniformResourceIndex(tid.y)];
    float acc = 0;
    acc += tex.SampleLevel(s, float2(0.0, 0), 0).x;
    acc += tex.SampleLevel(s, float2(0.1, 0), 0).x;
    acc += tex.SampleLevel(s, float2(0.2, 0), 0).x;
    outputBuffer[0] = acc;
}

// The hoisted divergent conversion coalesces to exactly two descriptor loads, just like the uniform
// case: one per-lane texture load + one per-lane sampler load, reused across all three samples.
// DIVERGENT-COUNT-2: OpLoad %{{[0-9]+}} %{{[0-9]+}}
// DIVERGENT-NOT: OpLoad %{{[0-9]+}} %{{[0-9]+}}
// DIVERGENT: OpFunctionEnd

// Plain (non-bindless) `UniformConstant` resource arrays are coalesced identically -- the fix keys
// on the general read-only-UniformConstant-element invariant, not on the descriptor-heap shape. A
// `Texture2D[]` / `SamplerState[]` indexed with a uniform index into hoisted locals and sampled 3x
// must also collapse to one texture load + one sampler load, pinning the intended broadened scope.
Texture2D gTextures[4];
SamplerState gSamplers[4];

[shader("compute")]
[numthreads(1, 1, 1)]
void plainArrayMain(uniform uint idx)
{
    Texture2D tex = gTextures[idx];
    SamplerState s = gSamplers[idx];
    float acc = 0;
    acc += tex.SampleLevel(s, float2(0.0, 0), 0).x;
    acc += tex.SampleLevel(s, float2(0.1, 0), 0).x;
    acc += tex.SampleLevel(s, float2(0.2, 0), 0).x;
    outputBuffer[0] = acc;
}

// Exactly two resource loads (one texture + one sampler) feed all three samples. The `uniform uint`
// entry-point parameter is delivered via a PushConstant load, which disassembles with a named
// `%uint` type operand and so does not match the numeric-numeric `OpLoad %<id> %<id>` resource-load
// pattern -- keeping the count clean.
// PLAINARR-COUNT-2: OpLoad %{{[0-9]+}} %{{[0-9]+}}
// PLAINARR-NOT: OpLoad %{{[0-9]+}} %{{[0-9]+}}
// PLAINARR: OpFunctionEnd
