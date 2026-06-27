// Negative test for shader-slang/slang#11568: the untyped heap-handle types are nameable, so
// `UntypedResourceHandle(i)` / `UntypedSamplerHandle(j)` is a construction site distinct from the
// `ResourceDescriptorHeap[i]` subscript. This is the escape hatch around a subscript-only gate:
// without gating the constructor too, `Texture2D t = UntypedResourceHandle(i);` would still reach
// the Family-1 conversion and emit garbage on a target that cannot lower a `uint2` descriptor
// handle (here Metal). The `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` on the untyped
// handle constructors closes that escape — direct construction is diagnosed the same as the
// subscript form.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    UntypedResourceHandle rh = UntypedResourceHandle(tid.x);
    UntypedSamplerHandle  sh = UntypedSamplerHandle(tid.y);
    Texture2D    t = rh;
    SamplerState s = sh;
    outputBuffer[0] = t.SampleLevel(s, float2(0, 0), 0).x;
}

// CHECK: E36107
