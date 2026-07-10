// Negative test for shader-slang/slang#11568: the untyped heap-handle types are nameable, so
// `UntypedResourceHandle(i)` / `UntypedSamplerHandle(j)` is a construction site distinct from the
// `ResourceDescriptorHeap[i]` subscript. This is the escape hatch around a subscript-only gate:
// without gating the constructor too, `Texture2D t = UntypedResourceHandle(i);` would still reach
// the untyped-handle-to-concrete-type conversion and emit garbage on a target that cannot lower a
// `uint2` descriptor handle (here Metal). The `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]`
// on the untyped handle constructors closes that escape — direct construction is diagnosed the
// same as the subscript form.
//
// The two constructors are gated independently, so the resource and sampler constructors are
// exercised from their own entry points — `resourceMain` references only `UntypedResourceHandle`
// and `samplerMain` only `UntypedSamplerHandle` — and each is asserted independently. E36107 is a
// per-entry-point availability error, so a regression that removed the gate from just one
// constructor lets that entry point compile clean and fails its directive, instead of passing on
// the other constructor's diagnostic (which a single shared `CHECK: E36107` could not catch).

//DIAGNOSTIC_TEST:SIMPLE(filecheck=RES): -target metal -stage compute -entry resourceMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SAMP): -target metal -stage compute -entry samplerMain

RWStructuredBuffer<float> outputBuffer;
Texture2D boundTexture;

[numthreads(1, 1, 1)]
void resourceMain(uint3 tid : SV_DispatchThreadID)
{
    // Only the resource-handle constructor is referenced, so the diagnosed feature is its gate.
    UntypedResourceHandle rh = UntypedResourceHandle(tid.x);
    Texture2D t = rh;
    outputBuffer[0] = t.Load(int3(0, 0, 0)).x;
}

[numthreads(1, 1, 1)]
void samplerMain(uint3 tid : SV_DispatchThreadID)
{
    // Only the sampler-handle constructor is referenced; `boundTexture` is an ordinary bound
    // resource (no heap indexing), so the sole gated construct is the sampler constructor.
    UntypedSamplerHandle sh = UntypedSamplerHandle(tid.y);
    SamplerState s = sh;
    outputBuffer[0] = boundTexture.SampleLevel(s, float2(0, 0), 0).x;
}

// RES: E36107
// SAMP: E36107
