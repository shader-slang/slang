// Regression/feature test for shader-slang/slang#11568:
// Allow HLSL SM6.6 ResourceDescriptorHeap[i] / SamplerDescriptorHeap[i] directly in Slang source.
// The resource type is recovered from the assignment target via implicit conversion from the
// untyped heap handle, then rides the existing DescriptorHandle<T> lowering on every gated target:
// HLSL emits the SM6.6 heap-index strings, SPIR-V uses SPV_EXT_descriptor_heap, and GLSL/WGSL emit
// the same `_slang_resource_heap` array access as the equivalent DescriptorHandle<T> path.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Family-1: T solved from the declared variable type, not from the index argument.
    Texture2D t = ResourceDescriptorHeap[tid.x];
    SamplerState s = SamplerDescriptorHeap[tid.y];
    // A non-texture resource kind takes a different legalization path (structured buffer).
    RWStructuredBuffer<float> buf = ResourceDescriptorHeap[tid.z];
    // Family-2: the .Handle (DescriptorHandle<T>) conversion spelling.
    Texture2D.Handle th = ResourceDescriptorHeap[tid.x];
    Texture2D t2 = th;
    outputBuffer[0] = t.SampleLevel(s, float2(0.0, 0.0), 0.0).x
        + t2.SampleLevel(s, float2(0.0, 0.0), 0.0).x
        + buf[0];
}

// Each spelling must emit a heap index derived from the corresponding thread component, so the
// checks pin the thread component inside the bracket (`.x` texture, `.y` sampler, `.z` buffer),
// not just the bare prefix, proving the three accesses route distinct indices.
// HLSL: ResourceDescriptorHeap[{{.*}}.x{{.*}}]
// HLSL: SamplerDescriptorHeap[{{.*}}.y{{.*}}]
// HLSL: ResourceDescriptorHeap[{{.*}}.z{{.*}}]

// The SPIR-V output must declare the extension, declare both heap built-ins, and actually
// index each heap via an untyped access chain (not merely declare the extension).
// SPV: OpExtension "SPV_EXT_descriptor_heap"
// SPV: BuiltIn ResourceHeapEXT
// SPV: BuiltIn SamplerHeapEXT
// SPV: OpUntypedAccessChainKHR{{.*}}%slang_resourceHeap
// SPV: OpUntypedAccessChainKHR{{.*}}%slang_samplerHeap

// GLSL and WGSL are inside the same `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` gate, so
// the untyped path must actually lower there too (GPU-free text emit, no device). Both declare a
// `_slang_resource_heap` array and index it with a thread-derived `uint2(index, 0)` heap value,
// exactly as the `DescriptorHandle<T>` path does in desc-handle-3.slang — verifying the path emits,
// not merely that it is gated.
// GLSL: _slang_resource_heap{{[0-9_]+}}[]
// GLSL: _slang_resource_heap{{[0-9_]+}}[uvec2(gl_GlobalInvocationID
// WGSL: _slang_resource_heap{{[0-9_]+}}{{.*}}array<texture_2d
// WGSL: _slang_resource_heap{{[0-9_]+}}[vec2<u32>(tid
